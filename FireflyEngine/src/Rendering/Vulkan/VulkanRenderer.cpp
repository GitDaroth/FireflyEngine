#include "pch.h"
#include "Rendering/Vulkan/VulkanRenderer.h"

#include "Core/ResourceRegistry.h"
#include "Rendering/RenderingAPI.h"
#include "Rendering/Vulkan/VulkanMesh.h"
#include "Rendering/Vulkan/VulkanShader.h"
#include "Rendering/Vulkan/VulkanMaterial.h"
#include "Rendering/Vulkan/VulkanUtils.h"
#include "Rendering/Vulkan/VulkanFrameBuffer.h"
#include "Rendering/Vulkan/VulkanRenderPass.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Components/MaterialComponent.h"
#include "Rendering/MeshGenerator.h"

namespace Firefly
{
	VulkanRenderer::VulkanRenderer()
	{
		m_vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
		m_device = m_vkContext->GetDevice();
		m_descriptorPool = m_vkContext->GetDescriptorPool();
	}

	void VulkanRenderer::Init()
	{
		CreateRenderPass();
		CreateFramebuffers();

		CreateUniformBuffers();
		CreateDescriptorSetLayouts();
		AllocateDescriptorSets();
		CreatePipelines();

		CreateScreenTexturePassResources();
	}

	void VulkanRenderer::Destroy()
	{
		m_device->WaitIdle();

		DestroyScreenTexturePassResources();

		DestroyPipelines();
		DestroyDescriptorSetLayouts();
		DestroyUniformBuffers();

		DestroyFramebuffers();
		DestroyRenderPass();
	}

	void VulkanRenderer::BeginDrawRecording()
	{
		m_entities.clear();
		m_materials.clear();
		m_entityMaterialIndices.clear();
	}

	void VulkanRenderer::RecordDraw(const Entity& entity)
	{
		if(entity.HasComponents<MeshComponent, MaterialComponent, TransformComponent>())
			m_entities.push_back(entity);
	}

	void VulkanRenderer::EndDrawRecording()
	{
		for (size_t i = 0; i < m_entities.size(); i++)
		{
			std::shared_ptr<Material> entityMaterial = m_entities[i].GetComponent<MaterialComponent>().m_material;
			bool foundMaterial = false;
			size_t entityMaterialIndex = 0;
			for (size_t j = 0; j < m_materials.size(); j++)
			{
				if (m_materials[j] == entityMaterial)
				{
					entityMaterialIndex = j;
					foundMaterial = true;
					break;
				}
			}
			if (!foundMaterial)
			{
				entityMaterialIndex = m_materials.size();
				m_materials.push_back(entityMaterial);
			}
			m_entityMaterialIndices.push_back(entityMaterialIndex);
		}
	}

	void VulkanRenderer::SubmitDraw(std::shared_ptr<Camera> camera)
	{
		if (m_vkContext->GetWidth() == 0 || m_vkContext->GetHeight() == 0)
		{
			m_device->WaitIdle();
			return;
		}

		if (!m_vkContext->AquireNextImage())
		{
			RecreateResources();
			return;
		}

		uint32_t currentImageIndex = m_vkContext->GetCurrentImageIndex();
		vk::CommandBuffer currentCommandBuffer = m_vkContext->GetCurrentCommandBuffer();

		m_mainRenderPass->Begin(m_mainFrameBuffers[currentImageIndex]);

		UpdateUniformBuffers(camera);

		for (size_t i = 0; i < m_entities.size(); i++)
		{
			std::shared_ptr<VulkanMaterial> material = std::dynamic_pointer_cast<VulkanMaterial>(m_materials[m_entityMaterialIndices[i]]);
			std::shared_ptr<VulkanMesh> mesh = std::dynamic_pointer_cast<VulkanMesh>(m_entities[i].GetComponent<MeshComponent>().m_mesh);
			std::string shaderTag = material->GetShader()->GetTag();

			currentCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines[shaderTag]);

			std::vector<vk::DescriptorSet> descriptorSets =
			{
				m_sceneDataDescriptorSets[currentImageIndex],
				m_materialDataDescriptorSets[currentImageIndex],
				material->GetTexturesDescriptorSet(),
				m_objectDataDescriptorSets[currentImageIndex]
			};
			std::vector<uint32_t> dynamicOffsets =
			{
				static_cast<uint32_t>(m_entityMaterialIndices[i] * m_materialDataDynamicAlignment),
				static_cast<uint32_t>(i * m_objectDataDynamicAlignment)
			};
			currentCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayouts[shaderTag], 0,
				descriptorSets.size(), descriptorSets.data(),
				dynamicOffsets.size(), dynamicOffsets.data());

