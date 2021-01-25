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

	protected:
		virtual void OnInit(void* pixelData) override;

	private:
		void CreateImage(void* pixelData);
		void DestroyImage();
		void CreateImageView();
		void DestroyImageView();
		void CreateSampler();
		void DestroySampler();
		static vk::Format ConvertToVulkanFormat(Format format);
		static uint32_t GetBytePerPixel(Format format);

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