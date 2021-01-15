#include "pch.h"
#include "Rendering/Vulkan/VulkanShader.h"

#include "Rendering/Vulkan/VulkanContext.h"
#include <spirv_reflect.h>

namespace Firefly
{
	VulkanShader::VulkanShader(std::shared_ptr<GraphicsContext> context) :
		Shader(context)
	{
		std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(context);
		m_device = vkContext->GetDevice()->GetDevice();
	}

	void VulkanShader::OnInit(const ShaderCode& shaderCode)
	{
		if (!shaderCode.vertex.empty())
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode.vertex, vk::ShaderStageFlagBits::eVertex));
		if (!shaderCode.tesselationControl.empty())
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode.tesselationControl, vk::ShaderStageFlagBits::eTessellationControl));
		if (!shaderCode.tesselationEvaluation.empty())
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode.tesselationEvaluation, vk::ShaderStageFlagBits::eTessellationEvaluation));
		if (!shaderCode.geometry.empty())
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode.geometry, vk::ShaderStageFlagBits::eGeometry));
		if (!shaderCode.fragment.empty())
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode.fragment, vk::ShaderStageFlagBits::eFragment));
	}

	void VulkanShader::Destroy()
	{
		m_shaderStageCreateInfos.clear();

		for (auto shaderModule : m_shaderModules)
			m_device.destroyShaderModule(shaderModule);
		m_shaderModules.clear();
	}

	std::vector<vk::PipelineShaderStageCreateInfo> VulkanShader::GetShaderStageCreateInfos() const
	{
		return m_shaderStageCreateInfos;
	}

	vk::PipelineShaderStageCreateInfo VulkanShader::CreateShaderStage(const std::vector<char>& shaderCode, vk::ShaderStageFlagBits shaderStage)
	{
		vk::ShaderModule shaderModule;
		vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.pNext = nullptr;
		shaderModuleCreateInfo.flags = {};
		shaderModuleCreateInfo.codeSize = shaderCode.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
		vk::Result result = m_device.createShaderModule(&shaderModuleCreateInfo, nullptr, &shaderModule);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan vertex shader module!");

		m_shaderModules.push_back(shaderModule);

		vk::PipelineShaderStageCreateInfo shaderStageCreateInfo{};
		shaderStageCreateInfo.pNext = nullptr;
		shaderStageCreateInfo.flags = {};
		shaderStageCreateInfo.stage = shaderStage;
		shaderStageCreateInfo.module = shaderModule;
		shaderStageCreateInfo.pName = "main";
		shaderStageCreateInfo.pSpecializationInfo = nullptr;

		return shaderStageCreateInfo;
	}

	void VulkanShader::PrintShaderReflection(const std::vector<char>& shaderCode)
	{
		SpvReflectShaderModule module;
		SpvReflectResult result = spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &module);
		if (result != SPV_REFLECT_RESULT_SUCCESS) return;

		uint32_t inputVariablesCount = 0;
		result = spvReflectEnumerateInputVariables(&module, &inputVariablesCount, NULL);
		if (result != SPV_REFLECT_RESULT_SUCCESS) return;
		std::vector<SpvReflectInterfaceVariable*> inputVariables(inputVariablesCount);
		result = spvReflectEnumerateInputVariables(&module, &inputVariablesCount, inputVariables.data());
		if (result != SPV_REFLECT_RESULT_SUCCESS) return;

		uint32_t outputVariablesCount = 0;
		result = spvReflectEnumerateOutputVariables(&module, &outputVariablesCount, NULL);
		if (result != SPV_REFLECT_RESULT_SUCCESS) return;
		std::vector<SpvReflectInterfaceVariable*> outputVariables(outputVariablesCount);
		result = spvReflectEnumerateOutputVariables(&module, &outputVariablesCount, outputVariables.data());
		if (result != SPV_REFLECT_RESULT_SUCCESS) return;


		Logger::Debug("SPIRV-Reflect", "-----------------------------------");
		// PRINT MODULE
		std::string shaderStageName = "";
		switch (module.shader_stage)
		{
		case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
			shaderStageName = "Vertex Shader"; break;
		case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
			shaderStageName = "Fragment Shader"; break;
		case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
			shaderStageName = "Geometry Shader"; break;
		case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			shaderStageName = "Tesselation Control Shader"; break;
		case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			shaderStageName = "Tesselation Evaluation Shader"; break;
		case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
			shaderStageName = "Compute Shader"; break;
		}

		Logger::Debug("SPIRV-Reflect", "{0} Module: {1} (v{2})", shaderStageName, spvReflectSourceLanguage(module.source_language), module.source_language_version);
		// ---------------------------------
		// PRINT INPUT AND OUTPUT VARIABLES
		for (auto inputVariable : inputVariables)
		{
			if (inputVariable->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
				continue;
			Logger::Debug("SPIRV-Reflect", "in (location = {0}) {1} {2}", inputVariable->location, GetDataTypeName(*inputVariable->type_description), inputVariable->name);
		}
		for (auto outputVariable : outputVariables)
		{
			if (outputVariable->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
				continue;
			Logger::Debug("SPIRV-Reflect", "out (location = {0}) {1} {2}", outputVariable->location, GetDataTypeName(*outputVariable->type_description), outputVariable->name);
		}
		// ---------------------------------
		Logger::Debug("SPIRV-Reflect", "-----------------------------------");

		spvReflectDestroyShaderModule(&module);
	}

	std::string VulkanShader::GetDataTypeName(const SpvReflectTypeDescription& typeDescription)
	{
		std::string type = "unknown_type";
		switch (typeDescription.op)
		{
		case SpvOpTypeStruct:
			type = "struct"; break;
		case SpvOpTypeVoid:
			type = "void"; break;
		case SpvOpTypeBool:
			type = "bool"; break;
		case SpvOpTypeInt:
			if (typeDescription.traits.numeric.scalar.signedness)
				type = "int";
			else
				type = "uint";
			break;
		case SpvOpTypeFloat:
			switch (typeDescription.traits.numeric.scalar.width)
			{
			case 32:
				type = "float"; break;
			case 64:
				type = "double"; break;
			}
			break;
		case SpvOpTypeVector:
			switch (typeDescription.traits.numeric.scalar.width)
			{
			case 32:
				switch (typeDescription.traits.numeric.vector.component_count)
				{
				case 2:
					type = "vec2"; break;
				case 3:
					type = "vec3"; break;
				case 4:
					type = "vec4"; break;
				}
				break;

			case 64:
				switch (typeDescription.traits.numeric.vector.component_count)
				{
				case 2:
					type = "dvec2"; break;
				case 3:
					type = "dvec3"; break;
				case 4:
					type = "dvec4"; break;
				}
				break;
			}
			break;
		case SpvOpTypeMatrix:
			switch (typeDescription.traits.numeric.scalar.width)
			{
			case 32:
				switch (typeDescription.traits.numeric.matrix.column_count)
				{
				case 2:
					type = "mat2"; break;
				case 3:
					type = "mat3"; break;
				case 4:
					type = "mat4"; break;
				}
				break;

			case 64:
				switch (typeDescription.traits.numeric.matrix.column_count)
				{
				case 2:
					type = "dmat2"; break;
				case 3:
					type = "dmat3"; break;
				case 4:
					type = "dmat4"; break;
				}
				break;
			}
			break;
		}
		return type;
	}
}