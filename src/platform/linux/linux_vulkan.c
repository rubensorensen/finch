#include "linux_vulkan.h"
#include "finch/log/log.h"
#include "finch/utils/utils.h"

#include <stdlib.h>

static VkInstance               vulkan_instance;
static VkSurfaceKHR             vulkan_surface;
static VkPhysicalDevice         vulkan_physical_device;
static VkDevice                 vulkan_device;
static VkQueue                  vulkan_graphics_queue;
static VkQueue                  vulkan_present_queue;
static VkSwapchainKHR           vulkan_swap_chain;
static VkImage                  vulkan_swap_chain_images[100];
static u32                      vulkan_swap_chain_images_count;
static VkFormat                 vulkan_swap_chain_image_format;
static VkExtent2D               vulkan_swap_chain_extent;
static VkDebugUtilsMessengerEXT vulkan_debug_messenger;

static const char* instance_extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};
static const u32 instance_extensions_count =
    sizeof(instance_extensions) / sizeof(instance_extensions[0]);

static const char* device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static const u32 device_extensions_count =
    sizeof(device_extensions) / sizeof(device_extensions[0]);

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


b32 vulkan_queue_family_indices_is_complete(VulkanQueueFamilyIndices* indices)
{
    return (indices->has_graphics_family &&
            indices->has_present_family);
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

static void vulkan_create_surface(X11State* x11_state)
{
    VkXlibSurfaceCreateInfoKHR create_info = {0};
    create_info.sType                      = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    create_info.dpy                        = x11_state->display;
    create_info.window                     = x11_state->window;

    VkResult result = vkCreateXlibSurfaceKHR(vulkan_instance, &create_info,
                                             NULL, &vulkan_surface);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Could not create vulkan surface: %s", buf);
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
            indices.graphics_family_index = i;
        }
        
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i,
                                             vulkan_surface, &present_support);
        if (present_support) {
            indices.has_present_family = true;
            indices.present_family_index = i;
        }
    }
    
    free(queue_families);
    
    return indices;
}

b32 vulkan_check_device_extension_support(VkPhysicalDevice device)
{
    u32 extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
    VkExtensionProperties* supported_extensions =
        (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extension_count);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, supported_extensions);

    FC_TRACE("Supported device extensions:");
    for (u32 i = 0; i < extension_count; ++i) {
        FC_TRACE("|-- %s", supported_extensions[i].extensionName);
    }
    
    for (u32 i = 0; i < device_extensions_count; ++i) {
        b32 extension_found = false;
        for (u32 j = 0; j < extension_count; ++j) {
            if (string_compare((char*)device_extensions[i],
                               supported_extensions[j].extensionName)) {
                extension_found = true;
                break;
            }
        }

        if (!extension_found) {
            return false;
        }
    }

    free(supported_extensions);
    return true;
}

static VulkanSwapChainSupportDetails vulkan_query_swap_chain_support(VkPhysicalDevice device)
{
    VulkanSwapChainSupportDetails details = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkan_surface, &details.capabilities);
    
    u32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_surface,
                                         &format_count, NULL);
    if (format_count != 0) {
        // TODO: Assert that format_count is not greater than max format count size
        details.format_count = format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_surface,
                                             &format_count, details.formats);
    }

    u32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan_surface,
                                              &present_mode_count, NULL);
    if (present_mode_count != 0) {
        // TODO: Assert that present_mode_count is not greater than max present_mode_count size
        details.present_mode_count = present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan_surface,
                                                  &present_mode_count, details.present_modes);
    }
    
    return details;
}

static VkSurfaceFormatKHR vulkan_choose_swap_surface_format(VkSurfaceFormatKHR* available_formats, u32 available_formats_count)
{
    for (u32 i = 0; i < available_formats_count; ++i) {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_formats[i];
        }
    }

    return available_formats[0];
}

static VkPresentModeKHR vulkan_choose_swap_present_mode(VkPresentModeKHR* available_present_modes, u32 available_present_modes_count)
{
    for (u32 i = 0; i < available_present_modes_count; ++i) {
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_modes[i];
        }
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D vulkan_choose_swap_extent(X11State* x11_state,
                                            VkSurfaceCapabilitiesKHR* capabilities)
{
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    }

    u32 width, height;
    x11_get_framebuffer_size(x11_state, &width, &height);

    VkExtent2D actual_extent = { width, height };
    actual_extent.width = clamp_u32(actual_extent.width,
                                    capabilities->minImageExtent.width,
                                    capabilities->maxImageExtent.width);
    actual_extent.height = clamp_u32(actual_extent.height,
                                    capabilities->minImageExtent.height,
                                    capabilities->maxImageExtent.height);

    return actual_extent;
}

