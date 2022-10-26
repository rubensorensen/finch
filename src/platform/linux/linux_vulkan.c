#include "linux_vulkan.h"
#include "finch/log/log.h"
#include "finch/utils/utils.h"

#include <stdlib.h>

#define MAX_FRAMES_IN_FLIGHT 2

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
static VkImageView              vulkan_swap_chain_image_views[100];
static u32                      vulkan_swap_chain_image_views_count;
static VkFramebuffer            vulkan_swap_chain_framebuffers[100];
static u32                      vulkan_swap_chain_framebuffers_count;
static VkRenderPass             vulkan_render_pass;
static VkPipelineLayout         vulkan_pipeline_layout;
static VkPipeline               vulkan_graphics_pipeline;
static VkCommandPool            vulkan_command_pool;
static VkCommandBuffer          vulkan_command_buffers[MAX_FRAMES_IN_FLIGHT];
static VkDebugUtilsMessengerEXT vulkan_debug_messenger;

static VkSemaphore              vulkan_image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
static VkSemaphore              vulkan_render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
static VkFence                  vulkan_in_flight_fences[MAX_FRAMES_IN_FLIGHT];

static u32                      vulkan_current_frame;
static b32                      vulkan_framebuffer_resized;

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

static void vulkan_create_image_views(void)
{
    vulkan_swap_chain_image_views_count = vulkan_swap_chain_images_count;

    for (u32 i = 0; i < vulkan_swap_chain_images_count; ++i) {
        VkImageViewCreateInfo create_info = {0};
        create_info.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image    = vulkan_swap_chain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format   = vulkan_swap_chain_image_format;

        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel   = 0;
        create_info.subresourceRange.levelCount     = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount     = 1;

        VkResult result = vkCreateImageView(vulkan_device, &create_info,
                                            NULL, &vulkan_swap_chain_image_views[i]);
        if (result != VK_SUCCESS) {
            char buf[1024];
            vulkan_result_to_string(buf, result);
            FC_ERROR("Failed to create image view: %s", buf);
        }
    }
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

static VkShaderModule vulkan_create_shader_module(char* shader_code,
                                                  u32 shader_code_size)
{
    VkShaderModuleCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shader_code_size;
    create_info.pCode = (u32*)shader_code;

    VkShaderModule shader_module;
    VkResult result = vkCreateShaderModule(vulkan_device, &create_info,
                                           NULL, &shader_module);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to create shader module: %s", buf);
    }

    return shader_module;
}

static void vulkan_create_render_pass()
{
    VkAttachmentDescription color_attachment = {0};
    color_attachment.format = vulkan_swap_chain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {0};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(vulkan_device, &render_pass_info,
                                         NULL, &vulkan_render_pass);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to create render pass: %s", buf);
    }

}

static void vulkan_create_graphics_pipeline()
{

    // Prepare shaders
    u32 max_shader_code_size = 1024 * 1024;
    char vertex_shader_code[max_shader_code_size];
    u32 vertex_shader_code_size = read_entire_file(vertex_shader_code,
                                                   max_shader_code_size,
                                                   "./assets/shaders/vert.spv");

    char fragment_shader_code[max_shader_code_size];
    u32 fragment_shader_code_size = read_entire_file(fragment_shader_code,
                                                     max_shader_code_size,
                                                     "./assets/shaders/frag.spv");

    VkShaderModule vertex_shader_module =
        vulkan_create_shader_module(vertex_shader_code, vertex_shader_code_size);
    VkShaderModule fragment_shader_module =
        vulkan_create_shader_module(fragment_shader_code, fragment_shader_code_size);

    VkPipelineShaderStageCreateInfo vertex_shader_stage_info = {0};
    vertex_shader_stage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_info.module = vertex_shader_module;
    vertex_shader_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_stage_info = {0};
    fragment_shader_stage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_info.module = fragment_shader_module;
    fragment_shader_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vertex_shader_stage_info,
        fragment_shader_stage_info
    };

    // Prepare dynamic states
    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    u32 dynamic_states_count = sizeof(dynamic_states) / sizeof(dynamic_states[0]);

    VkPipelineDynamicStateCreateInfo dynamic_state = {0};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = dynamic_states_count;
    dynamic_state.pDynamicStates    = dynamic_states;

    // Prepare vertex input
    VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount   = 0;
    vertex_input_info.pVertexBindingDescriptions      = NULL;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions    = NULL;

    // Prepare input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    // Using dynamic viewport so no need to set anything
    VkPipelineViewportStateCreateInfo viewport_state = {0};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp          = 0.0f;
    rasterizer.depthBiasSlopeFactor    = 0.0f;

    // Multisampling (Disabled for now)
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = NULL;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    // Depth and stencil testing
    // TODO: do this

    // Color blending
    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask = (VK_COLOR_COMPONENT_R_BIT |
                                             VK_COLOR_COMPONENT_G_BIT |
                                             VK_COLOR_COMPONENT_B_BIT |
                                             VK_COLOR_COMPONENT_A_BIT);
    color_blend_attachment.blendEnable         = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending = {0};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = NULL;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = NULL;

    VkResult result = vkCreatePipelineLayout(vulkan_device, &pipeline_layout_info,
                                             NULL, &vulkan_pipeline_layout);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to create pipeline layout: %s", buf);
    }

    // Pipeline creation
    VkGraphicsPipelineCreateInfo pipeline_info = {0};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages    = shader_stages;

    pipeline_info.pVertexInputState   = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState      = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState   = &multisampling;
    pipeline_info.pDepthStencilState  = NULL;
    pipeline_info.pColorBlendState    = &color_blending;
    pipeline_info.pDynamicState       = &dynamic_state;

    pipeline_info.layout = vulkan_pipeline_layout;

    pipeline_info.renderPass = vulkan_render_pass;
    pipeline_info.subpass    = 0;

    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex  = -1;

    result = vkCreateGraphicsPipelines(vulkan_device, VK_NULL_HANDLE,
                                                1, &pipeline_info, NULL,
                                                &vulkan_graphics_pipeline);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to create graphics pipeline: %s", buf);
    }

    vkDestroyShaderModule(vulkan_device, vertex_shader_module, NULL);
    vkDestroyShaderModule(vulkan_device, fragment_shader_module, NULL);

}

