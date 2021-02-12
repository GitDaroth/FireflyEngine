#pragma once

#include "Rendering/Texture.h"
#include <vulkan/vulkan.hpp>

namespace Firefly
{
    class VulkanTexture : public Texture
    {
    public:
        VulkanTexture();

        virtual void Destroy() override;

        vk::Image GetImage() const;
        vk::ImageView GetImageView() const;
        vk::Sampler GetSampler() const;
        bool HasSampler() const;

        static void CreateVulkanBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryPropertyFlags,
            vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
        static void CreateVulkanImage(uint32_t width, uint32_t height, vk::Format format,
            uint32_t mipMapLevels, uint32_t arrayLayers, vk::SampleCountFlagBits sampleCount,
            vk::ImageUsageFlags usage, vk::MemoryPropertyFlags memoryPropertyFlags, vk::ImageCreateFlags createFlags,
            vk::Image& image, vk::DeviceMemory& imageMemory);
        static void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, vk::Format format, uint32_t arrayLayers);
        static void TransitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
            vk::Format format, uint32_t mipMapLevels, uint32_t arrayLayers);
        static void GenerateMipMaps(vk::Image image, uint32_t width, uint32_t height, vk::Format format, uint32_t mipMapLevels, uint32_t arrayLayers);
        static uint32_t GetMemoryTypeIndex(vk::MemoryRequirements memoryRequirements, vk::MemoryPropertyFlags memoryPropertyFlags);
        static vk::ImageAspectFlags GetImageAspectFlags(vk::Format format);

        static vk::Format ConvertToVulkanFormat(Format format);
        static vk::ImageViewType ConvertToVulkanTextureType(Type type);
        static vk::SamplerAddressMode ConvertToVulkanWrapMode(WrapMode wrapMode);
        static vk::SamplerMipmapMode ConvertToVulkanMipMapFilterMode(FilterMode mipMapFilterMode);
        static vk::Filter ConvertToVulkanFilterMode(FilterMode filterMode);
        static vk::SampleCountFlagBits ConvertToVulkanSampleCount(SampleCount sampleCount);

    protected:
        virtual void OnInit(void* pixelData) override;

    private:
        void CreateImage(void* pixelData);
        void DestroyImage();
        void CreateImageView();
        void DestroyImageView();
        void CreateSampler();
        void DestroySampler();

        vk::Device m_device;
        vk::PhysicalDevice m_physicalDevice;
        vk::CommandPool m_commandPool;
        vk::DescriptorPool m_descriptorPool;
        vk::Queue m_queue;

        vk::Image m_image;
        vk::DeviceMemory m_imageMemory;
        vk::ImageView m_imageView;
        vk::Sampler m_sampler;
        vk::Format m_format;
    };
}