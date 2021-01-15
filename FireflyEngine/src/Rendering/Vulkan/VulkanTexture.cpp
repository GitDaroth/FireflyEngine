#include "pch.h"
#include "Rendering/Vulkan/VulkanTexture.h"

#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanUtils.h"

namespace Firefly
{
	VulkanTexture::VulkanTexture(std::shared_ptr<GraphicsContext> context) :
		Texture(context)
	{
		std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(context);
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

	void VulkanTexture::OnInit(unsigned char* pixelData, ColorSpace colorSpace)
	{
        switch (colorSpace)
        {
        case ColorSpace::SRGB:
            m_format = vk::Format::eR8G8B8A8Srgb;
            break;
        case ColorSpace::RGB:
        default:
            m_format = vk::Format::eR8G8B8A8Unorm;
            break;
        }

        CreateImage(pixelData);
        CreateImageView();
        CreateSampler();
	}

    void VulkanTexture::CreateImage(unsigned char* pixelData)
    {
        m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_width, m_height)))) + 1;

        vk::DeviceSize imageSize = m_width * m_height * 4;
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
        VulkanUtils::CreateImage(m_device, m_physicalDevice, m_width, m_height, m_mipLevels, vk::SampleCountFlagBits::e1, m_format, tiling, imageUsageFlags, memoryPropertyFlags, m_image, m_imageMemory);

        vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined;
        vk::ImageLayout newLayout = vk::ImageLayout::eTransferDstOptimal;
        VulkanUtils::TransitionImageLayout(m_device, m_commandPool, m_queue, m_image, m_mipLevels, m_format, oldLayout, newLayout);

        VulkanUtils::CopyBufferToImage(m_device, m_commandPool, m_queue, stagingBuffer, m_image, m_width, m_height);

        VulkanUtils::GenerateMipmaps(m_device, m_physicalDevice, m_commandPool, m_queue, m_image, m_format, m_width, m_height, m_mipLevels);

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
        m_imageView = VulkanUtils::CreateImageView(m_device, m_image, m_mipLevels, m_format, vk::ImageAspectFlagBits::eColor);
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
        samplerCreateInfo.maxLod = static_cast<float>(m_mipLevels);
        samplerCreateInfo.mipLodBias = 0.0f;

        if (m_device.createSampler(&samplerCreateInfo, nullptr, &m_sampler) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create Vulkan texture sampler!");
    }

    void VulkanTexture::DestroySampler()
    {
        m_device.destroySampler(m_sampler);
    }
}