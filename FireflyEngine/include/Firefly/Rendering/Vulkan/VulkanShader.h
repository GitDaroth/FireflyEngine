#pragma once

#include "Rendering/Shader.h"
#include "vulkan/vulkan.hpp"

struct SpvReflectTypeDescription;

namespace Firefly
{
	class VulkanShader : public Shader
	{
	public:
		VulkanShader();

		virtual void Destroy() override;

		std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos() const;

	protected:
		virtual void OnInit(const ShaderCode& shaderCode) override;

	private:
		vk::PipelineShaderStageCreateInfo CreateShaderStage(const std::vector<char>& shaderCode, vk::ShaderStageFlagBits shaderStage);

		void PrintShaderReflection(const std::vector<char>& shaderCode);
		std::string GetDataTypeName(const SpvReflectTypeDescription& typeDescription);

		vk::Device m_device;
		std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStageCreateInfos;
		std::vector<vk::ShaderModule> m_shaderModules;
	};
}