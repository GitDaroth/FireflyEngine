#include "pch.h"
#include "Rendering/Vulkan/VulkanMaterial.h"

#include <fstream>
#include "Rendering/Vulkan/VulkanMesh.h"

namespace Firefly
{
	VulkanMaterial::VulkanMaterial(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, vk::Device device, vk::RenderPass renderPass, vk::Extent2D swapchainExtent) :
		m_device(device)
	{
		m_shaderStageCreateInfos.push_back(CreateShaderStage(vertexShaderPath, vk::ShaderStageFlagBits::eVertex));
		m_shaderStageCreateInfos.push_back(CreateShaderStage(fragmentShaderPath, vk::ShaderStageFlagBits::eFragment));

		CreatePipeline(renderPass, swapchainExtent);
	}

	VulkanMaterial::~VulkanMaterial()
	{
		DestroyPipeline();
	}

	vk::Pipeline VulkanMaterial::GetPipeline() const
	{
		return m_pipeline;
	}

	vk::PipelineLayout VulkanMaterial::GetPipelineLayout() const
	{
		return m_pipelineLayout;
	}

	void VulkanMaterial::CreatePipeline(vk::RenderPass renderPass, vk::Extent2D swapchainExtent)
	{
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
		pipelineLayoutCreateInfo.setLayoutCount = 0;
		pipelineLayoutCreateInfo.pSetLayouts = nullptr;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

		vk::Result result = m_device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
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
	}

	vk::PipelineShaderStageCreateInfo VulkanMaterial::CreateShaderStage(const std::string& shaderPath, vk::ShaderStageFlagBits shaderStage)
	{
		std::vector<char> shaderCode = ReadBinaryFile(shaderPath);
		vk::ShaderModule shaderModule;
		vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.pNext = nullptr;
		shaderModuleCreateInfo.flags = {};
		shaderModuleCreateInfo.codeSize = shaderCode.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
		vk::Result result = m_device.createShaderModule(&shaderModuleCreateInfo, nullptr, &shaderModule);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan vertex shader module!");

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
		FIREFLY_ASSERT(file.is_open(), "Failed to open binary file: {0}!", fileName);

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> fileBytes(fileSize);
		file.seekg(0);
		file.read(fileBytes.data(), fileSize);
		file.close();

		return fileBytes;
	}
}