static void vulkan_create_framebuffers(void)
{
    vulkan_swap_chain_framebuffers_count = vulkan_swap_chain_image_views_count;

    for (u32 i = 0; i < vulkan_swap_chain_image_views_count; ++i) {
        VkImageView attachments[] = { vulkan_swap_chain_image_views[i] };

        VkFramebufferCreateInfo framebuffer_info = {0};
        framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass      = vulkan_render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments    = attachments;
        framebuffer_info.width           = vulkan_swap_chain_extent.width;
        framebuffer_info.height          = vulkan_swap_chain_extent.height;
        framebuffer_info.layers          = 1;

        VkResult result = vkCreateFramebuffer(vulkan_device, &framebuffer_info,
                                              NULL, &vulkan_swap_chain_framebuffers[i]);
        if (result != VK_SUCCESS) {
            char buf[1024];
            vulkan_result_to_string(buf, result);
            FC_ERROR("Failed to create framebuffer: %s", buf);
        }
    }
}

static void vulkan_create_command_pool(void)
{
    VulkanQueueFamilyIndices queue_family_indices =
        vulkan_find_queue_families(vulkan_physical_device);

    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family_index;

    VkResult result = vkCreateCommandPool(vulkan_device, &pool_info, NULL,
                                          &vulkan_command_pool);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to create framebuffer: %s", buf);
    }
}

static void vulkan_create_command_buffers(void)
{
    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = vulkan_command_pool;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = (u32)MAX_FRAMES_IN_FLIGHT;

    VkResult result = vkAllocateCommandBuffers(vulkan_device, &alloc_info,
                                        vulkan_command_buffers);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to allocate command buffers: %s", buf);
    }
}

static void vulkan_record_command_buffer(VkCommandBuffer command_buffer, u32 image_index)
{
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = NULL;

    VkResult result = vkBeginCommandBuffer(command_buffer, &begin_info);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to begin recording command buffer: %s", buf);
    }

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType               = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass          = vulkan_render_pass;
    render_pass_info.framebuffer         = vulkan_swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset.x = 0;
    render_pass_info.renderArea.offset.y = 0;
    render_pass_info.renderArea.extent   = vulkan_swap_chain_extent;

    VkClearValue clear_color         = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues    = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      vulkan_graphics_pipeline);

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)vulkan_swap_chain_extent.width;
    viewport.height = (f32)vulkan_swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = vulkan_swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    result = vkEndCommandBuffer(command_buffer);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to record command buffer: %s", buf);
    }
}

static void vulkan_create_sync_objects(void)
{
    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkResult result;
        result = vkCreateSemaphore(vulkan_device, &semaphore_info,
                                   NULL, &vulkan_image_available_semaphores[i]);
        if (result != VK_SUCCESS) {
            char buf[1024];
            vulkan_result_to_string(buf, result);
            FC_ERROR("Failed to create semaphore: %s", buf);
        }

        result = vkCreateSemaphore(vulkan_device, &semaphore_info,
                                   NULL, &vulkan_render_finished_semaphores[i]);
        if (result != VK_SUCCESS) {
            char buf[1024];
            vulkan_result_to_string(buf, result);
            FC_ERROR("Failed to create semaphore: %s", buf);
        }

        result = vkCreateFence(vulkan_device, &fence_info,
                               NULL, &vulkan_in_flight_fences[i]);
        if (result != VK_SUCCESS) {
            char buf[1024];
            vulkan_result_to_string(buf, result);
            FC_ERROR("Failed to create fence: %s", buf);
        }
    }
}

