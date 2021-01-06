#pragma once

#include "vulkan/vulkan.hpp"

namespace Firefly
{
	class VulkanMaterial
	{
	public:
		VulkanMaterial(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, vk::Device device, vk::RenderPass renderPass, vk::Extent2D swapchainExtent);
		~VulkanMaterial();

		vk::Pipeline GetPipeline() const;
		vk::PipelineLayout GetPipelineLayout() const;

	private:
		void CreateGraphicsPipeline();
		void DestroyGraphicsPipeline();

		static std::vector<char> ReadBinaryFile(const std::string& fileName);

		vk::Device m_device;
		vk::RenderPass m_renderPass;
		vk::Extent2D m_swapchainExtent;

		vk::PipelineLayout m_pipelineLayout;
		vk::Pipeline m_pipeline;
		std::string m_vertexShaderPath;
		std::string m_fragmentShaderPath;
	};
}