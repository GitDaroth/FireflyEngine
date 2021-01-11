#pragma once

#include "Rendering/Renderer.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

#include "Rendering/Vulkan/VulkanMesh.h"
#include "Rendering/Vulkan/VulkanMaterial.h"
#include "Rendering/Vulkan/VulkanRenderObject.h"

namespace Firefly
{
	struct CameraData
	{
		glm::vec4 position;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::mat4 viewProjectionMatrix;
	};

	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer();

		virtual void Init(std::shared_ptr<GraphicsContext> context) override;
		virtual void Destroy() override;

		virtual void BeginDrawRecording() override;
		virtual void RecordDraw(const Entity& entity) override;
		virtual void EndDrawRecording() override;
		virtual void SubmitDraw(std::shared_ptr<Camera> camera) override;

	private:
		
		void RecreateSwapchain();
		void CreateSwapchain();
		void DestroySwapchain();

		void CreateCommandPool();
		void DestroyCommandPool();
		void AllocateCommandBuffers();
		void FreeCommandBuffers();

		void CreateCameraDataUniformBuffers();
		void DestroyCameraDataUniformBuffers();
		void CreateDescriptorPool();
		void DestroyDescriptorPool();
		void AllocateGlobalDescriptorSets();

		void CreateDepthImage();
		void DestroyDepthImage();

		void CreateRenderPass();
		void DestroyRenderPass();

		void CreateFramebuffers();
		void DestroyFramebuffers();

		void CreateSynchronizationPrimitivesForRendering();
		void DestroySynchronizationPrimitivesForRendering();
		
		std::shared_ptr<VulkanContext> m_context;
		std::shared_ptr<VulkanDevice> m_device;
		std::shared_ptr<VulkanSwapchain> m_swapchain;

		vk::RenderPass m_renderPass;
		std::vector<vk::Framebuffer> m_framebuffers;

		vk::CommandPool m_commandPool;
		std::vector<vk::CommandBuffer> m_commandBuffers;

		vk::DescriptorPool m_descriptorPool;
		std::vector<vk::DescriptorSet> m_globalDescriptorSets;
		std::vector<vk::Buffer> m_cameraDataUniformBuffers;
		std::vector<vk::DeviceMemory> m_cameraDataUniformBufferMemories;

		vk::Image m_depthImage;
		vk::DeviceMemory m_depthImageMemory;
		vk::ImageView m_depthImageView;
		vk::Format m_depthFormat;

		uint32_t m_currentFrameIndex = 0;
		uint32_t m_currentImageIndex = 0;
		std::vector<vk::Semaphore> m_isNewImageAvailableSemaphores;
		std::vector<vk::Semaphore> m_isRenderedImageAvailableSemaphores;
		std::vector<vk::Fence> m_isCommandBufferAvailableFences;


		VulkanMesh* m_armchairMesh;
		VulkanMesh* m_globeMesh;
		VulkanMesh* m_pistolMesh;
		VulkanMaterial* m_material;
		std::vector<VulkanRenderObject*> m_renderObjects;
	};
}