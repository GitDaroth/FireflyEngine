#include "pch.h"
#include "Rendering/Vulkan/VulkanTexture.h"

#include "Rendering/RenderingAPI.h"
#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanUtils.h"

namespace Firefly
{
	VulkanTexture::VulkanTexture() :
		Texture()
	{
		std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
		m_device = vkContext->GetDevice()->GetDevice();
        m_physicalDevice = vkContext->GetDevice()->GetPhysicalDevice();
        m_commandPool = vkContext->GetCommandPool();
		m_descriptorPool = vkContext->GetDescriptorPool();
        m_queue = vkContext->GetDevice()->GetGraphicsQueue();
	}

	void VulkanTexture::Destroy()
	{
        if (m_description.useSampler)
            DestroySampler();
        DestroyImageView();
        DestroyImage();
	}

    vk::Image VulkanTexture::GetImage() const
    {
        return m_image;
    }

    vk::ImageView VulkanTexture::GetImageView() const
    {
        return m_imageView;
    }

    vk::Sampler VulkanTexture::GetSampler() const
    {
        FIREFLY_ASSERT(m_description.useSampler, "Missing Vulkan image sampler!");
        return m_sampler;
    }

	void VulkanTexture::OnInit(void* pixelData)
	{
        m_format = ConvertToVulkanFormat(m_description.format);

        CreateImage(pixelData);
        CreateImageView();
        if (m_description.useSampler)
            CreateSampler();
	}

    void VulkanTexture::CreateImage(void* pixelData)
    {
        vk::ImageUsageFlags usage;
        if (pixelData)
            usage |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
        if (m_description.useSampler)
            usage |= vk::ImageUsageFlagBits::eSampled;
        if (m_description.useAsAttachment)
        {
            if (HasColorFormat())
                usage |= vk::ImageUsageFlagBits::eColorAttachment;
            else if (HasDepthFormat() || HasDepthStencilFormat())
                usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        }

        vk::Format format = ConvertToVulkanFormat(m_description.format);
        vk::SampleCountFlagBits sampleCount = ConvertToVulkanSampleCount(m_description.sampleCount);
        vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        CreateVulkanImage(m_description.width, m_description.height, format,
                          m_mipMapLevels, m_arrayLayers, sampleCount,
                          usage, memoryPropertyFlags,
                          m_image, m_imageMemory);

        if (pixelData)
        {
            vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined;
            vk::ImageLayout newLayout = vk::ImageLayout::eTransferDstOptimal;
            TransitionImageLayout(m_image, oldLayout, newLayout, format, m_mipMapLevels, m_arrayLayers);

            uint32_t bytePerPixel = GetBytePerPixel(m_description.format);
            vk::DeviceSize bufferSize = m_description.width * m_description.height * m_arrayLayers * bytePerPixel;
            vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eTransferSrc;
            vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

            vk::Buffer stagingBuffer;
            vk::DeviceMemory stagingBufferMemory;
            CreateVulkanBuffer(bufferSize, usage, memoryPropertyFlags, stagingBuffer, stagingBufferMemory);

            void* mappedMemory;
            m_device.mapMemory(stagingBufferMemory, 0, bufferSize, {}, &mappedMemory);
            memcpy(mappedMemory, pixelData, bufferSize);
            m_device.unmapMemory(stagingBufferMemory);

            CopyBufferToImage(stagingBuffer, m_image, m_description.width, m_description.height, format, m_arrayLayers);

            if (m_description.useSampler)
            {
                if (m_description.sampler.isMipMappingEnabled)
                {
                    // automatically transitions image layout into "eShaderReadOnlyOptimal"
                    GenerateMipMaps(m_image, m_description.width, m_description.height, format, m_mipMapLevels, m_arrayLayers);
                }
                else
                {
                    vk::ImageLayout oldLayout = vk::ImageLayout::eTransferDstOptimal;
                    vk::ImageLayout newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                    TransitionImageLayout(m_image, oldLayout, newLayout, format, m_mipMapLevels, m_arrayLayers);
                }
            }

            m_device.destroyBuffer(stagingBuffer);
            m_device.freeMemory(stagingBufferMemory);
        }
    }

    void VulkanTexture::DestroyImage()
    {
        m_device.destroyImage(m_image);
        m_device.freeMemory(m_imageMemory);
    }

