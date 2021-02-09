#pragma once

#include "Rendering/Renderer.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/RenderPass.h"
#include "Rendering/Mesh.h"
#include "Rendering/Material.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanMesh.h"
#include <unordered_map>

namespace Firefly
{
	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer();

		virtual void Init() override;
		virtual void Destroy() override;

		virtual void BeginDrawRecording() override;
		virtual void RecordDraw(const Entity& entity) override;
		virtual void EndDrawRecording() override;
		virtual void SubmitDraw(std::shared_ptr<Camera> camera) override;

	private:
		void UpdateUniformBuffers(std::shared_ptr<Camera> camera);

		void RecreateResources();

		void CreateRenderPass();
		void DestroyRenderPass();

		void CreateFramebuffers();
		void DestroyFramebuffers();

		void CreateUniformBuffers();
		void DestroyUniformBuffers();
		void CreateDescriptorSetLayouts();
		void DestroyDescriptorSetLayouts();
		void AllocateDescriptorSets();

		void CreateSceneDataUniformBuffers();
		void CreateMaterialDataUniformBuffers();
		void CreateObjectDataUniformBuffers();

		void AllocateSceneDataDescriptorSets();
		void AllocateMaterialDataDescriptorSets();
		void AllocateObjectDataDescriptorSets();

		void CreatePipelines();
		void DestroyPipelines();

		void CreateScreenTexturePassResources();
		void DestroyScreenTexturePassResources();

		void CreatePBRShaderResources();
		void DestroyPBRShaderResources();

		void CreateImageBasedLightingResources();
		void DestroyImageBasedLightingResources();

		std::shared_ptr<VulkanContext> m_vkContext;
		std::shared_ptr<VulkanDevice> m_device;

		std::unordered_map<std::string, vk::Pipeline> m_pipelines;
		std::unordered_map<std::string, vk::PipelineLayout> m_pipelineLayouts;

		Texture::SampleCount m_msaaSampleCount = Texture::SampleCount::SAMPLE_4;
		std::shared_ptr<RenderPass> m_mainRenderPass;
		std::vector<std::shared_ptr<FrameBuffer>> m_mainFrameBuffers;
		std::shared_ptr<VulkanTexture> m_depthTexture;
		std::shared_ptr<VulkanTexture> m_colorTexture;
		std::vector<std::shared_ptr<VulkanTexture>> m_colorResolveTextures;

		vk::RenderPass m_screenTextureRenderPass;
		std::vector<vk::Framebuffer> m_screenTextureFramebuffers;
		vk::PipelineLayout m_screenTexturePipelineLayout;
		vk::Pipeline m_screenTexturePipeline;
		vk::DescriptorSetLayout m_screenTextureDescriptorSetLayout;
		std::vector<vk::DescriptorSet> m_screenTextureDescriptorSets;
		std::shared_ptr<Shader> m_screenTextureShader;
		std::shared_ptr<VulkanMesh> m_quadMesh;
		std::shared_ptr<VulkanMesh> m_cubeMesh;

		vk::DescriptorPool m_descriptorPool;

		vk::DescriptorSetLayout m_sceneDataDescriptorSetLayout;
		std::vector<vk::DescriptorSet> m_sceneDataDescriptorSets;
		std::vector<vk::Buffer> m_sceneDataUniformBuffers;
		std::vector<vk::DeviceMemory> m_sceneDataUniformBufferMemories;

		vk::DescriptorSetLayout m_materialDataDescriptorSetLayout;
		std::vector<vk::DescriptorSet> m_materialDataDescriptorSets;
		std::vector<vk::Buffer> m_materialDataUniformBuffers;
		std::vector<vk::DeviceMemory> m_materialDataUniformBufferMemories;
		MaterialData* m_materialData;
		size_t m_materialDataCount = 100;
		size_t m_materialDataDynamicAlignment;

		vk::DescriptorSetLayout m_materialTexturesDescriptorSetLayout;

		vk::DescriptorSetLayout m_objectDataDescriptorSetLayout;
		std::vector<vk::DescriptorSet> m_objectDataDescriptorSets;
		std::vector<vk::Buffer> m_objectDataUniformBuffers;
		std::vector<vk::DeviceMemory> m_objectDataUniformBufferMemories;
		ObjectData* m_objectData;
		size_t m_objectDataCount = 1000;
		size_t m_objectDataDynamicAlignment;

		std::vector<Entity> m_entities;
		std::vector<std::shared_ptr<Material>> m_materials;
		std::vector<size_t> m_entityMaterialIndices;

		std::shared_ptr<VulkanTexture> m_environmentCubeMap;
		std::shared_ptr<VulkanTexture> m_irradianceCubeMap;
		std::shared_ptr<VulkanTexture> m_prefilterCubeMap;
		std::shared_ptr<VulkanTexture> m_brdfLUT;

		std::shared_ptr<Shader> m_environmentMapShader;
		vk::PipelineLayout m_environmentMapPipelineLayout;
		vk::Pipeline m_environmentMapPipeline;
		vk::DescriptorSetLayout m_environmentMapDescriptorSetLayout;
		vk::DescriptorSet m_environmentMapDescriptorSet;
		vk::DescriptorSetLayout m_imageBasedLightingDescriptorSetLayout;
		vk::DescriptorSet m_imageBasedLightingDescriptorSet;
	};
}