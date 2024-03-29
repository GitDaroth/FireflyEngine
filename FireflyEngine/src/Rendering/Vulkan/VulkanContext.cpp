#include "Rendering/Vulkan/VulkanContext.h"

#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Window/WindowsWindow.h"

namespace Firefly
{
    void VulkanContext::OnInit(std::shared_ptr<Window> window)
    {
        CreateInstance();
        if (AreValidationLayersEnabled())
            CreateDebugMessenger();
        CreateSurface();
        CreateDevice();
        CreateSwapchain();
        CreateCommandPool();
        AllocateCommandBuffers();
        CreateDescriptorPool();
        CreateSynchronizationPrimitives();

        PrintGpuInfo();
    }

    void VulkanContext::Destroy()
    {
        m_device->WaitIdle();

        DestroySynchronizationPrimitives();
        DestroyDescriptorPool();
        FreeCommandBuffers();
        DestroyCommandPool();
        DestroySwapchain();
        DestroyDevice();
        DestroySurface();
        if (AreValidationLayersEnabled())
            DestroyDebugMessenger();
        DestroyInstance();
    }

    bool VulkanContext::BeginScreenFrame()
    {
        vk::Result result = m_device->GetHandle().acquireNextImageKHR(m_swapchain->GetHandle(), UINT64_MAX, m_isNewImageAvailableSemaphores[m_currentFrameIndex], nullptr, &m_currentImageIndex);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
        {
            m_device->WaitIdle();
            DestroySwapchain();
            CreateSwapchain();
            return false;
        }
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to aquire next image from the swapchain!");

        // wait until the indexed command buffer is not used anymore before recording new commands to it
        m_device->GetHandle().waitForFences(1, &m_isScreenCommandBufferAvailableFences[m_currentImageIndex], true, UINT64_MAX);
        m_device->GetHandle().resetFences(1, &m_isScreenCommandBufferAvailableFences[m_currentImageIndex]);

        m_screenCommandBuffers[m_currentImageIndex].reset({});
        vk::CommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        result = m_screenCommandBuffers[m_currentImageIndex].begin(&commandBufferBeginInfo);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to begin recording Vulkan command buffer!");

        m_currentCommandBuffer = m_screenCommandBuffers[m_currentImageIndex];

        return true;
    }