static void vulkan_create_swap_chain(X11State* x11_state)
{
    VulkanSwapChainSupportDetails swap_chain_support =
        vulkan_query_swap_chain_support(vulkan_physical_device);

    VkSurfaceFormatKHR surface_format =
        vulkan_choose_swap_surface_format(swap_chain_support.formats,
                                          swap_chain_support.format_count);
    VkPresentModeKHR present_mode =
        vulkan_choose_swap_present_mode(swap_chain_support.present_modes,
                                        swap_chain_support.present_mode_count);
    VkExtent2D extent = vulkan_choose_swap_extent(x11_state,
                                                  &swap_chain_support.capabilities);
    u32 image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.minImageCount > 0 &&
        image_count > swap_chain_support.capabilities.maxImageCount) {
        image_count = swap_chain_support.capabilities.maxImageCount;
    }

    VulkanQueueFamilyIndices indices =
        vulkan_find_queue_families(vulkan_physical_device);
    u32 queue_family_indices[] = {
        indices.graphics_family_index,
        indices.present_family_index
    };

    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface                  = vulkan_surface;
    create_info.minImageCount            = image_count;
    create_info.imageFormat              = surface_format.format;
    create_info.imageColorSpace          = surface_format.colorSpace;
    create_info.imageExtent              = extent;
    create_info.imageArrayLayers         = 1;
    create_info.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices   = NULL;
    if (indices.graphics_family_index != indices.present_family_index) {
        create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices   = queue_family_indices;
    }

    create_info.preTransform   = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode    = present_mode;
    create_info.clipped        = VK_TRUE;
    create_info.oldSwapchain   = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(vulkan_device, &create_info,
                                     NULL, &vulkan_swap_chain);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to create swap buffer: %s", buf);
    }

    vkGetSwapchainImagesKHR(vulkan_device, vulkan_swap_chain, &image_count, NULL);
    vulkan_swap_chain_images_count = image_count;
    vkGetSwapchainImagesKHR(vulkan_device, vulkan_swap_chain,
                            &image_count, vulkan_swap_chain_images);
    
    vulkan_swap_chain_image_format = surface_format.format;
    vulkan_swap_chain_extent       = extent;
}

static b32 vulkan_is_device_suitable(VkPhysicalDevice device)
{
    VulkanQueueFamilyIndices indices = vulkan_find_queue_families(device);

    b32 device_extensions_supported = vulkan_check_device_extension_support(device);

    b32 swap_chain_support_adequate = false;
    if (device_extensions_supported) {
        VulkanSwapChainSupportDetails swap_chain_support =
            vulkan_query_swap_chain_support(device);
        swap_chain_support_adequate = (swap_chain_support.format_count > 0 &&
                                       swap_chain_support.present_mode_count > 0);
    }
    
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device, &device_features);

    b32 gpu_is_discrete =
        device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    b32 gpu_supports_geometry_shaders = device_features.geometryShader;
    
    return (gpu_is_discrete &&
            gpu_supports_geometry_shaders &&
            device_extensions_supported &&
            swap_chain_support_adequate &&
            vulkan_queue_family_indices_is_complete(&indices));
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

    f32 queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_infos[100];

    // Check for duplicate queue indices, only create unique queues
    
    u32 queue_families[] = {indices.graphics_family_index, indices.present_family_index};
    u32 queue_families_count = sizeof(queue_families) / sizeof(queue_families[0]);
    
    u32 unique_queue_families[100];
    u32 unique_queue_families_count = 0;
    
    for (u32 i = 0; i < queue_families_count; ++i) {
        b32 exists = false;
        for (u32 j = 0; j < unique_queue_families_count; ++j) {
            if (queue_families[i] == unique_queue_families[j]) {
                    exists = true;
                    break;
                }
        }
        if (!exists) {
            unique_queue_families[unique_queue_families_count++] = queue_families[i];
        }
    }                                                              

    // Create queues
    for (u32 i = 0; i < unique_queue_families_count; ++i) {
        VkDeviceQueueCreateInfo queue_create_info = {0};
        queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = unique_queue_families[i];
        queue_create_info.queueCount       = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos[i]              = queue_create_info;
    }
    

    VkPhysicalDeviceFeatures device_features = {0};

    VkDeviceCreateInfo create_info      = {0};
    create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos       = queue_create_infos;
    create_info.queueCreateInfoCount    = unique_queue_families_count;
    create_info.pEnabledFeatures        = &device_features;
    create_info.enabledExtensionCount   = device_extensions_count;
    create_info.ppEnabledExtensionNames = device_extensions;
    create_info.enabledLayerCount       = validation_layers_count;
    create_info.ppEnabledLayerNames     = validation_layers;

    VkResult result = vkCreateDevice(vulkan_physical_device, &create_info,
                                     NULL, &vulkan_device);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to create logical device: %s", buf);
    }

    vkGetDeviceQueue(vulkan_device, indices.graphics_family_index,
                     0, &vulkan_graphics_queue);
    vkGetDeviceQueue(vulkan_device, indices.present_family_index,
                     0, &vulkan_present_queue);
}

void x11_vulkan_init(X11State* x11_state)
{
    FC_INFO("Initializing Vulkan");
    vulkan_create_instance(x11_state);
    vulkan_init_debug_messenger();
    vulkan_create_surface(x11_state);
    vulkan_pick_physical_device();
    vulkan_create_logical_device();
    vulkan_create_swap_chain(x11_state);
    FC_INFO("Vulkan initialized");
}

void x11_vulkan_deinit(X11State* x11_state)
{
    FC_INFO("Deinitializing Vulkan");
    (void)x11_state;
    destroy_debug_utils_messenger_EXT(vulkan_instance, vulkan_debug_messenger, NULL);
    vkDestroySwapchainKHR(vulkan_device, vulkan_swap_chain, NULL);
    vkDestroyDevice(vulkan_device, NULL);
    vkDestroySurfaceKHR(vulkan_instance, vulkan_surface, NULL);
    vkDestroyInstance(vulkan_instance, NULL);
    FC_INFO("Vulkan deinitialized");
}
