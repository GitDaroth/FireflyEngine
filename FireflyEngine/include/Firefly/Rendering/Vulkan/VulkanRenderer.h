#pragma once

#include "Rendering/Renderer.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Material.h"
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

		void RecreateSwapchain();
		void CreateSwapchain();
		void DestroySwapchain();

		void AllocateCommandBuffers();
		void FreeCommandBuffers();

		void CreateColorImage();
		void DestroyColorImage();

		void CreateDepthImage();
		void DestroyDepthImage();

		void CreateRenderPass();
		void DestroyRenderPass();

		void CreateFramebuffers();
		void DestroyFramebuffers();

		void CreateSynchronizationPrimitivesForRendering();
		void DestroySynchronizationPrimitivesForRendering();

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
		
		std::shared_ptr<VulkanContext> m_vkContext;
		std::shared_ptr<VulkanDevice> m_device;
		std::shared_ptr<VulkanSwapchain> m_swapchain;

		std::unordered_map<std::string, vk::Pipeline> m_pipelines;
		std::unordered_map<std::string, vk::PipelineLayout> m_pipelineLayouts;

		vk::RenderPass m_renderPass;
		std::vector<vk::Framebuffer> m_framebuffers;

		vk::CommandPool m_commandPool;
		std::vector<vk::CommandBuffer> m_commandBuffers;

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

		vk::SampleCountFlagBits m_msaaSampleCount;

		vk::Image m_colorImage;
		vk::DeviceMemory m_colorImageMemory;
		vk::ImageView m_colorImageView;

		vk::Image m_depthImage;
		vk::DeviceMemory m_depthImageMemory;
		vk::ImageView m_depthImageView;
		vk::Format m_depthFormat;

		uint32_t m_currentFrameIndex = 0;
		uint32_t m_currentImageIndex = 0;
		std::vector<vk::Semaphore> m_isNewImageAvailableSemaphores;
		std::vector<vk::Semaphore> m_isRenderedImageAvailableSemaphores;
		std::vector<vk::Fence> m_isCommandBufferAvailableFences;

		std::vector<Entity> m_entities;
		std::vector<std::shared_ptr<Material>> m_materials;
		std::vector<size_t> m_entityMaterialIndices;
	};
}