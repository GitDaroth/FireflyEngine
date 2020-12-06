#include <vulkan/vulkan.hpp>

#include "Rendering/Mesh.h"

struct GLFWwindow;

namespace Firefly
{
	class VulkanContext
	{
	public:
		VulkanContext(void* window);
		~VulkanContext();

		void Draw();

	private:
		void CreateInstance();
		void DestroyInstance();

		void CreateDebugMessenger();
		void DestroyDebugMessenger();

		void CreateSurface();
		void DestroySurface();

		void PickPhysicalDevice();
		void CreateDevice();
		void DestroyDevice();

		void CreateSwapchain();
		void RecreateSwapchain();
		void DestroySwapchain();

		void CreateDynamicUniformBuffer();
		void DestroyDynamicUniformBuffer();

		void CreateDescriptorPool();
		void DestroyDescriptorPool();
		void AllocateDescriptorSets();
		void FreeDescriptorSets();

		void CreateRenderPass();
		void DestroyRenderPass();
		void CreateGraphicsPipeline();
		void DestroyGraphicsPipeline();

		void CreateCommandPool();
		void DestroyCommandPool();
		void AllocateCommandBuffers();
		void FreeCommandBuffers();

		void CreateSynchronizationPrimitivesForRendering();
		void DestroySynchronizationPrimitivesForRendering();

		void CreateVertexBuffers();
		void DestroyVertexBuffers();
		void CreateIndexBuffers();
		void DestroyIndexBuffers();

		vk::CommandBuffer BeginOneTimeCommandBuffer();
		void EndCommandBuffer(vk::CommandBuffer commandBuffer);
		void CreateBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
		void CopyBuffer(vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size);
		vk::ImageView CreateImageView(vk::Image image, uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags imageAspectFlags);
		std::vector<const char*> GetRequiredInstanceExtensions() const;
		std::vector<const char*> GetRequiredInstanceLayers() const;
		std::vector<const char*> GetRequiredDeviceExtensions() const;
		std::vector<const char*> GetRequiredDeviceLayers() const;
		constexpr bool AreValidationLayersEnabled() const;
		static std::vector<char> ReadBinaryFile(const std::string& fileName);

		GLFWwindow* m_glfwWindow;

		vk::Instance m_instance;
		vk::SurfaceKHR m_surface;

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

		vk::RenderPass m_renderPass;
		std::vector<vk::Framebuffer> m_framebuffers;

		vk::Pipeline m_graphicsPipeline;
		vk::PipelineLayout m_pipelineLayout;

		vk::CommandPool m_commandPool;
		std::vector<vk::CommandBuffer> m_commandBuffers;

		std::vector<Mesh::Vertex> m_vertices;
		vk::Buffer m_vertexBuffer;
		vk::DeviceMemory m_vertexBufferMemory;

		std::vector<uint32_t> m_indices;
		vk::Buffer m_indexBuffer;
		vk::DeviceMemory m_indexBufferMemory;

		vk::DescriptorPool m_descriptorPool;
		vk::DescriptorSetLayout m_descriptorSetLayout;
		std::vector<vk::DescriptorSet> m_descriptorSets;

		std::vector<glm::mat4> m_modelMatrices;
		size_t m_modelMatrixUniformAlignment;
		glm::mat4* m_modelMatrixUniformData;
		std::vector<vk::Buffer> m_uniformBuffers;
		std::vector<vk::DeviceMemory> m_uniformBufferMemories;

		uint32_t m_instanceCount = 10;

		uint32_t m_currentFrameIndex = 0;
		std::vector<vk::Semaphore> m_isImageAvailableSemaphore;
		std::vector<vk::Semaphore> m_isRenderingFinishedSemaphore;
		std::vector<vk::Fence> m_isCommandBufferFinishedFences;

		vk::DebugUtilsMessengerEXT m_debugMessenger;
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
	};
}