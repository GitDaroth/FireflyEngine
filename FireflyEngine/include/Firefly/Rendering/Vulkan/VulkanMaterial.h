#pragma once

#include "vulkan/vulkan.hpp"

#include "Rendering/Vulkan/VulkanSwapchain.h"

struct SpvReflectTypeDescription;

namespace Firefly
{
	class VulkanMaterial
	{
	public:
		VulkanMaterial(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, vk::Device device, vk::RenderPass renderPass, VulkanSwapchain* swapchain);
		~VulkanMaterial();

		void CreatePipeline(vk::RenderPass renderPass, VulkanSwapchain* swapchain);
		void DestroyPipeline();

		vk::Pipeline GetPipeline() const;
		vk::PipelineLayout GetPipelineLayout() const;
		vk::DescriptorSetLayout GetGlobalDescriptorSetLayout() const;

	private:
		vk::PipelineShaderStageCreateInfo CreateShaderStage(const std::vector<char>& shaderCode, vk::ShaderStageFlagBits shaderStage);

		static std::vector<char> ReadBinaryFile(const std::string& fileName);
		void PrintShaderReflection(const std::vector<char>& shaderCode);
		std::string GetDataTypeName(const SpvReflectTypeDescription& typeDescription);

		vk::Device m_device;

		vk::PipelineLayout m_pipelineLayout;
		vk::Pipeline m_pipeline;
		std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStageCreateInfos;

		vk::DescriptorSetLayout m_globalDescriptorSetLayout;
	};
}