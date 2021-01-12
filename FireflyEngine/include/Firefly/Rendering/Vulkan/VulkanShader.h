#pragma once

#include "vulkan/vulkan.hpp"
#include "Rendering/Vulkan/VulkanSwapchain.h"

struct SpvReflectTypeDescription;

namespace Firefly
{
	struct ShaderCodePath
	{
		std::string vertexShaderPath = "";
		std::string tesselationControlShaderPath = "";
		std::string tesselationEvaluationShaderPath = "";
		std::string geometryShaderPath = "";
		std::string fragmentShaderPath = "";
		std::string computeShaderPath = "";
	};

	class VulkanShader
	{
	public:
		void Init(vk::Device device, const std::string& tag, const ShaderCodePath& shaderCodePath);
		void Destroy();

		std::string GetTag() const;
		std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos() const;

	private:
		vk::PipelineShaderStageCreateInfo CreateShaderStage(const std::vector<char>& shaderCode, vk::ShaderStageFlagBits shaderStage);
		static std::vector<char> ReadBinaryFile(const std::string& fileName);
		void PrintShaderReflection(const std::vector<char>& shaderCode);
		std::string GetDataTypeName(const SpvReflectTypeDescription& typeDescription);

		vk::Device m_device;
		std::string m_tag;
		std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStageCreateInfos;
		std::vector<vk::ShaderModule> m_shaderModules;
	};
}