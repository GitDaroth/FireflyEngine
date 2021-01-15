#pragma once

#include "Rendering/Material.h"
#include <vulkan/vulkan.hpp>

namespace Firefly
{
	class VulkanMaterial : public Material
	{
	public:
		VulkanMaterial(std::shared_ptr<GraphicsContext> context);

		virtual void Destroy() override;

		vk::DescriptorSet GetTexturesDescriptorSet() const;

	protected:
		virtual void OnInit() override;
		virtual void OnSetTexture(std::shared_ptr<Texture> texture, TextureUsage usage) override;

	private:
		vk::Device m_device;
		vk::DescriptorPool m_descriptorPool;

		vk::DescriptorSetLayout m_materialTexturesDescriptorSetLayout;
		vk::DescriptorSet m_materialTexturesDescriptorSet;
	};
}