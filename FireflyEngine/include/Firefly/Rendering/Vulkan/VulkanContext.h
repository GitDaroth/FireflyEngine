#pragma once

#include <vulkan/vulkan.hpp>

#include "Rendering/Mesh.h"

#include "Rendering/Vulkan/VulkanInstance.h"
#include "Rendering/Vulkan/VulkanDebugMessenger.h"
#include "Rendering/Vulkan/VulkanSurface.h"
#include "Rendering/Vulkan/VulkanDevice.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanMesh.h"
#include "Rendering/Vulkan/VulkanMaterial.h"
#include "Rendering/Vulkan/VulkanRenderObject.h"

namespace Firefly
{
	//struct UboPerFrame
	//{
	//	glm::mat4 viewMatrix;
	//	glm::mat4 projectionMatrix;
	//};

	//struct UboPerObject
	//{
	//	glm::mat4* modelMatrixData; // contains the model matrices of all objects in the scene
	//};

	class VulkanContext
	{
	private:
		VulkanContext();
		static VulkanContext* m_singleton;

	public:
		static VulkanContext* GetSingleton();

		void Init(void* window);
		void Destroy();

		void Draw();

		vk::CommandBuffer BeginOneTimeCommandBuffer();
		void EndCommandBuffer(vk::CommandBuffer commandBuffer);
		void CreateBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
		void CopyBuffer(vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size);
		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits sampleCount, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags imageUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Image& image, vk::DeviceMemory& imageMemory);
		vk::ImageView CreateImageView(vk::Image image, uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags imageAspectFlags);


	private:
		vk::PhysicalDevice PickPhysicalDevice();

		void RecreateSwapchain();

		void CreateCommandPool();
		void DestroyCommandPool();
		void AllocateCommandBuffers();
		void FreeCommandBuffers();

		// Per Shader
		//void CreateUniformBuffers();
		//void DestroyUniformBuffers();
		//void CreateDescriptorPool();
		//void DestroyDescriptorPool();
		//void AllocateDescriptorSets();
		//void FreeDescriptorSets();

		void CreateDepthImage();
		void DestroyDepthImage();

		void CreateRenderPass();
		void DestroyRenderPass();

		void CreateFramebuffers();
		void DestroyFramebuffers();

		//void CreateGraphicsPipeline();
		//void DestroyGraphicsPipeline();

		void CreateSynchronizationPrimitivesForRendering();
		void DestroySynchronizationPrimitivesForRendering();

		void TransitionImageLayout(vk::Image image, uint32_t mipLevels, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		vk::Format FindSupportedFormat(const std::vector<vk::Format>& formatCandidates, vk::ImageTiling tiling, vk::FormatFeatureFlags formatFeatureFlags);
		vk::Format FindDepthFormat();
		bool HasStencilComponent(vk::Format format);
		std::vector<const char*> GetRequiredInstanceExtensions() const;
		std::vector<const char*> GetRequiredInstanceLayers() const;
		std::vector<const char*> GetRequiredDeviceExtensions() const;
		std::vector<const char*> GetRequiredDeviceLayers() const;
		constexpr bool AreValidationLayersEnabled() const;
		static std::vector<char> ReadBinaryFile(const std::string& fileName);

		VulkanMesh* m_mesh;
		VulkanMaterial* m_material;
		std::vector<VulkanRenderObject*> m_renderObjects;
		//uint32_t m_objectCount = 10;
		//std::vector<glm::mat4> m_modelMatrices;

		VulkanInstance* m_instance;
		VulkanDebugMessenger* m_debugMessenger;
		VulkanSurface* m_surface;
		VulkanDevice* m_device;
		VulkanSwapchain* m_swapchain;

		vk::Image m_depthImage;
		vk::DeviceMemory m_depthImageMemory;
		vk::ImageView m_depthImageView;

		vk::RenderPass m_renderPass;
		std::vector<vk::Framebuffer> m_framebuffers;

		//vk::Pipeline m_graphicsPipeline;
		//vk::PipelineLayout m_pipelineLayout;

		vk::CommandPool m_commandPool;
		std::vector<vk::CommandBuffer> m_commandBuffers;

		//vk::DescriptorPool m_descriptorPool;
		//vk::DescriptorSetLayout m_descriptorSetLayout;
		//std::vector<vk::DescriptorSet> m_descriptorSets;

		//UboPerFrame m_uboPerFrame;
		//std::vector<vk::Buffer> m_uniformBuffersPerFrame;
		//std::vector<vk::DeviceMemory> m_uniformBufferMemoriesPerFrame;

		//size_t m_modelMatrixUniformAlignment;
		//UboPerObject m_uboPerObject;
		//std::vector<vk::Buffer> m_uniformBuffersPerObject;
		//std::vector<vk::DeviceMemory> m_uniformBufferMemoriesPerObject;

		uint32_t m_currentFrameIndex = 0;
		std::vector<vk::Semaphore> m_isImageAvailableSemaphore;
		std::vector<vk::Semaphore> m_isRenderingFinishedSemaphore;
		std::vector<vk::Fence> m_isCommandBufferFinishedFences;
	};
}