    void VulkanTexture::CreateImageView()
    {
        vk::Format format = ConvertToVulkanFormat(m_description.format);

        vk::ImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.pNext = nullptr;
        imageViewCreateInfo.flags = {};
        imageViewCreateInfo.image = m_image;
        imageViewCreateInfo.viewType = ConvertToVulkanTextureType(m_description.type);
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
        imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
        imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
        imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
        imageViewCreateInfo.subresourceRange.aspectMask = GetImageAspectFlags(format);
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = m_mipMapLevels;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = m_arrayLayers;

        vk::Result result = m_device.createImageView(&imageViewCreateInfo, nullptr, &m_imageView);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan image view!");
    }

    void VulkanTexture::DestroyImageView()
    {
        m_device.destroyImageView(m_imageView);
    }

    void VulkanTexture::CreateSampler()
    {
        vk::SamplerAddressMode wrapMode = ConvertToVulkanWrapMode(m_description.sampler.wrapMode);

        float maxAnisotropy = m_physicalDevice.getProperties().limits.maxSamplerAnisotropy;
        if (m_description.sampler.maxAnisotropy > maxAnisotropy)
        {
            Logger::Warn("Vulkan", "Max. anisotropy ({0}) is bigger than the device limit ({1}).", m_description.sampler.maxAnisotropy, maxAnisotropy);
            m_description.sampler.maxAnisotropy = maxAnisotropy;
        }

        vk::SamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.pNext = nullptr;
        samplerCreateInfo.flags = {};
        samplerCreateInfo.addressModeU = wrapMode;
        samplerCreateInfo.addressModeV = wrapMode;
        samplerCreateInfo.addressModeW = wrapMode;
        samplerCreateInfo.anisotropyEnable = m_description.sampler.isAnisotropicFilteringEnabled;
        samplerCreateInfo.maxAnisotropy = m_description.sampler.maxAnisotropy;
        samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerCreateInfo.unnormalizedCoordinates = false;
        samplerCreateInfo.compareEnable = false;
        samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
        samplerCreateInfo.minFilter = ConvertToVulkanFilterMode(m_description.sampler.minificationFilterMode);
        samplerCreateInfo.magFilter = ConvertToVulkanFilterMode(m_description.sampler.magnificationFilterMode);
        samplerCreateInfo.mipmapMode = ConvertToVulkanMipMapFilterMode(m_description.sampler.mipMapFilterMode);
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = static_cast<float>(m_mipMapLevels);
        samplerCreateInfo.mipLodBias = 0.0f;

        vk::Result result = m_device.createSampler(&samplerCreateInfo, nullptr, &m_sampler);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan image sampler!");
    }

    void VulkanTexture::DestroySampler()
    {
        m_device.destroySampler(m_sampler);
    }

    void VulkanTexture::CreateVulkanBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryPropertyFlags, 
                                           vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
    {
        std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
        vk::Device device = vkContext->GetDevice()->GetDevice();
        vk::PhysicalDevice physicalDevice = vkContext->GetDevice()->GetPhysicalDevice();

        vk::BufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.pNext = nullptr;
        bufferCreateInfo.flags = {};
        bufferCreateInfo.size = bufferSize;
        bufferCreateInfo.usage = usage;
        bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
        bufferCreateInfo.queueFamilyIndexCount = 0;
        bufferCreateInfo.pQueueFamilyIndices = nullptr;

        vk::Result result = device.createBuffer(&bufferCreateInfo, nullptr, &buffer);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan buffer!");


        vk::MemoryRequirements memoryRequirements;
        device.getBufferMemoryRequirements(buffer, &memoryRequirements);

        vk::MemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(memoryRequirements, memoryPropertyFlags);

        result = device.allocateMemory(&memoryAllocateInfo, nullptr, &bufferMemory);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Failed to allocate Vulkan image memory!");

        device.bindBufferMemory(buffer, bufferMemory, 0);
    }

