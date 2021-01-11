#include "pch.h"
#include "Rendering/Vulkan/VulkanMaterial.h"

#include <fstream>
#include <spirv_reflect.h>
#include "Rendering/Vulkan/VulkanMesh.h"
#include "Rendering/Vulkan/VulkanContext.h"

namespace Firefly
{
	VulkanMaterial::VulkanMaterial(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, vk::Device device, vk::RenderPass renderPass, VulkanSwapchain* swapchain) :
		m_device(device)
	{
		std::vector<char> vertexShaderCode = ReadBinaryFile(vertexShaderPath);
		std::vector<char> fragmentShaderCode = ReadBinaryFile(fragmentShaderPath);
		
		PrintShaderReflection(vertexShaderCode);
		PrintShaderReflection(fragmentShaderCode);

		m_shaderStageCreateInfos.push_back(CreateShaderStage(vertexShaderCode, vk::ShaderStageFlagBits::eVertex));
		m_shaderStageCreateInfos.push_back(CreateShaderStage(fragmentShaderCode, vk::ShaderStageFlagBits::eFragment));

		CreatePipeline(renderPass, swapchain);
	}

	VulkanMaterial::~VulkanMaterial()
	{
		DestroyPipeline();
	
		for (auto shaderModule : m_shaderModules)
			m_device.destroyShaderModule(shaderModule);
		m_shaderModules.clear();
	}

	vk::Pipeline VulkanMaterial::GetPipeline() const
	{
		return m_pipeline;
	}

	vk::PipelineLayout VulkanMaterial::GetPipelineLayout() const
	{
		return m_pipelineLayout;
	}

	vk::DescriptorSetLayout VulkanMaterial::GetGlobalDescriptorSetLayout() const
	{
		return m_globalDescriptorSetLayout;
	}