    bool VulkanContext::EndScreenFrame()
    {
        m_screenCommandBuffers[m_currentImageIndex].end();

        vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput; // ColorAttachmentOutputStage waits for isImageAvailableSemaphore
        vk::SubmitInfo submitInfo{};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_isNewImageAvailableSemaphores[m_currentFrameIndex];
        submitInfo.pWaitDstStageMask = &waitStageMask;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_screenCommandBuffers[m_currentImageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_isRenderedImageAvailableSemaphores[m_currentFrameIndex];

        vk::Result result = m_device->GetGraphicsQueue().submit(1, &submitInfo, m_isScreenCommandBufferAvailableFences[m_currentImageIndex]);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to submit commands to the graphics queue!");

        vk::PresentInfoKHR presentInfo{};
        std::vector<vk::SwapchainKHR> swapchains = { m_swapchain->GetHandle() };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_isRenderedImageAvailableSemaphores[m_currentFrameIndex];
        presentInfo.swapchainCount = swapchains.size();
        presentInfo.pSwapchains = swapchains.data();
        presentInfo.pImageIndices = &m_currentImageIndex;
        presentInfo.pResults = nullptr;

        result = m_device->GetPresentQueue().presentKHR(&presentInfo);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
        {
            m_device->WaitIdle();
            DestroySwapchain();
            CreateSwapchain();
            return false;
        }
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to present the image with the present queue!");

        m_currentFrameIndex = (m_currentFrameIndex + 1) % m_swapchain->GetImageCount();

        return true;
    }

    void VulkanContext::BeginOffscreenFrame()
    {
        // wait until the offscreen command buffer is not used anymore before recording new commands to it
        m_device->GetHandle().waitForFences(1, &m_isOffscreenCommandBufferAvailableFence, true, UINT64_MAX);
        m_device->GetHandle().resetFences(1, &m_isOffscreenCommandBufferAvailableFence);

        m_offscreenCommandBuffer.reset({});
        vk::CommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        vk::Result result = m_offscreenCommandBuffer.begin(&commandBufferBeginInfo);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to begin recording Vulkan command buffer!");

        m_currentCommandBuffer = m_offscreenCommandBuffer;
    }

    void VulkanContext::EndOffscreenFrame()
    {
        m_offscreenCommandBuffer.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_offscreenCommandBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        vk::Result result = m_device->GetGraphicsQueue().submit(1, &submitInfo, m_isOffscreenCommandBufferAvailableFence);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to submit commands to the graphics queue!");
    }

    vk::CommandBuffer VulkanContext::GetCurrentCommandBuffer()
    {
        return m_currentCommandBuffer;
    }

    uint32_t VulkanContext::GetCurrentImageIndex() const
    {
        return m_currentImageIndex;
    }

    std::shared_ptr<VulkanDevice> VulkanContext::GetDevice() const
    {
        return m_device;
    }

    std::shared_ptr<VulkanSwapchain> VulkanContext::GetSwapchain() const
    {
        return m_swapchain;
    }

    vk::SurfaceKHR VulkanContext::GetSurface() const
    {
        return m_surface;
    }

    vk::CommandPool VulkanContext::GetCommandPool() const
    {
        return m_commandPool;
    }

    vk::DescriptorPool VulkanContext::GetDescriptorPool() const
    {
        return m_descriptorPool;
    }

    void VulkanContext::CreateInstance()
    {
        std::string appName = "Sandbox";
        uint32_t appVersion = VK_MAKE_VERSION(0, 0, 0);
        std::string engineName = FIREFLY_ENGINE_NAME;
        uint32_t engineVersion = VK_MAKE_VERSION(FIREFLY_ENGINE_VERSION_MAJOR, FIREFLY_ENGINE_VERSION_MINOR, FIREFLY_ENGINE_VERSION_PATCH);
        uint32_t apiVersion = VK_API_VERSION_1_2;

        vk::ApplicationInfo applicationInfo{};
        applicationInfo.pNext = nullptr;
        applicationInfo.pApplicationName = appName.c_str();
        applicationInfo.applicationVersion = appVersion;
        applicationInfo.pEngineName = engineName.c_str();
        applicationInfo.engineVersion = engineVersion;
        applicationInfo.apiVersion = apiVersion;

        // TODO: Check if required instance extensions and layers are supported
        std::vector<const char*> requiredInstanceExtensions = GetRequiredInstanceExtensions();
        std::vector<const char*> requiredInstanceLayers = GetRequiredInstanceLayers();

        vk::InstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.pNext = nullptr;
        instanceCreateInfo.flags = {};
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledExtensionCount = requiredInstanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();
        instanceCreateInfo.enabledLayerCount = requiredInstanceLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = requiredInstanceLayers.data();

        vk::Result result = vk::createInstance(&instanceCreateInfo, nullptr, &m_instance);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan instance!");
    }

    void VulkanContext::DestroyInstance()
    {
        m_instance.destroy();
    }

    void VulkanContext::CreateSurface()
    {
        std::shared_ptr<WindowsWindow> windowsWindow = std::dynamic_pointer_cast<WindowsWindow>(m_window);
        FIREFLY_ASSERT(windowsWindow, "Vulkan requires a Windows window!");
        GLFWwindow* glfwWindow = (GLFWwindow*)(windowsWindow->GetGlfwWindow());
        FIREFLY_ASSERT(glfwWindow, "Vulkan requires a GLFWwindow!");

        VkSurfaceKHR surface;
        vk::Result result = vk::Result(glfwCreateWindowSurface(m_instance, glfwWindow, nullptr, &surface));
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan window surface!");
        m_surface = vk::SurfaceKHR(surface);
    }

    void VulkanContext::DestroySurface()
    {
        m_instance.destroySurfaceKHR(m_surface);
    }

    void VulkanContext::CreateDebugMessenger()
    {
        vk::DebugUtilsMessengerCreateInfoEXT debugMessngerCreateInfo{};
        debugMessngerCreateInfo.pNext = nullptr;
        debugMessngerCreateInfo.flags = {};
        debugMessngerCreateInfo.pUserData = nullptr;
        debugMessngerCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        debugMessngerCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        debugMessngerCreateInfo.pfnUserCallback = DebugMessengerCallback;

        vk::DispatchLoaderDynamic dispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
        vk::Result result = m_instance.createDebugUtilsMessengerEXT(&debugMessngerCreateInfo, nullptr, &m_debugMessenger, dispatchLoaderDynamic);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan debug messenger!");
    }

    void VulkanContext::DestroyDebugMessenger()
    {
        vk::DispatchLoaderDynamic dispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
        m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, dispatchLoaderDynamic);
    }