    void VulkanTexture::CreateVulkanImage(uint32_t width, uint32_t height, vk::Format format,
                                          uint32_t mipMapLevels, uint32_t arrayLayers, vk::SampleCountFlagBits sampleCount,
                                          vk::ImageUsageFlags usage, vk::MemoryPropertyFlags memoryPropertyFlags, 
                                          vk::Image& image, vk::DeviceMemory& imageMemory)
    {
        std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
        vk::Device device = vkContext->GetDevice()->GetDevice();

        vk::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.pNext = nullptr;
        imageCreateInfo.flags = {};
        imageCreateInfo.imageType = vk::ImageType::e2D;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = mipMapLevels;
        imageCreateInfo.arrayLayers = arrayLayers;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
        imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageCreateInfo.usage = usage;
        imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
        imageCreateInfo.samples = sampleCount;

        vk::Result result = device.createImage(&imageCreateInfo, nullptr, &image);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan image!");


        vk::MemoryRequirements memoryRequirements;
        device.getImageMemoryRequirements(image, &memoryRequirements);

        vk::MemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(memoryRequirements, memoryPropertyFlags);

        result = device.allocateMemory(&memoryAllocateInfo, nullptr, &imageMemory);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Failed to allocate Vulkan image memory!");

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    void VulkanTexture::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, vk::Format format, uint32_t arrayLayers)
    {
        std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
        vk::Device device = vkContext->GetDevice()->GetDevice();
        vk::CommandPool commandPool = vkContext->GetCommandPool();
        vk::Queue queue = vkContext->GetDevice()->GetGraphicsQueue();

        vk::CommandBuffer commandBuffer = VulkanUtils::BeginOneTimeCommandBuffer(device, commandPool);

        vk::BufferImageCopy bufferImageCopy{};
        bufferImageCopy.bufferOffset = 0;
        bufferImageCopy.bufferRowLength = 0;
        bufferImageCopy.bufferImageHeight = 0;
        bufferImageCopy.imageSubresource.aspectMask = GetImageAspectFlags(format);
        bufferImageCopy.imageSubresource.mipLevel = 0;
        bufferImageCopy.imageSubresource.baseArrayLayer = 0;
        bufferImageCopy.imageSubresource.layerCount = arrayLayers;
        bufferImageCopy.imageOffset = { 0, 0, 0 };
        bufferImageCopy.imageExtent = { width, height, 1 };
        commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &bufferImageCopy);

