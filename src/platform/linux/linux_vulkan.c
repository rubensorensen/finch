#include "linux_vulkan.h"
#include "finch/log/log.h"

#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>

#include <stdlib.h>

static VkInstance               vulkan_instance;
static VkPhysicalDevice         vulkan_physical_device;
static VkDevice                 vulkan_device;
static VkQueue                  vulkan_graphics_queue;
static VkDebugUtilsMessengerEXT vulkan_debug_messenger;

static const char* instance_extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};
static const u32 instance_extensions_count =
    sizeof(instance_extensions) / sizeof(instance_extensions[0]);

static const char* validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};
static const u32 validation_layers_count =
    sizeof(validation_layers) / sizeof(validation_layers[0]);

static void vulkan_result_to_string(char* buf, VkResult result)
{
    switch (result)
    {
        case VK_SUCCESS: {
            string_copy(buf, "Success");
        } break;
        case VK_NOT_READY: {
            string_copy(buf, "A fence or query has not yet completed");
        } break;
        case VK_TIMEOUT: {
            string_copy(buf, "A wait operation has not completed in the specified time");
        } break;
        case VK_EVENT_SET: {
            string_copy(buf, "An event is signaled");
        } break;
        case VK_EVENT_RESET: {
            string_copy(buf, "An event is unsignaled");
        } break;
        case VK_INCOMPLETE: {
            string_copy(buf, "A string_copy(buf, array was too small for the result");
        } break;
        case VK_ERROR_OUT_OF_HOST_MEMORY: {
            string_copy(buf, "A host memory allocation has failed");
        } break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: {
            string_copy(buf, "A device memory allocation has failed");
        } break;
        case VK_ERROR_INITIALIZATION_FAILED: {
            string_copy(buf, "Initialization of an object could not be completed for implementation-specific reasons");
        } break;
        case VK_ERROR_DEVICE_LOST: {
            string_copy(buf, "The logical or physical device has been lost");
        } break;
        case VK_ERROR_MEMORY_MAP_FAILED: {
            string_copy(buf, "Mapping of a memory object has failed");
        } break;
        case VK_ERROR_LAYER_NOT_PRESENT: {
            string_copy(buf, "A requested layer is not present or could not be loaded");
        } break;
        case VK_ERROR_EXTENSION_NOT_PRESENT: {
            string_copy(buf, "A requested extension is not supported");
        } break;
        case VK_ERROR_FEATURE_NOT_PRESENT: {
            string_copy(buf, "A requested feature is not supported");
        } break;
        case VK_ERROR_INCOMPATIBLE_DRIVER: {
            string_copy(buf, "The requested version of Vulkan is not supported by the driver or is otherwise incompatible");
        } break;
        case VK_ERROR_TOO_MANY_OBJECTS: {
            string_copy(buf, "Too many objects of the type have already been created");
        } break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED: {
            string_copy(buf, "A requested format is not supported on this device");
        } break;
        case VK_ERROR_SURFACE_LOST_KHR: {
            string_copy(buf, "A surface is no longer available");
        } break;
        case VK_SUBOPTIMAL_KHR: {
            string_copy(buf, "A swapchain no longer matches the surface properties exactly, but can still be used");
        } break;
        case VK_ERROR_OUT_OF_DATE_KHR: {
            string_copy(buf, "A surface has changed in such a way that it is no longer compatible with the swapchain");
        } break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: {
            string_copy(buf, "The display used by a swapchain does not use the same presentable image layout");
        } break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: {
            string_copy(buf, "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API");
        } break;
        case VK_ERROR_VALIDATION_FAILED_EXT: {
            string_copy(buf, "A validation layer found an error");
        } break;
        default: {
            string_copy(buf, "ERROR: UNKNOWN VULKAN ERROR");
        }
    }
}

VkResult create_debug_utils_messenger_EXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* create_info,
                                      const VkAllocationCallbacks* allocator,
                                      VkDebugUtilsMessengerEXT* debug_messenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func == NULL) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return func(instance, create_info, allocator, debug_messenger);
}

void destroy_debug_utils_messenger_EXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debug_messenger,
                                       const VkAllocationCallbacks* allocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debug_messenger, allocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    // Unused variables
    (void)messageType;
    (void)pUserData;
    
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            FC_TRACE("<VULKAN VALIDATION LAYER> %s", pCallbackData->pMessage);
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            FC_INFO("<VULKAN VALIDATION LAYER> %s", pCallbackData->pMessage);
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            FC_WARN("<VULKAN VALIDATION LAYER> %s", pCallbackData->pMessage);
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            FC_ERROR("<VULKAN VALIDATION LAYER> %s", pCallbackData->pMessage);
        } break;
        default: {
            FC_ERROR("<VULKAN VALIDATION LAYER (UNKNOWN SEVERITY)> %s",
                     pCallbackData->pMessage);
        }
    }


    return VK_FALSE;
}

void vulkan_populate_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT* create_info)
{
    create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info->messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info->pfnUserCallback = vulkan_debug_callback;
}

void vulkan_init_debug_messenger(void)
{
    VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
    vulkan_populate_debug_messenger(&create_info);

    VkResult result = create_debug_utils_messenger_EXT(vulkan_instance, &create_info,
                                                       NULL, &vulkan_debug_messenger);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("failed to set up debug messenger: %s", buf);
    }
}