	void VulkanMaterial::CreatePipeline(vk::RenderPass renderPass, VulkanSwapchain* swapchain)
	{
		// DESCRIPTOR LAYOUT
		vk::DescriptorSetLayoutBinding cameraDataLayoutBinding{};
		cameraDataLayoutBinding.binding = 0;
		cameraDataLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		cameraDataLayoutBinding.descriptorCount = 1;
		cameraDataLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		cameraDataLayoutBinding.pImmutableSamplers = nullptr;

		std::vector<vk::DescriptorSetLayoutBinding> bindings = { cameraDataLayoutBinding };

		vk::DescriptorSetLayoutCreateInfo globalDescriptorSetLayoutCreateInfo{};
		globalDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
		globalDescriptorSetLayoutCreateInfo.pBindings = bindings.data();

		vk::Result result = m_device.createDescriptorSetLayout(&globalDescriptorSetLayoutCreateInfo, nullptr, &m_globalDescriptorSetLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");

		// VERTEX INPUT STATE --------------------------
		vk::VertexInputBindingDescription vertexInputBindingDescription{};
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(VulkanMesh::Vertex);
		vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

		std::array<vk::VertexInputAttributeDescription, 5> vertexInputAttributeDescriptions{};
		vertexInputAttributeDescriptions[0].binding = 0;
		vertexInputAttributeDescriptions[0].location = 0;
		vertexInputAttributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[0].offset = offsetof(VulkanMesh::Vertex, position);
		vertexInputAttributeDescriptions[1].binding = 0;
		vertexInputAttributeDescriptions[1].location = 1;
		vertexInputAttributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[1].offset = offsetof(VulkanMesh::Vertex, normal);
		vertexInputAttributeDescriptions[2].binding = 0;
		vertexInputAttributeDescriptions[2].location = 2;
		vertexInputAttributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[2].offset = offsetof(VulkanMesh::Vertex, tangent);
		vertexInputAttributeDescriptions[3].binding = 0;
		vertexInputAttributeDescriptions[3].location = 3;
		vertexInputAttributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[3].offset = offsetof(VulkanMesh::Vertex, bitangent);
		vertexInputAttributeDescriptions[4].binding = 0;
		vertexInputAttributeDescriptions[4].location = 4;
		vertexInputAttributeDescriptions[4].format = vk::Format::eR32G32Sfloat;
		vertexInputAttributeDescriptions[4].offset = offsetof(VulkanMesh::Vertex, texCoords);

		vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
		vertexInputStateCreateInfo.pNext = nullptr;
		vertexInputStateCreateInfo.flags = {};
		vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
		vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
		vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
		// ---------------------------------------------
		// INPUT ASSEMBLY STATE ------------------------
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
		inputAssemblyStateCreateInfo.pNext = nullptr;
		inputAssemblyStateCreateInfo.flags = {};
		inputAssemblyStateCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssemblyStateCreateInfo.primitiveRestartEnable = false;
		// ---------------------------------------------
		// VIEWPORT STATE ------------------------------
		vk::Extent2D swapchainExtent = swapchain->GetExtent();
		vk::Viewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = (float)swapchainExtent.width;
		viewport.height = (float)swapchainExtent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		vk::Rect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchainExtent;

		vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.pNext = nullptr;
		viewportStateCreateInfo.flags = {};
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = &viewport;
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = &scissor;
		// ---------------------------------------------
		// RASTERIZATION STATE -------------------------
		vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
		rasterizationStateCreateInfo.pNext = nullptr;
		rasterizationStateCreateInfo.flags = {};
		rasterizationStateCreateInfo.depthClampEnable = false;
		rasterizationStateCreateInfo.rasterizerDiscardEnable = false;
		rasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
		rasterizationStateCreateInfo.lineWidth = 1.f;
		rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
		rasterizationStateCreateInfo.frontFace = vk::FrontFace::eCounterClockwise;
		rasterizationStateCreateInfo.depthBiasEnable = false;
		rasterizationStateCreateInfo.depthBiasConstantFactor = 0.f;
		rasterizationStateCreateInfo.depthBiasClamp = 0.f;
		rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.f;
		rasterizationStateCreateInfo.depthClampEnable = false;
		// ---------------------------------------------
		// MULTISAMPLE STATE ---------------------------
		vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
		multisampleStateCreateInfo.pNext = nullptr;
		multisampleStateCreateInfo.flags = {};
		multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisampleStateCreateInfo.sampleShadingEnable = false;
		multisampleStateCreateInfo.minSampleShading = 1.0f;
		multisampleStateCreateInfo.pSampleMask = nullptr;
		multisampleStateCreateInfo.alphaToCoverageEnable = false;
		multisampleStateCreateInfo.alphaToOneEnable = false;
		// ---------------------------------------------
		// COLOR BLEND STATE ---------------------------
		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{};
		colorBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachmentState.blendEnable = false;
		colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachmentState.alphaBlendOp = vk::BlendOp::eAdd;

		vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
		colorBlendStateCreateInfo.logicOpEnable = false;
		colorBlendStateCreateInfo.logicOp = vk::LogicOp::eCopy;
		colorBlendStateCreateInfo.attachmentCount = 1;
		colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
		colorBlendStateCreateInfo.blendConstants[0] = 0.f;
		colorBlendStateCreateInfo.blendConstants[1] = 0.f;
		colorBlendStateCreateInfo.blendConstants[2] = 0.f;
		colorBlendStateCreateInfo.blendConstants[3] = 0.f;
		// ---------------------------------------------
		// DEPTH STENCIL STATE -------------------------
		vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
		depthStencilStateCreateInfo.pNext = nullptr;
		depthStencilStateCreateInfo.flags = {};
		depthStencilStateCreateInfo.depthTestEnable = true;
		depthStencilStateCreateInfo.depthWriteEnable = true;
		depthStencilStateCreateInfo.depthCompareOp = vk::CompareOp::eLess;
		depthStencilStateCreateInfo.depthBoundsTestEnable = false;
		depthStencilStateCreateInfo.minDepthBounds = 0.0f;
		depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
		depthStencilStateCreateInfo.stencilTestEnable = false;
		depthStencilStateCreateInfo.front = {};
		depthStencilStateCreateInfo.back = {};
		// ---------------------------------------------
		// PIPELINE LAYOUT -----------------------------
		vk::PushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4);
		pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = {};
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_globalDescriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

		result = m_device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan pipeline layout!");
		// ---------------------------------------------
		// GRAPHICS PIPELINE ---------------------------
		vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.pNext = nullptr;
		pipelineCreateInfo.flags = {};
		pipelineCreateInfo.stageCount = m_shaderStageCreateInfos.size();
		pipelineCreateInfo.pStages = m_shaderStageCreateInfos.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
		pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
		pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
		pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
		pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
		pipelineCreateInfo.pDynamicState = nullptr;
		pipelineCreateInfo.layout = m_pipelineLayout;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.subpass = 0;
		pipelineCreateInfo.basePipelineHandle = nullptr;
		pipelineCreateInfo.basePipelineIndex = -1;

		result = m_device.createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan graphics pipeline!");
		// ---------------------------------------------
	}

	void VulkanMaterial::DestroyPipeline()
	{
		m_device.destroyPipeline(m_pipeline);
		m_device.destroyPipelineLayout(m_pipelineLayout);

		m_device.destroyDescriptorSetLayout(m_globalDescriptorSetLayout);
	}

	vk::PipelineShaderStageCreateInfo VulkanMaterial::CreateShaderStage(const std::vector<char>& shaderCode, vk::ShaderStageFlagBits shaderStage)
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

	std::vector<char> VulkanMaterial::ReadBinaryFile(const std::string& fileName)
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

	void VulkanMaterial::PrintShaderReflection(const std::vector<char>& shaderCode)
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

	std::string VulkanMaterial::GetDataTypeName(const SpvReflectTypeDescription& typeDescription)
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