        VulkanUtils::EndCommandBuffer(device, commandBuffer, commandPool, queue);
    }

    void VulkanTexture::TransitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::Format format, uint32_t mipMapLevels, uint32_t arrayLayers)
    {
        std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
        vk::Device device = vkContext->GetDevice()->GetDevice();
        vk::CommandPool commandPool = vkContext->GetCommandPool();
        vk::Queue queue = vkContext->GetDevice()->GetGraphicsQueue();

        vk::ImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.pNext = nullptr;
        imageMemoryBarrier.oldLayout = oldLayout;
        imageMemoryBarrier.newLayout = newLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = GetImageAspectFlags(format);
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = mipMapLevels;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = arrayLayers;

        vk::PipelineStageFlags sourcePipelineStageFlags;
        vk::PipelineStageFlags destinationPipelineStageFlags;

        switch (oldLayout)
        {
        case vk::ImageLayout::eUndefined:
            imageMemoryBarrier.srcAccessMask = {};
            sourcePipelineStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
            break;
        case vk::ImageLayout::eTransferDstOptimal:
            imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            sourcePipelineStageFlags = vk::PipelineStageFlagBits::eTransfer;
            break;
        default:
            FIREFLY_ASSERT(false, "Unsupported layout transition!");
        }

        switch (newLayout)
        {
        case vk::ImageLayout::eTransferDstOptimal:
            imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            destinationPipelineStageFlags = vk::PipelineStageFlagBits::eTransfer;
            break;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            destinationPipelineStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
            break;
        case vk::ImageLayout::eColorAttachmentOptimal:
            imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
            destinationPipelineStageFlags = vk::PipelineStageFlagBits::eEarlyFragmentTests;
            break;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        case vk::ImageLayout::eDepthAttachmentOptimal:
        case vk::ImageLayout::eStencilAttachmentOptimal:
            imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            destinationPipelineStageFlags = vk::PipelineStageFlagBits::eEarlyFragmentTests;
            break;
        default:
            FIREFLY_ASSERT(false, "Unsupported layout transition!");
        }

        vk::CommandBuffer commandBuffer = VulkanUtils::BeginOneTimeCommandBuffer(device, commandPool);
        commandBuffer.pipelineBarrier(sourcePipelineStageFlags, destinationPipelineStageFlags, {}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        VulkanUtils::EndCommandBuffer(device, commandBuffer, commandPool, queue);
    }

    void VulkanTexture::GenerateMipMaps(vk::Image image, uint32_t width, uint32_t height, vk::Format format, uint32_t mipMapLevels, uint32_t arrayLayers)
    {
        std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
        vk::Device device = vkContext->GetDevice()->GetDevice();
        vk::PhysicalDevice physicalDevice = vkContext->GetDevice()->GetPhysicalDevice();
        vk::CommandPool commandPool = vkContext->GetCommandPool();
        vk::Queue queue = vkContext->GetDevice()->GetGraphicsQueue();

        // Check if image format supports linear blitting
        vk::FormatProperties formatProperties;
        physicalDevice.getFormatProperties(format, &formatProperties);
        FIREFLY_ASSERT(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear, "Vulkan image format does not support linear blitting!");

        vk::CommandBuffer commandBuffer = VulkanUtils::BeginOneTimeCommandBuffer(device, commandPool);

        vk::ImageAspectFlags aspectMask = GetImageAspectFlags(format);

        vk::ImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.pNext = nullptr;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = arrayLayers;
        imageMemoryBarrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = width;
        int32_t mipHeight = height;
        for (uint32_t i = 1; i < mipMapLevels; i++)
        {
            imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
            imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            vk::ImageBlit imageBlit{};
            imageBlit.srcOffsets[0] = { 0, 0, 0 };
            imageBlit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            imageBlit.srcSubresource.aspectMask = aspectMask;
            imageBlit.srcSubresource.mipLevel = i - 1;
            imageBlit.srcSubresource.baseArrayLayer = 0;
            imageBlit.srcSubresource.layerCount = arrayLayers;
            imageBlit.dstOffsets[0] = { 0, 0, 0 };
            imageBlit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            imageBlit.dstSubresource.aspectMask = aspectMask;
            imageBlit.dstSubresource.mipLevel = i;
            imageBlit.dstSubresource.baseArrayLayer = 0;
            imageBlit.dstSubresource.layerCount = arrayLayers;
            commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, 1, &imageBlit, vk::Filter::eLinear);

            imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            imageMemoryBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            if (mipWidth > 1)
                mipWidth /= 2;
            if (mipHeight > 1)
                mipHeight /= 2;
        }

        imageMemoryBarrier.subresourceRange.baseMipLevel = mipMapLevels - 1;
        imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        imageMemoryBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        VulkanUtils::EndCommandBuffer(device, commandBuffer, commandPool, queue);
    }

    uint32_t VulkanTexture::GetMemoryTypeIndex(vk::MemoryRequirements memoryRequirements, vk::MemoryPropertyFlags memoryPropertyFlags)
    {
        std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
        vk::PhysicalDevice physicalDevice = vkContext->GetDevice()->GetPhysicalDevice();

        vk::PhysicalDeviceMemoryProperties memoryProperties;
        physicalDevice.getMemoryProperties(&memoryProperties);
        uint32_t memoryTypeIndex = UINT32_MAX;
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            // IsMemoryTypeSupported && AreMemoryTypePropertyFlagsSupported
            if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
            {
                memoryTypeIndex = i;
                break;
            }
        }
        FIREFLY_ASSERT(memoryTypeIndex < UINT32_MAX, "Failed to find suitable memory type for Vulkan image!");

        return memoryTypeIndex;
    }

    vk::ImageAspectFlags VulkanTexture::GetImageAspectFlags(vk::Format format)
    {
        vk::ImageAspectFlags aspectMask;
        switch (format)
        {
        case vk::Format::eD16Unorm:
        case vk::Format::eD32Sfloat:
            aspectMask = vk::ImageAspectFlagBits::eDepth;
            break;
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
            aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            break;
        default:
            aspectMask = vk::ImageAspectFlagBits::eColor;
            break;
        }

        return aspectMask;
    }

    vk::Format VulkanTexture::ConvertToVulkanFormat(Format format)
    {
        switch (format)
        {
        case Format::R_8:
            return vk::Format::eR8Unorm;
        case Format::R_8_NON_LINEAR:
            return vk::Format::eR8Srgb;
        case Format::R_16_FLOAT:
            return vk::Format::eR16Sfloat;
        case Format::R_32_FLOAT:
            return vk::Format::eR32Sfloat;
        case Format::RG_8:
            return vk::Format::eR8G8Unorm;
        case Format::RG_8_NON_LINEAR:
            return vk::Format::eR8G8Srgb;
        case Format::RG_16_FLOAT:
            return vk::Format::eR16G16Sfloat;
        case Format::RG_32_FLOAT:
            return vk::Format::eR32G32Sfloat;
        case Format::RGB_8:
            return vk::Format::eR8G8B8Unorm;
        case Format::RGB_8_NON_LINEAR:
            return vk::Format::eR8G8B8Srgb;
        case Format::RGB_16_FLOAT:
            return vk::Format::eR16G16B16Sfloat;
        case Format::RGB_32_FLOAT:
            return vk::Format::eR32G32B32Sfloat;
        case Format::RGBA_8:
            return vk::Format::eR8G8B8A8Unorm;
        case Format::RGBA_8_NON_LINEAR:
            return vk::Format::eR8G8B8A8Srgb;
        case Format::RGBA_16_FLOAT:
            return vk::Format::eR16G16B16A16Sfloat;
        case Format::RGBA_32_FLOAT:
            return vk::Format::eR32G32B32A32Sfloat;
        case Format::DEPTH_32_FLOAT:
            return vk::Format::eD32Sfloat;
        case Format::DEPTH_24_STENCIL_8:
            return vk::Format::eD24UnormS8Uint;
        }
    }

    vk::ImageViewType VulkanTexture::ConvertToVulkanTextureType(Type type)
    {
        switch (type)
        {
        case Type::TEXTURE_2D:
            return vk::ImageViewType::e2D;
        case Type::TEXTURE_CUBE_MAP:
            return vk::ImageViewType::eCube;
        }
    }

    vk::SamplerAddressMode VulkanTexture::ConvertToVulkanWrapMode(WrapMode wrapMode)
    {
        switch (wrapMode)
        {
        case WrapMode::REPEAT:
            return vk::SamplerAddressMode::eRepeat;
        case WrapMode::MIRRORED_REPEAT:
            return vk::SamplerAddressMode::eMirroredRepeat;
        case WrapMode::CLAMP_TO_EDGE:
            return vk::SamplerAddressMode::eClampToEdge;
        case WrapMode::MIRROR_CLAMP_TO_EDGE:
            return vk::SamplerAddressMode::eMirrorClampToEdge;
        case WrapMode::CLAMP_TO_BORDER:
            return vk::SamplerAddressMode::eClampToBorder;
        }
    }

    vk::SamplerMipmapMode VulkanTexture::ConvertToVulkanMipMapFilterMode(FilterMode mipMapFilterMode)
    {
        switch (mipMapFilterMode)
        {
        case FilterMode::LINEAR:
            return vk::SamplerMipmapMode::eLinear;
        case FilterMode::NEAREST:
            return vk::SamplerMipmapMode::eNearest;
        }
    }

    vk::Filter VulkanTexture::ConvertToVulkanFilterMode(FilterMode filterMode)
    {
        switch (filterMode)
        {
        case FilterMode::LINEAR:
            return vk::Filter::eLinear;
        case FilterMode::NEAREST:
            return vk::Filter::eNearest;
        }
    }
    vk::SampleCountFlagBits VulkanTexture::ConvertToVulkanSampleCount(SampleCount sampleCount)
    {
        switch (sampleCount)
        {
        case SampleCount::SAMPLE_1:
            return vk::SampleCountFlagBits::e1;
        case SampleCount::SAMPLE_2:
            return vk::SampleCountFlagBits::e2;
        case SampleCount::SAMPLE_4:
            return vk::SampleCountFlagBits::e4;
        case SampleCount::SAMPLE_8:
            return vk::SampleCountFlagBits::e8;
        case SampleCount::SAMPLE_16:
            return vk::SampleCountFlagBits::e16;
        case SampleCount::SAMPLE_32:
            return vk::SampleCountFlagBits::e32;
        case SampleCount::SAMPLE_64:
            return vk::SampleCountFlagBits::e64;
        }
    }
}