b32 check_instance_extension_support() {
    u32 extension_count;
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    VkExtensionProperties* supported_extensions =
        (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extension_count);
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, supported_extensions);

    FC_TRACE("Supported vulkan instance extensions:");
    for (u32 i = 0; i < extension_count; ++i) {
        FC_TRACE("|-- %s", supported_extensions[i].extensionName);
    }
    
    for (u32 i = 0; i < instance_extensions_count; ++i) {
        b32 extension_found = false;
        for (u32 j = 0; j < extension_count; ++j) {
            if (string_compare((char*)instance_extensions[i],
                               supported_extensions[j].extensionName)) {
                extension_found = true;
                break;
            }
        }

        if (!extension_found) {
            return false;
        }
    }
    
    return true;
}

b32 check_validation_layer_support() {
    u32 layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    VkLayerProperties* supported_layers =
        (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, supported_layers);

    FC_TRACE("Supported vulkan validation layers:");
    for (u32 i = 0; i < layer_count; ++i) {
        FC_TRACE("|-- %s", supported_layers[i].layerName);
    }
    
    for (u32 i = 0; i < validation_layers_count; ++i) {
        b32 layer_found = false;
        for (u32 j = 0; j < layer_count; ++j) {
            if (string_compare((char*)validation_layers[i], supported_layers[j].layerName)) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) {
            return false;
        }
    }
    
    return true;
}

static void vulkan_create_instance(X11State* x11_state)
{
    if (!check_instance_extension_support()) {
        FC_ERROR("Instance extensions requested, but not supported");
    }
    
    if (!check_validation_layer_support()) {
        FC_ERROR("Validation layers requested, but not supported");
    }
    
    VkApplicationInfo app_info  = {0};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = x11_state->window_attributes.title;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "Finch";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_0;

    
    VkInstanceCreateInfo create_info    = {0};
    create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo        = &app_info;
    create_info.enabledExtensionCount   = instance_extensions_count;
    create_info.ppEnabledExtensionNames = (const char* const*)instance_extensions;
    create_info.enabledLayerCount       = validation_layers_count;
    create_info.ppEnabledLayerNames     = (const char* const*)validation_layers;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
    vulkan_populate_debug_messenger(&debug_create_info);
    create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
    
    VkResult result = vkCreateInstance(&create_info, NULL, &vulkan_instance);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Could not create vulkan instance: %s", buf);
    }
}

static VulkanQueueFamilyIndices vulkan_find_queue_families(VkPhysicalDevice device)
{
    VulkanQueueFamilyIndices indices;

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device,
                                             &queue_family_count, NULL);
    VkQueueFamilyProperties* queue_families =
        (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device,
                                             &queue_family_count, queue_families);

    for (u32 i = 0; i < queue_family_count; ++i) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.has_graphics_family = true;
            indices.graphics_family = i;
        }
    }
    
    free(queue_families);
    
    return indices;
}

static b32 vulkan_is_device_suitable(VkPhysicalDevice device)
{
    VulkanQueueFamilyIndices indices = vulkan_find_queue_families(device);
    
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device, &device_features);

    b32 gpu_is_discreet =
        device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    b32 gpu_supports_geometry_shaders = device_features.geometryShader;
    b32 gpu_has_graphics_queue_family = indices.has_graphics_family;
    
    return (gpu_is_discreet &&
            gpu_supports_geometry_shaders &&
            gpu_has_graphics_queue_family);
}

static void vulkan_pick_physical_device(void)
{
    vulkan_physical_device = VK_NULL_HANDLE;
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, NULL);
    if (device_count == 0) {
        FC_ERROR("Failed to find GPUs with vulkan support!");
    }

    VkPhysicalDevice* physical_devices =
        (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * device_count);
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, physical_devices);

    for (u32 i = 0; i < device_count; ++i) {
        if (vulkan_is_device_suitable(physical_devices[i])) {
            vulkan_physical_device = physical_devices[i];
            break;
        }
    }
    free(physical_devices);
    
    if (vulkan_physical_device == VK_NULL_HANDLE) {
        FC_ERROR("Failed to find a suitable GPU");
    }

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(vulkan_physical_device, &device_properties);
    FC_INFO("Picked physical device: %s", device_properties.deviceName);
}

static void vulkan_create_logical_device(void)
{
    VulkanQueueFamilyIndices indices =
        vulkan_find_queue_families(vulkan_physical_device);
    
    VkDeviceQueueCreateInfo queue_create_info = {0};
    queue_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex        = indices.graphics_family;
    queue_create_info.queueCount              = 1;

    f32 queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features = {0};

    VkDeviceCreateInfo create_info    = {0};
    create_info.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos     = &queue_create_info;
    create_info.queueCreateInfoCount  = 1;
    create_info.pEnabledFeatures      = &device_features;
    create_info.enabledExtensionCount = 0;
    create_info.enabledLayerCount     = validation_layers_count;
    create_info.ppEnabledLayerNames   = validation_layers;

    VkResult result = vkCreateDevice(vulkan_physical_device, &create_info,
                                     NULL, &vulkan_device);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to create logical device: %s", buf);
    }

    vkGetDeviceQueue(vulkan_device, indices.graphics_family, 0, &vulkan_graphics_queue);
}

void x11_vulkan_init(X11State* x11_state)
{
    FC_INFO("Initializing Vulkan");
    vulkan_create_instance(x11_state);
    vulkan_init_debug_messenger();
    vulkan_pick_physical_device();
    vulkan_create_logical_device();
    FC_INFO("Vulkan initialized");
}

void x11_vulkan_deinit(X11State* x11_state)
{
    FC_INFO("Deinitializing Vulkan");
    (void)x11_state;
    destroy_debug_utils_messenger_EXT(vulkan_instance, vulkan_debug_messenger, NULL);
    vkDestroyDevice(vulkan_device, NULL);
    vkDestroyInstance(vulkan_instance, NULL);
    FC_INFO("Vulkan deinitialized");
}
