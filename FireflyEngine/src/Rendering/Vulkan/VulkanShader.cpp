#include "pch.h"
#include "Rendering/Vulkan/VulkanShader.h"

#include <fstream>
#include <spirv_reflect.h>
#include "Rendering/Vulkan/VulkanMesh.h"

namespace Firefly
{
	void VulkanShader::Init(vk::Device device, const std::string& tag, const ShaderCodePath& shaderCodePath)
	{
		m_device = device;
		m_tag = tag;

		if (!shaderCodePath.vertexShaderPath.empty())
		{
			std::vector<char> shaderCode = ReadBinaryFile(shaderCodePath.vertexShaderPath);
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode, vk::ShaderStageFlagBits::eVertex));
		}
		if (!shaderCodePath.tesselationControlShaderPath.empty())
		{
			std::vector<char> shaderCode = ReadBinaryFile(shaderCodePath.tesselationControlShaderPath);
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode, vk::ShaderStageFlagBits::eTessellationControl));
		}
		if (!shaderCodePath.tesselationEvaluationShaderPath.empty())
		{
			std::vector<char> shaderCode = ReadBinaryFile(shaderCodePath.tesselationEvaluationShaderPath);
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode, vk::ShaderStageFlagBits::eTessellationEvaluation));
		}
		if (!shaderCodePath.geometryShaderPath.empty())
		{
			std::vector<char> shaderCode = ReadBinaryFile(shaderCodePath.geometryShaderPath);
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode, vk::ShaderStageFlagBits::eGeometry));
		}
		if (!shaderCodePath.fragmentShaderPath.empty())
		{
			std::vector<char> shaderCode = ReadBinaryFile(shaderCodePath.fragmentShaderPath);
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode, vk::ShaderStageFlagBits::eFragment));
		}
		if (!shaderCodePath.computeShaderPath.empty())
		{
			std::vector<char> shaderCode = ReadBinaryFile(shaderCodePath.computeShaderPath);
			m_shaderStageCreateInfos.push_back(CreateShaderStage(shaderCode, vk::ShaderStageFlagBits::eCompute));
		}
	}

	void VulkanShader::Destroy()
	{
		m_shaderStageCreateInfos.clear();

		for (auto shaderModule : m_shaderModules)
			m_device.destroyShaderModule(shaderModule);
		m_shaderModules.clear();
	}

	std::string VulkanShader::GetTag() const
	{
		return m_tag;
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

	std::vector<char> VulkanShader::ReadBinaryFile(const std::string& fileName)
	{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("Failed to open binary file!");

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> fileBytes(fileSize);
		file.seekg(0);
		file.read(fileBytes.data(), fileSize);
		file.close();

		return fileBytes;
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