static void vulkan_clean_up_swap_chain()
{
    for (u32 i = 0; i < vulkan_swap_chain_framebuffers_count; ++i) {
        vkDestroyFramebuffer(vulkan_device, vulkan_swap_chain_framebuffers[i], NULL);
    }

    for (u32 i = 0; i < vulkan_swap_chain_image_views_count; ++i) {
        vkDestroyImageView(vulkan_device, vulkan_swap_chain_image_views[i], NULL);
    }

    vkDestroySwapchainKHR(vulkan_device, vulkan_swap_chain, NULL);
}

void vulkan_recreate_swap_chain(X11State* x11_state)
{
    // TODO: Handle minimization! If window is minimized, application should block!

    vkDeviceWaitIdle(vulkan_device);

    vulkan_clean_up_swap_chain();

    vulkan_create_swap_chain(x11_state);
    vulkan_create_image_views();
    vulkan_create_framebuffers();
}

void vulkan_draw_frame(X11State* x11_state)
{
    vkWaitForFences(vulkan_device, 1, &vulkan_in_flight_fences[vulkan_current_frame],
                    VK_TRUE, UINT64_MAX);

    u32 image_index;
    VkResult result;
    result = vkAcquireNextImageKHR(vulkan_device, vulkan_swap_chain, UINT64_MAX,
                                   vulkan_image_available_semaphores[vulkan_current_frame],
                                   VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vulkan_recreate_swap_chain(x11_state);
        return;
    }

    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to acquire swap chain image: %s", buf);
        return;
    }

    vkResetFences(vulkan_device, 1, &vulkan_in_flight_fences[vulkan_current_frame]);

    vkResetCommandBuffer(vulkan_command_buffers[vulkan_current_frame], 0);
    vulkan_record_command_buffer(vulkan_command_buffers[vulkan_current_frame],
                                 image_index);

    VkSubmitInfo submit_info = {0};
    submit_info.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {
        vulkan_image_available_semaphores[vulkan_current_frame]
    };

    VkPipelineStageFlags wait_stages[] =
        { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores    = wait_semaphores;
    submit_info.pWaitDstStageMask  = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &vulkan_command_buffers[vulkan_current_frame];

    VkSemaphore signal_semaphores[] = {
        vulkan_render_finished_semaphores[vulkan_current_frame]
    };

    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    result = vkQueueSubmit(vulkan_graphics_queue, 1, &submit_info,
                                    vulkan_in_flight_fences[vulkan_current_frame]);
    if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to submit draw command buffer: %s", buf);
    }

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = { vulkan_swap_chain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;

    result = vkQueuePresentKHR(vulkan_present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR || vulkan_framebuffer_resized) {
        vulkan_framebuffer_resized = false;
        vulkan_recreate_swap_chain(x11_state);
    } else if (result != VK_SUCCESS) {
        char buf[1024];
        vulkan_result_to_string(buf, result);
        FC_ERROR("Failed to present swap chain image: %s", buf);
    }

    vulkan_current_frame = (vulkan_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void vulkan_set_framebuffer_resized(b32 state)
{
    vulkan_framebuffer_resized = state;
}

void vulkan_wait_for_device_idle(void)
{
    vkDeviceWaitIdle(vulkan_device);
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
    vulkan_create_image_views();
    vulkan_create_render_pass();
    vulkan_create_graphics_pipeline();
    vulkan_create_framebuffers();
    vulkan_create_command_pool();
    vulkan_create_command_buffers();
    vulkan_create_sync_objects();

    FC_INFO("Vulkan initialized");
}

void x11_vulkan_deinit(X11State* x11_state)
{
    FC_INFO("Deinitializing Vulkan");
    (void)x11_state;

    vulkan_clean_up_swap_chain();

    vkDestroyPipeline(vulkan_device, vulkan_graphics_pipeline, NULL);
    vkDestroyPipelineLayout(vulkan_device, vulkan_pipeline_layout, NULL);
    vkDestroyRenderPass(vulkan_device, vulkan_render_pass, NULL);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(vulkan_device, vulkan_image_available_semaphores[i], NULL);
        vkDestroySemaphore(vulkan_device, vulkan_render_finished_semaphores[i], NULL);
        vkDestroyFence(vulkan_device, vulkan_in_flight_fences[i], NULL);
    }

    vkDestroyCommandPool(vulkan_device, vulkan_command_pool, NULL);
    vkDestroyDevice(vulkan_device, NULL);
    vkDestroySurfaceKHR(vulkan_instance, vulkan_surface, NULL);
    destroy_debug_utils_messenger_EXT(vulkan_instance, vulkan_debug_messenger, NULL);
    vkDestroyInstance(vulkan_instance, NULL);

    FC_INFO("Vulkan deinitialized");
}
