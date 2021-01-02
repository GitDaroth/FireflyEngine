#pragma once

#include <vulkan/vulkan.hpp>

#include "Rendering/Mesh.h"

#include "Rendering/Vulkan/VulkanInstance.h"
#include "Rendering/Vulkan/VulkanDebugMessenger.h"
#include "Rendering/Vulkan/VulkanSurface.h"

namespace Firefly
{
	struct UboPerFrame
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};

	struct UboPerObject
	{
		glm::mat4* modelMatrixData; // contains the model matrices of all objects in the scene
	};

	class VulkanContext
	{
	public:
		VulkanContext(void* window);
		~VulkanContext();

		void Draw();

	private:
		//void CreateSurface();
		//void DestroySurface();

		void PickPhysicalDevice();
		void CreateDevice();
		void DestroyDevice();

		void CreateSwapchain();
		void RecreateSwapchain();
		void DestroySwapchain();

		void CreateCommandPool();
		void DestroyCommandPool();
		void AllocateCommandBuffers();
		void FreeCommandBuffers();

		void CreateVertexBuffers();
		void DestroyVertexBuffers();
		void CreateIndexBuffers();
		void DestroyIndexBuffers();

		void CreateUniformBuffers();
		void DestroyUniformBuffers();

		void CreateDescriptorPool();
		void DestroyDescriptorPool();
		void AllocateDescriptorSets();
		void FreeDescriptorSets();

		void CreateDepthImage();
		void DestroyDepthImage();

		void CreateRenderPass();
		void DestroyRenderPass();

		void CreateFramebuffers();
		void DestroyFramebuffers();

		void CreateGraphicsPipeline();
		void DestroyGraphicsPipeline();

		void CreateSynchronizationPrimitivesForRendering();
		void DestroySynchronizationPrimitivesForRendering();

		vk::CommandBuffer BeginOneTimeCommandBuffer();
		void EndCommandBuffer(vk::CommandBuffer commandBuffer);
		void CreateBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
		void CopyBuffer(vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size);
		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits sampleCount, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags imageUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Image& image, vk::DeviceMemory& imageMemory);
		vk::ImageView CreateImageView(vk::Image image, uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags imageAspectFlags);
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

		std::vector<Mesh::Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
		std::vector<glm::mat4> m_modelMatrices;

		VulkanInstance* m_instance;
		VulkanDebugMessenger* m_debugMessenger;
		VulkanSurface* m_surface;

		vk::PhysicalDevice m_physicalDevice;
		vk::Device m_device;
		uint32_t m_graphicsQueueFamilyIndex;
		uint32_t m_presentQueueFamilyIndex;

		vk::SwapchainKHR m_swapchain;
		vk::SurfaceFormatKHR m_swapchainSurfaceFormat;
		vk::PresentModeKHR m_swapchainPresentMode;
		vk::Extent2D m_swapchainExtent;
		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::ImageView> m_swapchainImageViews;

		vk::Image m_depthImage;
		vk::DeviceMemory m_depthImageMemory;
		vk::ImageView m_depthImageView;

		vk::RenderPass m_renderPass;
		std::vector<vk::Framebuffer> m_framebuffers;

		vk::Pipeline m_graphicsPipeline;
		vk::PipelineLayout m_pipelineLayout;

		vk::CommandPool m_commandPool;
		std::vector<vk::CommandBuffer> m_commandBuffers;

		vk::Buffer m_vertexBuffer;
		vk::DeviceMemory m_vertexBufferMemory;

		vk::Buffer m_indexBuffer;
		vk::DeviceMemory m_indexBufferMemory;

		vk::DescriptorPool m_descriptorPool;
		vk::DescriptorSetLayout m_descriptorSetLayout;
		std::vector<vk::DescriptorSet> m_descriptorSets;

		UboPerFrame m_uboPerFrame;
		std::vector<vk::Buffer> m_uniformBuffersPerFrame;
		std::vector<vk::DeviceMemory> m_uniformBufferMemoriesPerFrame;

		size_t m_modelMatrixUniformAlignment;
		UboPerObject m_uboPerObject;
		std::vector<vk::Buffer> m_uniformBuffersPerObject;
		std::vector<vk::DeviceMemory> m_uniformBufferMemoriesPerObject;

		uint32_t m_objectCount = 10;

		uint32_t m_currentFrameIndex = 0;
		std::vector<vk::Semaphore> m_isImageAvailableSemaphore;
		std::vector<vk::Semaphore> m_isRenderingFinishedSemaphore;
		std::vector<vk::Fence> m_isCommandBufferFinishedFences;
	};
}