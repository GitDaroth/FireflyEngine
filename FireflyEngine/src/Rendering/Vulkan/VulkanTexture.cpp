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
        return m_sampler;
    }

	void VulkanTexture::OnInit(void* pixelData)
	{
        m_format = ConvertToVulkanFormat(m_description.format);

        CreateImage(pixelData);
        CreateImageView();
        CreateSampler();
	}

    void VulkanTexture::CreateImage(void* pixelData)
    {
        uint32_t bytePerPixel = GetBytePerPixel(m_description.format);
        vk::DeviceSize imageSize = m_description.width * m_description.height * bytePerPixel;
        vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
        vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        VulkanUtils::CreateBuffer(m_device, m_physicalDevice, imageSize, bufferUsageFlags, memoryPropertyFlags, stagingBuffer, stagingBufferMemory);

        void* mappedMemory;
        m_device.mapMemory(stagingBufferMemory, 0, imageSize, {}, &mappedMemory);
        memcpy(mappedMemory, pixelData, imageSize);
        m_device.unmapMemory(stagingBufferMemory);

        vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
        vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        VulkanUtils::CreateImage(m_device, m_physicalDevice, m_description.width, m_description.height, m_mipMapLevels, vk::SampleCountFlagBits::e1, m_format, tiling, imageUsageFlags, memoryPropertyFlags, m_image, m_imageMemory);

        vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined;
        vk::ImageLayout newLayout = vk::ImageLayout::eTransferDstOptimal;
        VulkanUtils::TransitionImageLayout(m_device, m_commandPool, m_queue, m_image, m_mipMapLevels, m_format, oldLayout, newLayout);

        VulkanUtils::CopyBufferToImage(m_device, m_commandPool, m_queue, stagingBuffer, m_image, m_description.width, m_description.height);

        VulkanUtils::GenerateMipmaps(m_device, m_physicalDevice, m_commandPool, m_queue, m_image, m_format, m_description.width, m_description.height, m_mipMapLevels);

        m_device.destroyBuffer(stagingBuffer);
        m_device.freeMemory(stagingBufferMemory);
    }

    void VulkanTexture::DestroyImage()
    {
        m_device.destroyImage(m_image);
        m_device.freeMemory(m_imageMemory);
    }

    void VulkanTexture::CreateImageView()
    {
        m_imageView = VulkanUtils::CreateImageView(m_device, m_image, m_mipMapLevels, m_format, vk::ImageAspectFlagBits::eColor);
    }

    void VulkanTexture::DestroyImageView()
    {
        m_device.destroyImageView(m_imageView);
    }

    void VulkanTexture::CreateSampler()
    {
        vk::SamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.pNext = nullptr;
        samplerCreateInfo.flags = {};
        samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerCreateInfo.anisotropyEnable = true;
        samplerCreateInfo.maxAnisotropy = m_physicalDevice.getProperties().limits.maxSamplerAnisotropy;
        samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerCreateInfo.unnormalizedCoordinates = false;
        samplerCreateInfo.compareEnable = false;
        samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
        samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerCreateInfo.magFilter = vk::Filter::eLinear;
        samplerCreateInfo.minFilter = vk::Filter::eLinear;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = static_cast<float>(m_mipMapLevels);
        samplerCreateInfo.mipLodBias = 0.0f;

        if (m_device.createSampler(&samplerCreateInfo, nullptr, &m_sampler) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create Vulkan texture sampler!");
    }

    void VulkanTexture::DestroySampler()
    {
        m_device.destroySampler(m_sampler);
    }

    vk::Format VulkanTexture::ConvertToVulkanFormat(Format format)
    {
        vk::Format vulkanFormat;

        switch (format)
        {
        case Format::R_8:
            vulkanFormat = vk::Format::eR8Unorm;
            break;
        case Format::R_8_NON_LINEAR:
            vulkanFormat = vk::Format::eR8Srgb;
            break;
        case Format::R_16_FLOAT:
            vulkanFormat = vk::Format::eR16Sfloat;
            break;
        case Format::R_32_FLOAT:
            vulkanFormat = vk::Format::eR32Sfloat;
            break;
        case Format::RG_8:
            vulkanFormat = vk::Format::eR8G8Unorm;
            break;
        case Format::RG_8_NON_LINEAR:
            vulkanFormat = vk::Format::eR8G8Srgb;
            break;
        case Format::RG_16_FLOAT:
            vulkanFormat = vk::Format::eR16G16Sfloat;
            break;
        case Format::RG_32_FLOAT:
            vulkanFormat = vk::Format::eR32G32Sfloat;
            break;
        case Format::RGB_8:
            vulkanFormat = vk::Format::eR8G8B8Unorm;
            break;
        case Format::RGB_8_NON_LINEAR:
            vulkanFormat = vk::Format::eR8G8B8Srgb;
            break;
        case Format::RGB_16_FLOAT:
            vulkanFormat = vk::Format::eR16G16B16Sfloat;
            break;
        case Format::RGB_32_FLOAT:
            vulkanFormat = vk::Format::eR32G32B32Sfloat;
            break;
        case Format::RGBA_8:
            vulkanFormat = vk::Format::eR8G8B8A8Unorm;
            break;
        case Format::RGBA_8_NON_LINEAR:
            vulkanFormat = vk::Format::eR8G8B8A8Srgb;
            break;
        case Format::RGBA_16_FLOAT:
            vulkanFormat = vk::Format::eR16G16B16A16Sfloat;
            break;
        case Format::RGBA_32_FLOAT:
            vulkanFormat = vk::Format::eR32G32B32A32Sfloat;
            break;
        case Format::DEPTH_32_FLOAT:
            vulkanFormat = vk::Format::eD32Sfloat;
            break;
        case Format::DEPTH_24_STENCIL_8:
            vulkanFormat = vk::Format::eD24UnormS8Uint;
            break;
        }

        return vulkanFormat;
    }

    uint32_t VulkanTexture::GetBytePerPixel(Format format)
    {
        uint32_t bytePerPixel = 0;

        switch (format)
        {
        case Format::R_8:
        case Format::R_8_NON_LINEAR:
            bytePerPixel = 1;
            break;
        case Format::RG_8:
        case Format::RG_8_NON_LINEAR:
        case Format::R_16_FLOAT:
            bytePerPixel = 2;
            break;
        case Format::RGB_8:
        case Format::RGB_8_NON_LINEAR:
            bytePerPixel = 3;
            break;
        case Format::R_32_FLOAT:
        case Format::RG_16_FLOAT:
        case Format::RGBA_8:
        case Format::RGBA_8_NON_LINEAR:
        case Format::DEPTH_32_FLOAT:
        case Format::DEPTH_24_STENCIL_8:
            bytePerPixel = 4;
            break;
        case Format::RGB_16_FLOAT:
            bytePerPixel = 6;
            break;
        case Format::RG_32_FLOAT:
        case Format::RGBA_16_FLOAT:
            bytePerPixel = 8;
            break;
        case Format::RGB_32_FLOAT:
            bytePerPixel = 12;
            break;
        case Format::RGBA_32_FLOAT:
            bytePerPixel = 16;
            break;
        }

        return bytePerPixel;
    }
}