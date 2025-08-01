#include "VulkanContext.h"
#include "pch.h"
#include "Core/Logging/Logger.h"

const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enable_validation_layers = false;
#else
constexpr bool enable_validation_layers = true;
#endif

VulkanContext::VulkanContext()
: m_VulkanDevice()
, m_VulkanSwapChain(m_VulkanDevice)
, m_VulkanGraphicsPipeline(m_VulkanSwapChain, m_VulkanDevice)
, m_VulkanRenderer(m_VulkanSwapChain, m_VulkanDevice, m_VulkanGraphicsPipeline) {}

void VulkanContext::Init(GLFWindow *window)
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface(window);
    m_VulkanDevice.PickPhysicalDevice(&m_Instance);
    m_VulkanDevice.CreateLogicalDevice(&m_Surface);
    m_VulkanSwapChain.CreateSwapchain(window->GetPixelResolution(), m_Surface);
    m_VulkanSwapChain.CreateImageViews();
    m_VulkanGraphicsPipeline.CreateGraphicsPipeline();
    m_VulkanRenderer.CreateCommandPool();
    m_VulkanRenderer.CreateCommandBuffer();
    m_VulkanRenderer.CreateSyncObjects();
}

void VulkanContext::Update()
{
    m_VulkanRenderer.DrawFrame();
}

void VulkanContext::WaitIdle()
{
    m_VulkanDevice.m_Device.waitIdle();
}

void VulkanContext::CreateInstance()
{
    constexpr vk::ApplicationInfo AppInfo {
        .pApplicationName = "Sample",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName = "Engine",
        .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion = vk::ApiVersion14
    };

    std::vector<char const*> requiredLayers;
    if (enable_validation_layers) {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }

    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enable_validation_layers) {
        requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    vk::InstanceCreateInfo CreateInfo {
        .pApplicationInfo = &AppInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data(),
    };
    m_Instance = vk::raii::Instance(m_Context, CreateInfo);
    CORE_LOG_INFO("Instance created.");
}

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
    std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
    return vk::False;
}

void VulkanContext::SetupDebugMessenger()
{
    if (!enable_validation_layers) {
        return;
    }
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT {
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &debugCallback
    };
    m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
    CORE_LOG_INFO("Setup debug messenger.");
}

void VulkanContext::CreateSurface(GLFWindow* window)
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(*m_Instance, window->GetWindowHandle(), nullptr, &surface) != 0) {
        CORE_LOG_ERROR("Failed to create window surface.");
        throw std::runtime_error("Failed to create window surface.");
    }
    m_Surface = vk::raii::SurfaceKHR(m_Instance, surface);
    CORE_LOG_INFO("Window surface created.");
}