			std::vector<vk::Buffer> vertexBuffers = { mesh->GetVertexBuffer() };
			vk::DeviceSize offsets[] = { 0 };
			currentCommandBuffer.bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets);
			currentCommandBuffer.bindIndexBuffer(mesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);

			currentCommandBuffer.drawIndexed(mesh->GetIndexCount(), 1, 0, 0, 0);
		}

		m_mainRenderPass->End();

		// RENDER RESOLVED COLOR TEXTURE TO SCREEN
		vk::ClearValue clearValue;
		clearValue.color = std::array<float, 4>{ 0.1f, 0.1f, 0.1f, 1.0f };

		vk::RenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_screenTextureRenderPass;
		renderPassBeginInfo.framebuffer = m_screenTextureFramebuffers[currentImageIndex];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = m_vkContext->GetSwapchain()->GetExtent();
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;
		currentCommandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

		currentCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_screenTexturePipeline);
		
		currentCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_screenTexturePipelineLayout, 0, 1, &m_screenTextureDescriptorSets[currentImageIndex], 0, nullptr);

		std::shared_ptr<VulkanMesh> quadMesh = std::dynamic_pointer_cast<VulkanMesh>(m_quadMesh);
		std::vector<vk::Buffer> vertexBuffers = { quadMesh->GetVertexBuffer() };
		vk::DeviceSize offsets[] = { 0 };
		currentCommandBuffer.bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets);
		currentCommandBuffer.bindIndexBuffer(quadMesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);

		currentCommandBuffer.drawIndexed(quadMesh->GetIndexCount(), 1, 0, 0, 0);

		currentCommandBuffer.endRenderPass();


		m_vkContext->SubmitCommands();

		if (!m_vkContext->PresentImage())
		{
			RecreateResources();
			return;
		}
	}

	void VulkanRenderer::UpdateUniformBuffers(std::shared_ptr<Camera> camera)
	{
		uint32_t currentImageIndex = m_vkContext->GetCurrentImageIndex();

		// Scene Data ---------
		glm::vec4 cameraPosition = glm::vec4(camera->GetPosition(), 1.0f);
		glm::mat4 viewMatrix = camera->GetViewMatrix();
		glm::mat4 projectionMatrix = camera->GetProjectionMatrix();
		projectionMatrix[1][1] *= -1; // Vulkan has inverted y axis in comparison to OpenGL

		SceneData sceneData;
		sceneData.viewMatrix = viewMatrix;
		sceneData.projectionMatrix = projectionMatrix;
		sceneData.viewProjectionMatrix = projectionMatrix * viewMatrix;
		sceneData.cameraPosition = cameraPosition;

		void* mappedMemory;
		m_device->GetDevice().mapMemory(m_sceneDataUniformBufferMemories[currentImageIndex], 0, sizeof(SceneData), {}, &mappedMemory);
		memcpy(mappedMemory, &sceneData, sizeof(SceneData));
		m_device->GetDevice().unmapMemory(m_sceneDataUniformBufferMemories[currentImageIndex]);
		// --------------------
		// Material Data ------
		for (size_t i = 0; i < m_materials.size(); i++)
		{
			MaterialData* materialData = (MaterialData*)((uint64_t)m_materialData + (i * m_materialDataDynamicAlignment));
			(*materialData).albedo = m_materials[i]->GetAlbedo();
			(*materialData).roughness = m_materials[i]->GetRoughness();
			(*materialData).metalness = m_materials[i]->GetMetalness();
			(*materialData).heightScale = m_materials[i]->GetHeightScale();
			(*materialData).hasAlbedoTexture = (float)m_materials[i]->IsTextureEnabled(Material::TextureUsage::Albedo);
			(*materialData).hasNormalTexture = (float)m_materials[i]->IsTextureEnabled(Material::TextureUsage::Normal);
			(*materialData).hasRoughnessTexture = (float)m_materials[i]->IsTextureEnabled(Material::TextureUsage::Roughness);
			(*materialData).hasMetalnessTexture = (float)m_materials[i]->IsTextureEnabled(Material::TextureUsage::Metalness);
			(*materialData).hasOcclusionTexture = (float)m_materials[i]->IsTextureEnabled(Material::TextureUsage::Occlusion);
			(*materialData).hasHeightTexture = (float)m_materials[i]->IsTextureEnabled(Material::TextureUsage::Height);
		}

		m_device->GetDevice().mapMemory(m_materialDataUniformBufferMemories[currentImageIndex], 0, m_materialDataCount * m_materialDataDynamicAlignment, {}, &mappedMemory);
		memcpy(mappedMemory, m_materialData, m_materialDataCount * m_materialDataDynamicAlignment);
		m_device->GetDevice().unmapMemory(m_materialDataUniformBufferMemories[currentImageIndex]);
		// --------------------
		// Object Data --------
		for (size_t i = 0; i < m_entities.size(); i++)
		{
			glm::mat4 modelMatrix = m_entities[i].GetComponent<TransformComponent>().m_transform;
			ObjectData* objectData = (ObjectData*)((uint64_t)m_objectData + (i * m_objectDataDynamicAlignment));
			(*objectData).modelMatrix = modelMatrix;
			(*objectData).normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(modelMatrix))));
		}

		m_device->GetDevice().mapMemory(m_objectDataUniformBufferMemories[currentImageIndex], 0, m_objectDataCount * m_objectDataDynamicAlignment, {}, &mappedMemory);
		memcpy(mappedMemory, m_objectData, m_objectDataCount * m_objectDataDynamicAlignment);
		m_device->GetDevice().unmapMemory(m_objectDataUniformBufferMemories[currentImageIndex]);
		// --------------------
	}

	void VulkanRenderer::RecreateResources()
	{
		m_device->WaitIdle();

		DestroyScreenTexturePassResources();
		DestroyPipelines();
		DestroyFramebuffers();

		CreateFramebuffers();
		CreatePipelines();
		CreateScreenTexturePassResources();
	}

	void VulkanRenderer::CreateRenderPass()
	{
		RenderPass::Description mainRenderPassDesc = {};
		mainRenderPassDesc.isDepthTestingEnabled = true;
		mainRenderPassDesc.depthCompareOperation = CompareOperation::LESS_OR_EQUAL;
		mainRenderPassDesc.isMultisamplingEnabled = true;
		mainRenderPassDesc.isSampleShadingEnabled = true;
		mainRenderPassDesc.minSampleShading = 1.0f;
		mainRenderPassDesc.colorAttachmentLayouts = { {Texture::Format::RGBA_8, m_msaaSampleCount} };
		mainRenderPassDesc.colorResolveAttachmentLayouts = { {Texture::Format::RGBA_8, Texture::SampleCount::SAMPLE_1} };
		mainRenderPassDesc.depthStencilAttachmentLayout = { Texture::Format::DEPTH_32_FLOAT, m_msaaSampleCount };
		m_mainRenderPass = RenderingAPI::CreateRenderPass(mainRenderPassDesc);
	}

	void VulkanRenderer::DestroyRenderPass()
	{
		m_mainRenderPass->Destroy();
	}

	void VulkanRenderer::CreateFramebuffers()
	{
		uint32_t width = m_vkContext->GetWidth();
		uint32_t height = m_vkContext->GetHeight();

		Texture::Description colorTextureDesc = {};
		colorTextureDesc.type = Texture::Type::TEXTURE_2D;
		colorTextureDesc.width = width;
		colorTextureDesc.height = height;
		colorTextureDesc.format = Texture::Format::RGBA_8;
		colorTextureDesc.sampleCount = m_msaaSampleCount;
		colorTextureDesc.useAsAttachment = true;
		colorTextureDesc.useSampler = false;
		m_colorTexture = std::dynamic_pointer_cast<VulkanTexture>(RenderingAPI::CreateTexture(colorTextureDesc));

		Texture::Description colorResolveTextureDesc = {};
		colorResolveTextureDesc.type = Texture::Type::TEXTURE_2D;
		colorResolveTextureDesc.width = width;
		colorResolveTextureDesc.height = height;
		colorResolveTextureDesc.format = Texture::Format::RGBA_8;
		colorResolveTextureDesc.sampleCount = Texture::SampleCount::SAMPLE_1;
		colorResolveTextureDesc.useAsAttachment = true;
		colorResolveTextureDesc.useSampler = true;
		colorResolveTextureDesc.sampler.isMipMappingEnabled = false;
		colorResolveTextureDesc.sampler.isAnisotropicFilteringEnabled = false;
		colorResolveTextureDesc.sampler.wrapMode = Texture::WrapMode::CLAMP_TO_EDGE;
		colorResolveTextureDesc.sampler.magnificationFilterMode = Texture::FilterMode::LINEAR;
		colorResolveTextureDesc.sampler.minificationFilterMode = Texture::FilterMode::LINEAR;
		for(size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
			m_colorResolveTextures.push_back(std::dynamic_pointer_cast<VulkanTexture>(RenderingAPI::CreateTexture(colorResolveTextureDesc)));

		Texture::Description depthTextureDesc = {};
		depthTextureDesc.type = Texture::Type::TEXTURE_2D;
		depthTextureDesc.width = width;
		depthTextureDesc.height = height;
		depthTextureDesc.format = Texture::Format::DEPTH_32_FLOAT;
		depthTextureDesc.sampleCount = m_msaaSampleCount;
		depthTextureDesc.useAsAttachment = true;
		depthTextureDesc.useSampler = false;
		m_depthTexture = std::dynamic_pointer_cast<VulkanTexture>(RenderingAPI::CreateTexture(depthTextureDesc));

		FrameBuffer::Attachment colorAttachment;
		colorAttachment.texture = m_colorTexture;

		FrameBuffer::Attachment depthAttachment;
		depthAttachment.texture = m_depthTexture;

		for (size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
		{
			FrameBuffer::Attachment colorResolveAttachment;
			colorResolveAttachment.texture = m_colorResolveTextures[i];

			FrameBuffer::Description frameBufferDesc = {};
			frameBufferDesc.width = width;
			frameBufferDesc.height = height;
			frameBufferDesc.colorAttachments = { colorAttachment };
			frameBufferDesc.colorResolveAttachments = { colorResolveAttachment };
			frameBufferDesc.depthStencilAttachment = { depthAttachment };
			m_mainFrameBuffers.push_back(RenderingAPI::CreateFrameBuffer(frameBufferDesc));
		}
	}

	void VulkanRenderer::DestroyFramebuffers()
	{
		for(auto frameBuffer : m_mainFrameBuffers)
			frameBuffer->Destroy();
		m_mainFrameBuffers.clear();

		m_depthTexture->Destroy();

		for(auto texture : m_colorResolveTextures)
			texture->Destroy();
		m_colorResolveTextures.clear();

		m_colorTexture->Destroy();
	}

	void VulkanRenderer::CreateUniformBuffers()
	{
		CreateSceneDataUniformBuffers();
		CreateMaterialDataUniformBuffers();
		CreateObjectDataUniformBuffers();
	}

	void VulkanRenderer::DestroyUniformBuffers()
	{
		for (size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
		{
			m_device->GetDevice().destroyBuffer(m_sceneDataUniformBuffers[i]);
			m_device->GetDevice().freeMemory(m_sceneDataUniformBufferMemories[i]);
			m_device->GetDevice().destroyBuffer(m_materialDataUniformBuffers[i]);
			m_device->GetDevice().freeMemory(m_materialDataUniformBufferMemories[i]);
			m_device->GetDevice().destroyBuffer(m_objectDataUniformBuffers[i]);
			m_device->GetDevice().freeMemory(m_objectDataUniformBufferMemories[i]);
		}
	}

	void VulkanRenderer::CreateDescriptorSetLayouts()
	{
		// SCENE DATA
		vk::DescriptorSetLayoutBinding sceneDataLayoutBinding{};
		sceneDataLayoutBinding.binding = 0;
		sceneDataLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		sceneDataLayoutBinding.descriptorCount = 1;
		sceneDataLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		sceneDataLayoutBinding.pImmutableSamplers = nullptr;

		std::vector<vk::DescriptorSetLayoutBinding> bindings = { sceneDataLayoutBinding };

		vk::DescriptorSetLayoutCreateInfo sceneDataDescriptorSetLayoutCreateInfo{};
		sceneDataDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
		sceneDataDescriptorSetLayoutCreateInfo.pBindings = bindings.data();

		vk::Result result = m_device->GetDevice().createDescriptorSetLayout(&sceneDataDescriptorSetLayoutCreateInfo, nullptr, &m_sceneDataDescriptorSetLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");

		// MATERIAL DATA
		vk::DescriptorSetLayoutBinding materialDataLayoutBinding{};
		materialDataLayoutBinding.binding = 0;
		materialDataLayoutBinding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		materialDataLayoutBinding.descriptorCount = 1;
		materialDataLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		materialDataLayoutBinding.pImmutableSamplers = nullptr;

		bindings = { materialDataLayoutBinding };

		vk::DescriptorSetLayoutCreateInfo materialDataDescriptorSetLayoutCreateInfo{};
		materialDataDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
		materialDataDescriptorSetLayoutCreateInfo.pBindings = bindings.data();

		result = m_device->GetDevice().createDescriptorSetLayout(&materialDataDescriptorSetLayoutCreateInfo, nullptr, &m_materialDataDescriptorSetLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");
		
		// MATERIAL TEXTURES
		vk::DescriptorSetLayoutBinding albedoTextureLayoutBinding{};
		albedoTextureLayoutBinding.binding = 0;
		albedoTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		albedoTextureLayoutBinding.descriptorCount = 1;
		albedoTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		albedoTextureLayoutBinding.pImmutableSamplers = nullptr;

		vk::DescriptorSetLayoutBinding normalTextureLayoutBinding{};
		normalTextureLayoutBinding.binding = 1;
		normalTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		normalTextureLayoutBinding.descriptorCount = 1;
		normalTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		normalTextureLayoutBinding.pImmutableSamplers = nullptr;

		vk::DescriptorSetLayoutBinding roughnessTextureLayoutBinding{};
		roughnessTextureLayoutBinding.binding = 2;
		roughnessTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		roughnessTextureLayoutBinding.descriptorCount = 1;
		roughnessTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		roughnessTextureLayoutBinding.pImmutableSamplers = nullptr;

		vk::DescriptorSetLayoutBinding metalnessTextureLayoutBinding{};
		metalnessTextureLayoutBinding.binding = 3;
		metalnessTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		metalnessTextureLayoutBinding.descriptorCount = 1;
		metalnessTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		metalnessTextureLayoutBinding.pImmutableSamplers = nullptr;

		vk::DescriptorSetLayoutBinding occlusionTextureLayoutBinding{};
		occlusionTextureLayoutBinding.binding = 4;
		occlusionTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		occlusionTextureLayoutBinding.descriptorCount = 1;
		occlusionTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		occlusionTextureLayoutBinding.pImmutableSamplers = nullptr;

		vk::DescriptorSetLayoutBinding heightTextureLayoutBinding{};
		heightTextureLayoutBinding.binding = 5;
		heightTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		heightTextureLayoutBinding.descriptorCount = 1;
		heightTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		heightTextureLayoutBinding.pImmutableSamplers = nullptr;

		bindings =
		{
			albedoTextureLayoutBinding,
			normalTextureLayoutBinding,
			roughnessTextureLayoutBinding,
			metalnessTextureLayoutBinding,
			occlusionTextureLayoutBinding,
			heightTextureLayoutBinding
		};

		std::vector<vk::DescriptorBindingFlags> bindingFlags(bindings.size(), vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
		vk::DescriptorSetLayoutBindingFlagsCreateInfo layoutBindingFlagsCreateInfo{};
		layoutBindingFlagsCreateInfo.pNext = nullptr;
		layoutBindingFlagsCreateInfo.bindingCount = bindingFlags.size();
		layoutBindingFlagsCreateInfo.pBindingFlags = bindingFlags.data();

		vk::DescriptorSetLayoutCreateInfo materialTexturesDescriptorSetLayoutCreateInfo{};
		materialTexturesDescriptorSetLayoutCreateInfo.pNext = &layoutBindingFlagsCreateInfo;
		materialTexturesDescriptorSetLayoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
		materialTexturesDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
		materialTexturesDescriptorSetLayoutCreateInfo.pBindings = bindings.data();

		result = m_device->GetDevice().createDescriptorSetLayout(&materialTexturesDescriptorSetLayoutCreateInfo, nullptr, &m_materialTexturesDescriptorSetLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");

		// OBJECT DATA
		vk::DescriptorSetLayoutBinding objectDataLayoutBinding{};
		objectDataLayoutBinding.binding = 0;
		objectDataLayoutBinding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		objectDataLayoutBinding.descriptorCount = 1;
		objectDataLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		objectDataLayoutBinding.pImmutableSamplers = nullptr;

		bindings = { objectDataLayoutBinding };

		vk::DescriptorSetLayoutCreateInfo objectDataDescriptorSetLayoutCreateInfo{};
		objectDataDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
		objectDataDescriptorSetLayoutCreateInfo.pBindings = bindings.data();

		result = m_device->GetDevice().createDescriptorSetLayout(&objectDataDescriptorSetLayoutCreateInfo, nullptr, &m_objectDataDescriptorSetLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");
	}

	void VulkanRenderer::DestroyDescriptorSetLayouts()
	{
		m_device->GetDevice().destroyDescriptorSetLayout(m_objectDataDescriptorSetLayout);
		m_device->GetDevice().destroyDescriptorSetLayout(m_materialDataDescriptorSetLayout);
		m_device->GetDevice().destroyDescriptorSetLayout(m_materialTexturesDescriptorSetLayout);
		m_device->GetDevice().destroyDescriptorSetLayout(m_sceneDataDescriptorSetLayout);
	}

	void VulkanRenderer::AllocateDescriptorSets()
	{
		AllocateSceneDataDescriptorSets();
		AllocateMaterialDataDescriptorSets();
		AllocateObjectDataDescriptorSets();
	}

	void VulkanRenderer::CreateSceneDataUniformBuffers()
	{
		size_t bufferSize = sizeof(SceneData);
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		m_sceneDataUniformBuffers.resize(m_vkContext->GetSwapchain()->GetImageCount());
		m_sceneDataUniformBufferMemories.resize(m_vkContext->GetSwapchain()->GetImageCount());
		for (size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
			VulkanUtils::CreateBuffer(m_device->GetDevice(), m_device->GetPhysicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags, m_sceneDataUniformBuffers[i], m_sceneDataUniformBufferMemories[i]);
	}

	void VulkanRenderer::CreateMaterialDataUniformBuffers()
	{
		size_t minUniformAlignment = m_device->GetPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
		m_materialDataDynamicAlignment = sizeof(SceneData);
		if (minUniformAlignment > 0)
			m_materialDataDynamicAlignment = (m_materialDataDynamicAlignment + minUniformAlignment - 1) & ~(minUniformAlignment - 1);

		size_t bufferSize = m_materialDataCount * m_materialDataDynamicAlignment; // TODO: grow/shrink dynamic buffer size dynamically
		m_materialData = (MaterialData*)_aligned_malloc(bufferSize, m_materialDataDynamicAlignment);
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		m_materialDataUniformBuffers.resize(m_vkContext->GetSwapchain()->GetImageCount());
		m_materialDataUniformBufferMemories.resize(m_vkContext->GetSwapchain()->GetImageCount());
		for (size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
			VulkanUtils::CreateBuffer(m_device->GetDevice(), m_device->GetPhysicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags, m_materialDataUniformBuffers[i], m_materialDataUniformBufferMemories[i]);

	}

	void VulkanRenderer::CreateObjectDataUniformBuffers()
	{
		size_t minUniformAlignment = m_device->GetPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
		m_objectDataDynamicAlignment = sizeof(SceneData);
		if (minUniformAlignment > 0)
			m_objectDataDynamicAlignment = (m_objectDataDynamicAlignment + minUniformAlignment - 1) & ~(minUniformAlignment - 1);

		size_t bufferSize = m_objectDataCount * m_objectDataDynamicAlignment; // TODO: grow/shrink dynamic buffer size dynamically
		m_objectData = (ObjectData*)_aligned_malloc(bufferSize, m_objectDataDynamicAlignment);
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		m_objectDataUniformBuffers.resize(m_vkContext->GetSwapchain()->GetImageCount());
		m_objectDataUniformBufferMemories.resize(m_vkContext->GetSwapchain()->GetImageCount());
		for (size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
			VulkanUtils::CreateBuffer(m_device->GetDevice(), m_device->GetPhysicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags, m_objectDataUniformBuffers[i], m_objectDataUniformBufferMemories[i]);
	}

	void VulkanRenderer::AllocateSceneDataDescriptorSets()
	{
		m_sceneDataDescriptorSets.resize(m_vkContext->GetSwapchain()->GetImageCount());
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(m_vkContext->GetSwapchain()->GetImageCount(), m_sceneDataDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = m_vkContext->GetSwapchain()->GetImageCount();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		vk::Result result = m_device->GetDevice().allocateDescriptorSets(&descriptorSetAllocateInfo, m_sceneDataDescriptorSets.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor sets!");

		for (size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
		{
			vk::DescriptorBufferInfo sceneDataDescriptorBufferInfo{};
			sceneDataDescriptorBufferInfo.buffer = m_sceneDataUniformBuffers[i];
			sceneDataDescriptorBufferInfo.offset = 0;
			sceneDataDescriptorBufferInfo.range = sizeof(SceneData);

			vk::WriteDescriptorSet sceneDataWriteDescriptorSet{};
			sceneDataWriteDescriptorSet.dstSet = m_sceneDataDescriptorSets[i];
			sceneDataWriteDescriptorSet.dstBinding = 0;
			sceneDataWriteDescriptorSet.dstArrayElement = 0;
			sceneDataWriteDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
			sceneDataWriteDescriptorSet.descriptorCount = 1;
			sceneDataWriteDescriptorSet.pBufferInfo = &sceneDataDescriptorBufferInfo;
			sceneDataWriteDescriptorSet.pImageInfo = nullptr;
			sceneDataWriteDescriptorSet.pTexelBufferView = nullptr;

			std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { sceneDataWriteDescriptorSet };

			m_device->GetDevice().updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void VulkanRenderer::AllocateMaterialDataDescriptorSets()
	{
		m_materialDataDescriptorSets.resize(m_vkContext->GetSwapchain()->GetImageCount());
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(m_vkContext->GetSwapchain()->GetImageCount(), m_materialDataDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = m_vkContext->GetSwapchain()->GetImageCount();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		vk::Result result = m_device->GetDevice().allocateDescriptorSets(&descriptorSetAllocateInfo, m_materialDataDescriptorSets.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor sets!");

		for (size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
		{
			vk::DescriptorBufferInfo materialDataDescriptorBufferInfo{};
			materialDataDescriptorBufferInfo.buffer = m_materialDataUniformBuffers[i];
			materialDataDescriptorBufferInfo.offset = 0;
			materialDataDescriptorBufferInfo.range = sizeof(MaterialData);

			vk::WriteDescriptorSet materialDataWriteDescriptorSet{};
			materialDataWriteDescriptorSet.dstSet = m_materialDataDescriptorSets[i];
			materialDataWriteDescriptorSet.dstBinding = 0;
			materialDataWriteDescriptorSet.dstArrayElement = 0;
			materialDataWriteDescriptorSet.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
			materialDataWriteDescriptorSet.descriptorCount = 1;
			materialDataWriteDescriptorSet.pBufferInfo = &materialDataDescriptorBufferInfo;
			materialDataWriteDescriptorSet.pImageInfo = nullptr;
			materialDataWriteDescriptorSet.pTexelBufferView = nullptr;

			std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { materialDataWriteDescriptorSet };

			m_device->GetDevice().updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void VulkanRenderer::AllocateObjectDataDescriptorSets()
	{
		m_objectDataDescriptorSets.resize(m_vkContext->GetSwapchain()->GetImageCount());
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(m_vkContext->GetSwapchain()->GetImageCount(), m_objectDataDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = m_vkContext->GetSwapchain()->GetImageCount();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		vk::Result result = m_device->GetDevice().allocateDescriptorSets(&descriptorSetAllocateInfo, m_objectDataDescriptorSets.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor sets!");

		for (size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
		{
			vk::DescriptorBufferInfo objectDataDescriptorBufferInfo{};
			objectDataDescriptorBufferInfo.buffer = m_objectDataUniformBuffers[i];
			objectDataDescriptorBufferInfo.offset = 0;
			objectDataDescriptorBufferInfo.range = sizeof(ObjectData);

			vk::WriteDescriptorSet objectDataWriteDescriptorSet{};
			objectDataWriteDescriptorSet.dstSet = m_objectDataDescriptorSets[i];
			objectDataWriteDescriptorSet.dstBinding = 0;
			objectDataWriteDescriptorSet.dstArrayElement = 0;
			objectDataWriteDescriptorSet.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
			objectDataWriteDescriptorSet.descriptorCount = 1;
			objectDataWriteDescriptorSet.pBufferInfo = &objectDataDescriptorBufferInfo;
			objectDataWriteDescriptorSet.pImageInfo = nullptr;
			objectDataWriteDescriptorSet.pTexelBufferView = nullptr;

			std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { objectDataWriteDescriptorSet };

			m_device->GetDevice().updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void VulkanRenderer::CreatePipelines()
	{
		std::vector<std::shared_ptr<Shader>> shaders = ShaderRegistry::Instance().GetAll();
		for (auto shader : shaders)
		{
			std::shared_ptr<VulkanShader> vkShader = std::dynamic_pointer_cast<VulkanShader>(shader);
			// VERTEX INPUT STATE --------------------------
			vk::VertexInputBindingDescription vertexInputBindingDescription{};
			vertexInputBindingDescription.binding = 0;
			vertexInputBindingDescription.stride = sizeof(Mesh::Vertex);
			vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

			std::array<vk::VertexInputAttributeDescription, 5> vertexInputAttributeDescriptions{};
			vertexInputAttributeDescriptions[0].binding = 0;
			vertexInputAttributeDescriptions[0].location = 0;
			vertexInputAttributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
			vertexInputAttributeDescriptions[0].offset = offsetof(Mesh::Vertex, position);
			vertexInputAttributeDescriptions[1].binding = 0;
			vertexInputAttributeDescriptions[1].location = 1;
			vertexInputAttributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
			vertexInputAttributeDescriptions[1].offset = offsetof(Mesh::Vertex, normal);
			vertexInputAttributeDescriptions[2].binding = 0;
			vertexInputAttributeDescriptions[2].location = 2;
			vertexInputAttributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
			vertexInputAttributeDescriptions[2].offset = offsetof(Mesh::Vertex, tangent);
			vertexInputAttributeDescriptions[3].binding = 0;
			vertexInputAttributeDescriptions[3].location = 3;
			vertexInputAttributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
			vertexInputAttributeDescriptions[3].offset = offsetof(Mesh::Vertex, bitangent);
			vertexInputAttributeDescriptions[4].binding = 0;
			vertexInputAttributeDescriptions[4].location = 4;
			vertexInputAttributeDescriptions[4].format = vk::Format::eR32G32Sfloat;
			vertexInputAttributeDescriptions[4].offset = offsetof(Mesh::Vertex, texCoords);

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
			vk::Extent2D extent;
			extent.width = m_mainFrameBuffers[0]->GetWidth();
			extent.height = m_mainFrameBuffers[0]->GetHeight();

			vk::Viewport viewport{};
			viewport.x = 0.f;
			viewport.y = 0.f;
			viewport.width = (float)extent.width;
			viewport.height = (float)extent.height;
			viewport.minDepth = 0.f;
			viewport.maxDepth = 1.f;

			vk::Rect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = extent;

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
			multisampleStateCreateInfo.rasterizationSamples = VulkanTexture::ConvertToVulkanSampleCount(m_mainRenderPass->GetSampleCount());
			multisampleStateCreateInfo.sampleShadingEnable = m_mainRenderPass->IsSampleShadingEnabled();
			multisampleStateCreateInfo.minSampleShading = m_mainRenderPass->GetMinSampleShading();
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
			depthStencilStateCreateInfo.depthTestEnable = m_mainRenderPass->isDepthTestingEnabled();
			depthStencilStateCreateInfo.depthWriteEnable = m_mainRenderPass->isDepthTestingEnabled();
			depthStencilStateCreateInfo.depthCompareOp = VulkanRenderPass::ConvertToVulkanCompareOperation(m_mainRenderPass->GetDepthCompareOperation());
			depthStencilStateCreateInfo.depthBoundsTestEnable = false;
			depthStencilStateCreateInfo.minDepthBounds = 0.0f;
			depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
			depthStencilStateCreateInfo.stencilTestEnable = false;
			depthStencilStateCreateInfo.front = {};
			depthStencilStateCreateInfo.back = {};
			// ---------------------------------------------
			// PIPELINE LAYOUT -----------------------------
			std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = 
			{ 
				m_sceneDataDescriptorSetLayout, 
				m_materialDataDescriptorSetLayout, 
				m_materialTexturesDescriptorSetLayout, 
				m_objectDataDescriptorSetLayout 
			};

			vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
			pipelineLayoutCreateInfo.pNext = nullptr;
			pipelineLayoutCreateInfo.flags = {};
			pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
			pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

			vk::PipelineLayout pipelineLayout;
			vk::Result result = m_device->GetDevice().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan pipeline layout!");
			// ---------------------------------------------
			// SHADER STAGE STATE --------------------------
			std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos = vkShader->GetShaderStageCreateInfos();
			// ---------------------------------------------
			// GRAPHICS PIPELINE ---------------------------
			vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
			pipelineCreateInfo.pNext = nullptr;
			pipelineCreateInfo.flags = {};
			pipelineCreateInfo.stageCount = shaderStageCreateInfos.size();
			pipelineCreateInfo.pStages = shaderStageCreateInfos.data();
			pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
			pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
			pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
			pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
			pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
			pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
			pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
			pipelineCreateInfo.pDynamicState = nullptr;
			pipelineCreateInfo.layout = pipelineLayout;
			pipelineCreateInfo.renderPass = std::dynamic_pointer_cast<VulkanRenderPass>(m_mainRenderPass)->GetHandle();
			pipelineCreateInfo.subpass = 0;
			pipelineCreateInfo.basePipelineHandle = nullptr;
			pipelineCreateInfo.basePipelineIndex = -1;

			vk::Pipeline pipeline;
			result = m_device->GetDevice().createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan graphics pipeline!");
			// ---------------------------------------------
			m_pipelineLayouts[vkShader->GetTag()] = pipelineLayout;
			m_pipelines[vkShader->GetTag()] = pipeline;
		}
	}

	void VulkanRenderer::DestroyPipelines()
	{
		for (auto pipelineEntry : m_pipelines)
			m_device->GetDevice().destroyPipeline(pipelineEntry.second);
		m_pipelines.clear();

		for(auto pipelineLayoutEntry : m_pipelineLayouts)
			m_device->GetDevice().destroyPipelineLayout(pipelineLayoutEntry.second);
		m_pipelineLayouts.clear();
	}

	void VulkanRenderer::CreateScreenTexturePassResources()
	{
		m_quadMesh = Firefly::MeshGenerator::CreateQuad(glm::vec2(2.0f));

		ShaderCode shaderCode = {};
		shaderCode.vertex = Shader::ReadShaderCodeFromFile("assets/shaders/Vulkan/screenTexture.vert.spv");
		shaderCode.fragment = Shader::ReadShaderCodeFromFile("assets/shaders/Vulkan/screenTexture.frag.spv");
		m_screenTextureShader = RenderingAPI::CreateShader("screenTexture", shaderCode);

		// RENDER PASS
		vk::AttachmentDescription colorAttachmentDescription{};
		colorAttachmentDescription.flags = {};
		colorAttachmentDescription.format = m_vkContext->GetSwapchain()->GetImageFormat();
		colorAttachmentDescription.samples = vk::SampleCountFlagBits::e1;
		colorAttachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachmentDescription.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		vk::AttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpassDescription.flags = {};
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentReference;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		vk::RenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.flags = {};
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 0;
		renderPassCreateInfo.pDependencies = nullptr;

		vk::Result result = m_device->GetDevice().createRenderPass(&renderPassCreateInfo, nullptr, &m_screenTextureRenderPass);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan render pass!");

		// FRAME BUFFERS
		vk::Extent2D swapchainExtent = m_vkContext->GetSwapchain()->GetExtent();
		m_screenTextureFramebuffers.resize(m_vkContext->GetSwapchain()->GetImageCount());
		for (size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
		{
			std::vector<vk::ImageView> attachments = { m_vkContext->GetSwapchain()->GetImageViews()[i] };
			vk::FramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.renderPass = m_screenTextureRenderPass;
			framebufferCreateInfo.attachmentCount = attachments.size();
			framebufferCreateInfo.pAttachments = attachments.data();
			framebufferCreateInfo.width = swapchainExtent.width;
			framebufferCreateInfo.height = swapchainExtent.height;
			framebufferCreateInfo.layers = 1;

			vk::Result result = m_device->GetDevice().createFramebuffer(&framebufferCreateInfo, nullptr, &m_screenTextureFramebuffers[i]);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan framebuffer!");
		}
		
		// DESCRIPTOR SETS
		vk::DescriptorSetLayoutBinding textureLayoutBinding{};
		textureLayoutBinding.binding = 0;
		textureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		textureLayoutBinding.descriptorCount = 1;
		textureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		textureLayoutBinding.pImmutableSamplers = nullptr;

		vk::DescriptorSetLayoutCreateInfo materialTexturesDescriptorSetLayoutCreateInfo{};
		materialTexturesDescriptorSetLayoutCreateInfo.pNext = nullptr;
		materialTexturesDescriptorSetLayoutCreateInfo.flags = {};
		materialTexturesDescriptorSetLayoutCreateInfo.bindingCount = 1;
		materialTexturesDescriptorSetLayoutCreateInfo.pBindings = &textureLayoutBinding;
		result = m_device->GetDevice().createDescriptorSetLayout(&materialTexturesDescriptorSetLayoutCreateInfo, nullptr, &m_screenTextureDescriptorSetLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");

		m_screenTextureDescriptorSets.resize(m_vkContext->GetSwapchain()->GetImageCount());
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(m_vkContext->GetSwapchain()->GetImageCount(), m_screenTextureDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = m_vkContext->GetSwapchain()->GetImageCount();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();
		result = m_device->GetDevice().allocateDescriptorSets(&descriptorSetAllocateInfo, m_screenTextureDescriptorSets.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor sets!");

		for (size_t i = 0; i < m_vkContext->GetSwapchain()->GetImageCount(); i++)
		{
			vk::DescriptorImageInfo descriptorImageInfo{};
			descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			descriptorImageInfo.imageView = m_colorResolveTextures[i]->GetImageView();
			descriptorImageInfo.sampler = m_colorResolveTextures[i]->GetSampler();

			vk::WriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.dstSet = m_screenTextureDescriptorSets[i];
			writeDescriptorSet.dstBinding = 0;
			writeDescriptorSet.dstArrayElement = 0;
			writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.pBufferInfo = nullptr;
			writeDescriptorSet.pImageInfo = &descriptorImageInfo;
			writeDescriptorSet.pTexelBufferView = nullptr;

			std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { writeDescriptorSet };
			m_device->GetDevice().updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}

		// PIPELINE
		std::shared_ptr<VulkanShader> vkShader = std::dynamic_pointer_cast<VulkanShader>(m_screenTextureShader);
		// VERTEX INPUT STATE --------------------------
		vk::VertexInputBindingDescription vertexInputBindingDescription{};
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(Mesh::Vertex);
		vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

		std::array<vk::VertexInputAttributeDescription, 5> vertexInputAttributeDescriptions{};
		vertexInputAttributeDescriptions[0].binding = 0;
		vertexInputAttributeDescriptions[0].location = 0;
		vertexInputAttributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[0].offset = offsetof(Mesh::Vertex, position);
		vertexInputAttributeDescriptions[1].binding = 0;
		vertexInputAttributeDescriptions[1].location = 1;
		vertexInputAttributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[1].offset = offsetof(Mesh::Vertex, normal);
		vertexInputAttributeDescriptions[2].binding = 0;
		vertexInputAttributeDescriptions[2].location = 2;
		vertexInputAttributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[2].offset = offsetof(Mesh::Vertex, tangent);
		vertexInputAttributeDescriptions[3].binding = 0;
		vertexInputAttributeDescriptions[3].location = 3;
		vertexInputAttributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[3].offset = offsetof(Mesh::Vertex, bitangent);
		vertexInputAttributeDescriptions[4].binding = 0;
		vertexInputAttributeDescriptions[4].location = 4;
		vertexInputAttributeDescriptions[4].format = vk::Format::eR32G32Sfloat;
		vertexInputAttributeDescriptions[4].offset = offsetof(Mesh::Vertex, texCoords);

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
		vk::Extent2D extent = m_vkContext->GetSwapchain()->GetExtent();

		vk::Viewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		vk::Rect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;

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
		multisampleStateCreateInfo.minSampleShading = 0.0f;
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
		depthStencilStateCreateInfo.depthTestEnable = false;
		depthStencilStateCreateInfo.depthWriteEnable = false;
		depthStencilStateCreateInfo.depthCompareOp = vk::CompareOp::eLess;
		depthStencilStateCreateInfo.depthBoundsTestEnable = false;
		depthStencilStateCreateInfo.minDepthBounds = 0.0f;
		depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
		depthStencilStateCreateInfo.stencilTestEnable = false;
		depthStencilStateCreateInfo.front = {};
		depthStencilStateCreateInfo.back = {};
		// ---------------------------------------------
		// PIPELINE LAYOUT -----------------------------
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = {};
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_screenTextureDescriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		result = m_device->GetDevice().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &m_screenTexturePipelineLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan pipeline layout!");
		// ---------------------------------------------
		// SHADER STAGE STATE --------------------------
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos = vkShader->GetShaderStageCreateInfos();
		// ---------------------------------------------
		// GRAPHICS PIPELINE ---------------------------
		vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.pNext = nullptr;
		pipelineCreateInfo.flags = {};
		pipelineCreateInfo.stageCount = shaderStageCreateInfos.size();
		pipelineCreateInfo.pStages = shaderStageCreateInfos.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
		pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
		pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
		pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
		pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
		pipelineCreateInfo.pDynamicState = nullptr;
		pipelineCreateInfo.layout = m_screenTexturePipelineLayout;
		pipelineCreateInfo.renderPass = m_screenTextureRenderPass;
		pipelineCreateInfo.subpass = 0;
		pipelineCreateInfo.basePipelineHandle = nullptr;
		pipelineCreateInfo.basePipelineIndex = -1;

		result = m_device->GetDevice().createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &m_screenTexturePipeline);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan graphics pipeline!");
		// ---------------------------------------------
	}

	void VulkanRenderer::DestroyScreenTexturePassResources()
	{
		m_device->GetDevice().destroyPipeline(m_screenTexturePipeline);
		m_device->GetDevice().destroyPipelineLayout(m_screenTexturePipelineLayout);
		m_device->GetDevice().destroyDescriptorSetLayout(m_screenTextureDescriptorSetLayout);

		for (const vk::Framebuffer& framebuffer : m_screenTextureFramebuffers)
			m_device->GetDevice().destroyFramebuffer(framebuffer);
		m_screenTextureFramebuffers.clear();

		m_device->GetDevice().destroyRenderPass(m_screenTextureRenderPass);
		m_screenTextureShader->Destroy();
		m_quadMesh->Destroy();
	}
}