    vk::PhysicalDevice VulkanContext::PickPhysicalDevice()
    {
        std::vector<vk::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();
        FIREFLY_ASSERT(physicalDevices.size() > 0, "Unable to find a graphics card with Vulkan support!");

        // TODO: Check required device extensions, layers, queue families and other requirements
        // TODO: Pick most suitable device

        return physicalDevices[0];
    }

    void VulkanContext::CreateDevice()
    {
        const std::vector<const char*>& requiredDeviceExtensions = GetRequiredDeviceExtensions();
        const std::vector<const char*>& requiredDeviceLayers = GetRequiredDeviceLayers();

        m_device = std::make_shared<VulkanDevice>();
        m_device->Init(PickPhysicalDevice(), m_surface, requiredDeviceExtensions, requiredDeviceLayers);
    }

    void VulkanContext::DestroyDevice()
    {
        m_device->Destroy();
    }

    void VulkanContext::CreateSwapchain()
    {
        m_swapchain = std::make_shared<VulkanSwapchain>();
        m_swapchain->Init(m_device->GetHandle(), m_device->GetPhysicalDevice(), m_surface, m_window->GetWidth(), m_window->GetHeight());
    }

    void VulkanContext::DestroySwapchain()
    {
        m_swapchain->Destroy();
    }

    void VulkanContext::CreateCommandPool()
    {
        vk::CommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.pNext = nullptr;
        commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        commandPoolCreateInfo.queueFamilyIndex = m_device->GetGraphicsQueueFamilyIndex();

        vk::Result result = m_device->GetHandle().createCommandPool(&commandPoolCreateInfo, nullptr, &m_commandPool);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan command pool!");
    }

    void VulkanContext::DestroyCommandPool()
    {
        m_device->GetHandle().destroyCommandPool(m_commandPool);
    }

    void VulkanContext::AllocateCommandBuffers()
    {
        m_screenCommandBuffers.resize(m_swapchain->GetImageCount());
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.pNext = nullptr;
        commandBufferAllocateInfo.commandPool = m_commandPool;
        commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        commandBufferAllocateInfo.commandBufferCount = m_screenCommandBuffers.size();

        vk::Result result = m_device->GetHandle().allocateCommandBuffers(&commandBufferAllocateInfo, m_screenCommandBuffers.data());
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan command buffers!");

        commandBufferAllocateInfo.pNext = nullptr;
        commandBufferAllocateInfo.commandPool = m_commandPool;
        commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        commandBufferAllocateInfo.commandBufferCount = 1;

        result = m_device->GetHandle().allocateCommandBuffers(&commandBufferAllocateInfo, &m_offscreenCommandBuffer);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan command buffers!");
    }

    void VulkanContext::FreeCommandBuffers()
    {
        m_device->GetHandle().freeCommandBuffers(m_commandPool, 1, &m_offscreenCommandBuffer);
        m_device->GetHandle().freeCommandBuffers(m_commandPool, m_screenCommandBuffers.size(), m_screenCommandBuffers.data());
    }

    void VulkanContext::CreateDescriptorPool()
    {
        vk::DescriptorPoolSize uniformBufferDescriptorPoolSize{};
        uniformBufferDescriptorPoolSize.type = vk::DescriptorType::eUniformBuffer;
        uniformBufferDescriptorPoolSize.descriptorCount = 100;

        vk::DescriptorPoolSize uniformBufferDynamicDescriptorPoolSize{};
        uniformBufferDynamicDescriptorPoolSize.type = vk::DescriptorType::eUniformBufferDynamic;
        uniformBufferDynamicDescriptorPoolSize.descriptorCount = 100;

        vk::DescriptorPoolSize imageSamplerDescriptorPoolSize{};
        imageSamplerDescriptorPoolSize.type = vk::DescriptorType::eCombinedImageSampler;
        imageSamplerDescriptorPoolSize.descriptorCount = 100;

        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes =
        {
            uniformBufferDescriptorPoolSize,
            uniformBufferDynamicDescriptorPoolSize,
            imageSamplerDescriptorPoolSize
        };

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.pNext = nullptr;
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind; // Needed in order to update textures on the fly
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = 1000;

        vk::Result result = m_device->GetHandle().createDescriptorPool(&descriptorPoolCreateInfo, nullptr, &m_descriptorPool);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan descriptor pool!");
    }

    void VulkanContext::DestroyDescriptorPool()
    {
        m_device->GetHandle().resetDescriptorPool(m_descriptorPool, {});
        m_device->GetHandle().destroyDescriptorPool(m_descriptorPool);
    }

    void VulkanContext::CreateSynchronizationPrimitives()
    {
        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = {};

        vk::FenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        m_isNewImageAvailableSemaphores.resize(m_swapchain->GetImageCount());
        m_isRenderedImageAvailableSemaphores.resize(m_swapchain->GetImageCount());
        m_isScreenCommandBufferAvailableFences.resize(m_swapchain->GetImageCount());
        for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
        {
            vk::Result result = m_device->GetHandle().createSemaphore(&semaphoreCreateInfo, nullptr, &m_isNewImageAvailableSemaphores[i]);
            FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Semaphore!");

            result = m_device->GetHandle().createSemaphore(&semaphoreCreateInfo, nullptr, &m_isRenderedImageAvailableSemaphores[i]);
            FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Semaphore!");

            result = m_device->GetHandle().createFence(&fenceCreateInfo, nullptr, &m_isScreenCommandBufferAvailableFences[i]);
            FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Fence!");
        }

        vk::Result result = m_device->GetHandle().createFence(&fenceCreateInfo, nullptr, &m_isOffscreenCommandBufferAvailableFence);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Fence!");
    }

    void VulkanContext::DestroySynchronizationPrimitives()
    {
        m_device->GetHandle().destroyFence(m_isOffscreenCommandBufferAvailableFence);

        for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
        {
            m_device->GetHandle().destroyFence(m_isScreenCommandBufferAvailableFences[i]);
            m_device->GetHandle().destroySemaphore(m_isRenderedImageAvailableSemaphores[i]);
            m_device->GetHandle().destroySemaphore(m_isNewImageAvailableSemaphores[i]);
        }
    }

    void VulkanContext::PrintGpuInfo()
    {
        vk::PhysicalDeviceProperties deviceProperties = m_device->GetPhysicalDevice().getProperties();

        std::string vendorName = "Unknown Vendor";
        switch (deviceProperties.vendorID)
        {
        case 0x1002:
            vendorName = "AMD";
            break;
        case 0x1010:
            vendorName = "ImgTec";
            break;
        case 0x10DE:
            vendorName = "NVIDIA";
            break;
        case 0x13B5:
            vendorName = "ARM";
            break;
        case 0x5143:
            vendorName = "Qualcomm";
            break;
        case 0x8086:
            vendorName = "INTEL";
            break;
        }

        Logger::Info("Vulkan", "API Version: {0}.{1}.{2}", VK_VERSION_MAJOR(deviceProperties.apiVersion), VK_VERSION_MINOR(deviceProperties.apiVersion), VK_VERSION_PATCH(deviceProperties.apiVersion));
        Logger::Info("Vulkan", "{0} {1}", vendorName, deviceProperties.deviceName);
    }

    std::vector<const char*> VulkanContext::GetRequiredInstanceExtensions() const
    {
        std::vector<const char*> requiredInstanceExtensions;
        if (AreValidationLayersEnabled())
            requiredInstanceExtensions.push_back("VK_EXT_debug_utils");

        uint32_t glfwInstanceExtensionCount = 0;
        const char** glfwInstanceExtensionNames;
        glfwInstanceExtensionNames = glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionCount);
        for (size_t i = 0; i < glfwInstanceExtensionCount; i++)
            requiredInstanceExtensions.push_back(glfwInstanceExtensionNames[i]);

        return requiredInstanceExtensions;
    }

    std::vector<const char*> VulkanContext::GetRequiredInstanceLayers() const
    {
        std::vector<const char*> requiredInstanceLayers;
        if (AreValidationLayersEnabled())
            requiredInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");

        return requiredInstanceLayers;
    }

    std::vector<const char*> VulkanContext::GetRequiredDeviceExtensions() const
    {
        std::vector<const char*> requiredDeviceExtensions;
        requiredDeviceExtensions.push_back("VK_KHR_swapchain");

        return requiredDeviceExtensions;
    }

    std::vector<const char*> VulkanContext::GetRequiredDeviceLayers() const
    {
        std::vector<const char*> requiredDeviceLayers;
        return requiredDeviceLayers;
    }

    constexpr bool VulkanContext::AreValidationLayersEnabled() const
    {
#ifdef NDEBUG
        return false;
#else 
        return true;
#endif
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        std::string messageTypeLabel;
        switch (messageType)
        {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            messageTypeLabel = "general";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            messageTypeLabel = "validation";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            messageTypeLabel = "performance";
            break;
        default:
            messageTypeLabel = "unknown";
            break;
        }

        std::string message = "[" + messageTypeLabel + "] debug message: " + std::string(pCallbackData->pMessage);
        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            Logger::Warn("Vulkan", message);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            Logger::Error("Vulkan", message);
            break;
        }
        return VK_FALSE;
    }
}