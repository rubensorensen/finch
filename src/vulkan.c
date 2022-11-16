#include "finch/vulkan.h"
#include "finch/log.h"
#include "finch/utils.h"
#include "finch/application.h"
#include "finch/platform.h"

#include <stdlib.h>
#include <string.h>

#define VSYNC 1

// Implemented in platform layer
extern VkSurfaceKHR create_surface(VkInstance instance);

static Vertex g_vertices[] = {
    {
        .position = {.x = -0.5f, .y = -0.5f},
        .color    = {.r = 1.0f, .g = 0.0f, .b = 0.0f}
    },
    {
        .position = {.x = 0.5f, .y = -0.5f},
        .color    = {.r = 0.0f, .g = 1.0f, .b = 0.0f}
    },
    {
        .position = {.x = 0.5f, .y = 0.5f},
        .color    = {.r = 0.0f, .g = 0.0f, .b = 1.0f}
    },
    {
        .position = {.x = -0.5f, .y = 0.5f},
        .color    = {.r = 1.0f, .g = 1.0f, .b = 1.0f}
    }
};

static u16 g_indices[] = {0, 1, 2, 2, 3, 0};

static VulkanCore g_core;
static VulkanQueueFamilyIndices g_queue_family_indices;
static VulkanQueues g_queues;

static VkBuffer g_vertex_buffer;
static VkDeviceMemory g_vertex_buffer_memory;
static VkBuffer g_index_buffer;
static VkDeviceMemory g_index_buffer_memory;

static UniformBuffers g_uniform_buffers;
static VkRenderPass g_render_pass;

static VkDescriptorSetLayout g_descriptor_set_layout;
static VkDescriptorPool g_descriptor_pool;
static DescriptorSets g_descriptor_sets;

static VulkanPipeline g_pipeline;
static VkCommandPool        g_command_pool;
static VulkanCommandBuffers g_command_buffers;

static VulkanSwapChainInfo g_swap_chain_info;
static VulkanFramebuffers g_framebuffers;

static u32 g_current_frame;

static const char* g_instance_extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,

#ifdef FINCH_LOGGING
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // FINCH_LOGGING
    
};

static const char* g_device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const char* g_validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef FINCH_LOGGING
static VkDebugUtilsMessengerEXT vulkan_debug_messenger;
VkResult create_debug_utils_messenger_EXT(const VkDebugUtilsMessengerCreateInfoEXT* create_info,
                                      const VkAllocationCallbacks* allocator,
                                      VkDebugUtilsMessengerEXT* debug_messenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_core.instance, "vkCreateDebugUtilsMessengerEXT");

    if (func == NULL) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return func(g_core.instance, create_info, allocator, debug_messenger);
}

void destroy_debug_utils_messenger_EXT(VkDebugUtilsMessengerEXT debug_messenger,
                                       const VkAllocationCallbacks* allocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_core.instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(g_core.instance, debug_messenger, allocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    UNUSED(messageType);
    UNUSED(pUserData);

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

void vulkan_init_debug_messenger()
{
    VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
    vulkan_populate_debug_messenger(&create_info);

    VkResult result = create_debug_utils_messenger_EXT(&create_info, NULL,
                                                       &vulkan_debug_messenger);
    VULKAN_VERIFY(result);
}
#endif // FINCH_LOGGING

void vkresult_to_string(char* buf, VkResult result)
{
    switch (result)
    {
        case VK_SUCCESS: {
            strcpy(buf, "Success");
        } break;
        case VK_NOT_READY: {
            strcpy(buf, "A fence or query has not yet completed");
        } break;
        case VK_TIMEOUT: {
            strcpy(buf, "A wait operation has not completed in the specified time");
        } break;
        case VK_EVENT_SET: {
            strcpy(buf, "An event is signaled");
        } break;
        case VK_EVENT_RESET: {
            strcpy(buf, "An event is unsignaled");
        } break;
        case VK_INCOMPLETE: {
            strcpy(buf, "A string_copy(buf, array was too small for the result");
        } break;
        case VK_ERROR_OUT_OF_HOST_MEMORY: {
            strcpy(buf, "A host memory allocation has failed");
        } break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: {
            strcpy(buf, "A device memory allocation has failed");
        } break;
        case VK_ERROR_INITIALIZATION_FAILED: {
            strcpy(buf, "Initialization of an object could not be completed for implementation-specific reasons");
        } break;
        case VK_ERROR_DEVICE_LOST: {
            strcpy(buf, "The logical or physical device has been lost");
        } break;
        case VK_ERROR_MEMORY_MAP_FAILED: {
            strcpy(buf, "Mapping of a memory object has failed");
        } break;
        case VK_ERROR_LAYER_NOT_PRESENT: {
            strcpy(buf, "A requested layer is not present or could not be loaded");
        } break;
        case VK_ERROR_EXTENSION_NOT_PRESENT: {
            strcpy(buf, "A requested extension is not supported");
        } break;
        case VK_ERROR_FEATURE_NOT_PRESENT: {
            strcpy(buf, "A requested feature is not supported");
        } break;
        case VK_ERROR_INCOMPATIBLE_DRIVER: {
            strcpy(buf, "The requested version of Vulkan is not supported by the driver or is otherwise incompatible");
        } break;
        case VK_ERROR_TOO_MANY_OBJECTS: {
            strcpy(buf, "Too many objects of the type have already been created");
        } break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED: {
            strcpy(buf, "A requested format is not supported on this device");
        } break;
        case VK_ERROR_SURFACE_LOST_KHR: {
            strcpy(buf, "A surface is no longer available");
        } break;
        case VK_SUBOPTIMAL_KHR: {
            strcpy(buf, "A swapchain no longer matches the surface properties exactly, but can still be used");
        } break;
        case VK_ERROR_OUT_OF_DATE_KHR: {
            strcpy(buf, "A surface has changed in such a way that it is no longer compatible with the swapchain");
        } break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: {
            strcpy(buf, "The display used by a swapchain does not use the same presentable image layout");
        } break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: {
            strcpy(buf, "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API");
        } break;
        case VK_ERROR_VALIDATION_FAILED_EXT: {
            strcpy(buf, "A validation layer found an error");
        } break;
        default: {
            strcpy(buf, "ERROR: UNKNOWN VULKAN ERROR");
        }
    }
}

void vulkan_verify(VkResult result)
{
    if (result != VK_SUCCESS) {
        char buf[512];
        vkresult_to_string(buf, result);
        FC_ERROR("Vulkan: %s", buf);
    }
}

b32 queue_family_indices_is_complete(VulkanQueueFamilyIndices* indices)
{
    return (indices->has_graphics_family &&
            indices->has_present_family);
}

b32 check_instance_extension_support() {
    u32 extension_count;
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    VkExtensionProperties* supported_extensions =
        (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extension_count);
    
    if (!supported_extensions) {
        return false;
    }
    
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, supported_extensions);

    for (u32 i = 0; i < ARRAY_COUNT(g_instance_extensions); ++i) {
        b32 extension_found = false;
        for (u32 j = 0; j < extension_count; ++j) {
            if (strcmp((char*)g_instance_extensions[i],
                               supported_extensions[j].extensionName) != 0) {
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

b32 check_validation_layer_support() {
    u32 layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    VkLayerProperties* supported_layers =
        (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * layer_count);

    if (!supported_layers) {
        return false;
    }
    
    vkEnumerateInstanceLayerProperties(&layer_count, supported_layers);

    for (u32 i = 0; i < ARRAY_COUNT(g_validation_layers); ++i) {
        b32 layer_found = false;
        for (u32 j = 0; j < layer_count; ++j) {
            if (strcmp((char*)g_validation_layers[i], supported_layers[j].layerName) != 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) {
            return false;
        }
    }

    free(supported_layers);
    
    return true;
}

static void create_instance(ApplicationState* app_state)
{
    if (!check_instance_extension_support()) {
        FC_ERROR("Instance extensions requested, but not supported");
    }

    if (!check_validation_layer_support()) {
        FC_ERROR("Validation layers requested, but not supported");
    }

    VkApplicationInfo app_info  = {0};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = app_state->name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "Finch";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_0;


    VkInstanceCreateInfo create_info    = {0};
    create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo        = &app_info;
    create_info.enabledExtensionCount   = ARRAY_COUNT(g_instance_extensions);
    create_info.ppEnabledExtensionNames = (const char* const*)g_instance_extensions;
    create_info.enabledLayerCount       = ARRAY_COUNT(g_validation_layers);
    create_info.ppEnabledLayerNames     = (const char* const*)g_validation_layers;

#ifdef FINCH_LOGGING
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
    vulkan_populate_debug_messenger(&debug_create_info);
    create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
#endif // FINCH_LOGGING
    
    VkResult result = vkCreateInstance(&create_info, NULL, &g_core.instance);
    VULKAN_VERIFY(result);
}

static VulkanQueueFamilyIndices find_queue_families(VkPhysicalDevice phy_device)
{
    VulkanQueueFamilyIndices indices = {0};

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_family_count, NULL);
    VkQueueFamilyProperties* queue_families =
        (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);

    if (!queue_families) {
        return indices;
    }
    
    vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_family_count,
                                             queue_families);

    for (u32 i = 0; i < queue_family_count; ++i) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.has_graphics_family = true;
            indices.graphics_family_index = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(phy_device, i,
                                             g_core.surface, &present_support);
        if (present_support) {
            indices.has_present_family = true;
            indices.present_family_index = i;
        }
    }

    free(queue_families);

    return indices;
}

b32 check_device_extension_support(VkPhysicalDevice phy_device)
{
    u32 extension_count = 0;
    vkEnumerateDeviceExtensionProperties(phy_device, NULL, &extension_count, NULL);
    VkExtensionProperties* supported_extensions =
        (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extension_count);

    if (!supported_extensions) {
        return false;
    }
    
    vkEnumerateDeviceExtensionProperties(phy_device, NULL, &extension_count,
                                         supported_extensions);

    for (u32 i = 0; i < ARRAY_COUNT(g_device_extensions); ++i) {
        b32 extension_found = false;
        for (u32 j = 0; j < extension_count; ++j) {
            if (strcmp((char*)g_device_extensions[i],
                       supported_extensions[j].extensionName) != 0) {
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

static VulkanSwapChainSupportDetails
query_swap_chain_support(VkPhysicalDevice phy_device)
{
    VulkanSwapChainSupportDetails details = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy_device, g_core.surface,
                                              &details.capabilities);

    u32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, g_core.surface,
                                         &format_count, NULL);
    if (format_count != 0) {
        // TODO: Assert that format_count is not greater than max format count size
        details.format_count = format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, g_core.surface,
                                             &format_count, details.formats);
    }

    u32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(phy_device, g_core.surface,
                                              &present_mode_count, NULL);
    if (present_mode_count != 0) {
        // TODO: Assert that present_mode_count is not greater than max present_mode_count size
        details.present_mode_count = present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(phy_device, g_core.surface,
                                                  &present_mode_count,
                                                  details.present_modes);
    }

    return details;
}

static VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR* available_formats, u32 available_formats_count)
{
    for (u32 i = 0; i < available_formats_count; ++i) {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_formats[i];
        }
    }

    return available_formats[0];
}

static VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR* available_present_modes, u32 available_present_modes_count)
{
    VkPresentModeKHR optimal_present_mode;
    
#if VSYNC
    optimal_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
#else
    optimal_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
#endif
    
    for (u32 i = 0; i < available_present_modes_count; ++i) {
        if (available_present_modes[i] == optimal_present_mode) {
            return available_present_modes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR* capabilities)
{
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    }

    u32 width, height;
    platform_get_framebuffer_size(&width, &height);

    VkExtent2D actual_extent = { width, height };
    actual_extent.width = clamp_u32(actual_extent.width,
                                    capabilities->minImageExtent.width,
                                    capabilities->maxImageExtent.width);
    actual_extent.height = clamp_u32(actual_extent.height,
                                    capabilities->minImageExtent.height,
                                    capabilities->maxImageExtent.height);

    return actual_extent;
}

static VulkanImageViews create_image_views(VulkanImages images, VkFormat format)
{
    VulkanImageViews image_views = {0};
    image_views.count = images.count;

    for (u32 i = 0; i < image_views.count; ++i) {
        VkImageViewCreateInfo create_info = {0};
        create_info.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image    = images.images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format   = format;

        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel   = 0;
        create_info.subresourceRange.levelCount     = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount     = 1;

        VkResult result = vkCreateImageView(g_core.device, &create_info,
                                            NULL, &image_views.views[i]);
        VULKAN_VERIFY(result);
    }
    
    return image_views;
}

static VulkanFramebuffers
create_framebuffers(VkRenderPass render_pass, VulkanImageViews image_views,
                    VkExtent2D extent)
{
    VulkanFramebuffers framebuffers = {0};
    framebuffers.count = image_views.count;

    for (u32 i = 0; i < image_views.count; ++i) {
        VkImageView attachments[] = { image_views.views[i] };

        VkFramebufferCreateInfo framebuffer_info = {0};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass      = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments    = attachments;
        framebuffer_info.width           = extent.width;
        framebuffer_info.height          = extent.height;
        framebuffer_info.layers          = 1;

        VkResult result = vkCreateFramebuffer(g_core.device, &framebuffer_info, NULL,
                                              &framebuffers.framebuffers[i]);
        VULKAN_VERIFY(result);
    }

    return framebuffers;
}

static VulkanSwapChainInfo create_swap_chain(VulkanQueueFamilyIndices indices)
{   
    VulkanSwapChainSupportDetails swap_chain_support =
        query_swap_chain_support(g_core.phy_device);
    
    VkSurfaceFormatKHR surface_format =
        choose_swap_surface_format(swap_chain_support.formats,
                                   swap_chain_support.format_count);
    VkPresentModeKHR present_mode =
        choose_swap_present_mode(swap_chain_support.present_modes,
                                 swap_chain_support.present_mode_count);
    
    VkExtent2D extent = choose_swap_extent(&swap_chain_support.capabilities);
    
    u32 image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.minImageCount > 0 &&
        image_count > swap_chain_support.capabilities.maxImageCount) {
        image_count = swap_chain_support.capabilities.maxImageCount;
    }
    
    u32 queue_family_indices[] = {
        indices.graphics_family_index,
        indices.present_family_index
    };

    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface                  = g_core.surface;
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

    VulkanSwapChainInfo swap_chain_info;
    VkResult result = vkCreateSwapchainKHR(g_core.device, &create_info, NULL,
                                           &swap_chain_info.swap_chain);
    VULKAN_VERIFY(result);
    
    vkGetSwapchainImagesKHR(g_core.device, swap_chain_info.swap_chain,
                            &image_count, NULL);
    
    swap_chain_info.images.count = image_count;
    vkGetSwapchainImagesKHR(g_core.device, swap_chain_info.swap_chain,
                            &image_count, swap_chain_info.images.images);

    swap_chain_info.image_format = surface_format.format;
    swap_chain_info.extent       = extent;
    swap_chain_info.image_views  = create_image_views(swap_chain_info.images, surface_format.format);
    
    return swap_chain_info;
}

static b32 vulkan_is_device_suitable(VkPhysicalDevice phy_device)
{
    VulkanQueueFamilyIndices indices = find_queue_families(phy_device);

    b32 device_extensions_supported = check_device_extension_support(phy_device);

    b32 swap_chain_support_adequate = false;
    if (device_extensions_supported) {
        VulkanSwapChainSupportDetails swap_chain_support = 
            query_swap_chain_support(phy_device);
        swap_chain_support_adequate = (swap_chain_support.format_count > 0 &&
                                       swap_chain_support.present_mode_count > 0);
    }

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(phy_device, &device_properties);

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(phy_device, &device_features);

    b32 gpu_is_discrete = 
        device_properties.deviceType  == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    b32 gpu_supports_geometry_shaders  = device_features.geometryShader;

    return (gpu_is_discrete               &&
            gpu_supports_geometry_shaders &&
            device_extensions_supported   &&
            swap_chain_support_adequate   &&
            queue_family_indices_is_complete(&indices));
}

static void pick_physical_device(VulkanQueueFamilyIndices* indices)
{   
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(g_core.instance, &device_count, NULL);
    if (device_count == 0) {
        FC_ERROR("Failed to find GPUs with vulkan support!");
    }

    VkPhysicalDevice* physical_devices =
        (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * device_count);

    if (!physical_devices) {
        g_core.phy_device = physical_devices[0];
        return;
    }
    
    vkEnumeratePhysicalDevices(g_core.instance, &device_count, physical_devices);

    for (u32 i = 0; i < device_count; ++i) {
        if (vulkan_is_device_suitable(physical_devices[i])) {
            g_core.phy_device = physical_devices[i];
            *indices = find_queue_families(g_core.phy_device);
            break;
        }
    }
    
    free(physical_devices);

    if (g_core.phy_device == VK_NULL_HANDLE) {
        FC_ERROR("Failed to find a suitable GPU");
    }

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(g_core.phy_device, &device_properties);
    FC_INFO("Picked physical device: %s", device_properties.deviceName);
}

static void
setup_logical_device(VulkanQueueFamilyIndices indices, VulkanQueues* queues)
{
    f32                     queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_infos[100];

    // Check for duplicate queue indices, only create unique queues

    u32 queue_families[] = {
        indices.graphics_family_index,
        indices.present_family_index
    };
    u32 queue_families_count = ARRAY_COUNT(queue_families);

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
        queue_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex        = unique_queue_families[i];
        queue_create_info.queueCount              = 1;
        queue_create_info.pQueuePriorities        = &queue_priority;
        queue_create_infos[i]                     = queue_create_info;
    }


    VkPhysicalDeviceFeatures device_features = {0};

    VkDeviceCreateInfo create_info      = {0};
    create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos       = queue_create_infos;
    create_info.queueCreateInfoCount    = unique_queue_families_count;
    create_info.pEnabledFeatures        = &device_features;
    create_info.enabledExtensionCount   = ARRAY_COUNT(g_device_extensions);
    create_info.ppEnabledExtensionNames = g_device_extensions;
    create_info.enabledLayerCount       = ARRAY_COUNT(g_validation_layers);
    create_info.ppEnabledLayerNames     = g_validation_layers;

    VkResult result = vkCreateDevice(g_core.phy_device, &create_info,
                                     NULL, &g_core.device);
    VULKAN_VERIFY(result);

    vkGetDeviceQueue(g_core.device, indices.graphics_family_index, 0, &queues->graphics);
    vkGetDeviceQueue(g_core.device, indices.present_family_index, 0, &queues->present);
}

static VkShaderModule
create_shader_module(char* shader_code, u32 shader_code_size)
{
    VkShaderModuleCreateInfo create_info = {0};
    create_info.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize                 = shader_code_size;
    create_info.pCode                    = (u32*)shader_code;

    VkShaderModule shader_module;
    VkResult result = vkCreateShaderModule(g_core.device, &create_info,
                                           NULL, &shader_module);
    VULKAN_VERIFY(result);
        
    return shader_module;
}

static VkRenderPass
create_render_pass(VkFormat image_format)
{
    
    VkAttachmentDescription color_attachment = {0};
    color_attachment.format                  = image_format;
    color_attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {0};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_attachment_ref;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask       = 0;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {0};
    render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments    = &color_attachment;
    render_pass_info.subpassCount    = 1;
    render_pass_info.pSubpasses      = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies   = &dependency;

    VkRenderPass render_pass;
    VkResult result = vkCreateRenderPass(g_core.device, &render_pass_info,
                                         NULL, &render_pass);
    VULKAN_VERIFY(result);

    return render_pass;
}

static VkVertexInputBindingDescription
get_binding_desc(void)
{
    VkVertexInputBindingDescription binding_desc = {};
    binding_desc.binding   = 0;
    binding_desc.stride    = sizeof(Vertex);
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    return binding_desc;
}

static VulkanAttributeDescriptions
get_attrib_descs(void)
{
    VulkanAttributeDescriptions attrib_descs = {};
    attrib_descs.descs[0].binding = 0;
    attrib_descs.descs[0].location = 0;
    attrib_descs.descs[0].format = VK_FORMAT_R32G32_SFLOAT;
    attrib_descs.descs[0].offset = offsetof(Vertex, position);

    attrib_descs.descs[1].binding = 0;
    attrib_descs.descs[1].location = 1;
    attrib_descs.descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrib_descs.descs[1].offset = offsetof(Vertex, color);
    
    return attrib_descs;
}

static VulkanPipeline
create_graphics_pipeline(VkRenderPass render_pass,
                         VkDescriptorSetLayout descriptor_set_layout)
{
    VulkanPipeline pipeline;
    
    // Prepare shaders
    u32  max_shader_code_size = 1024 * 1024;
    char vertex_shader_code[max_shader_code_size];
    u32  vertex_shader_code_size = read_entire_file(vertex_shader_code,
                                                    max_shader_code_size,
                                                    "./assets/shaders/vert.spv");

    char fragment_shader_code[max_shader_code_size];
    u32  fragment_shader_code_size = read_entire_file(fragment_shader_code,
                                                      max_shader_code_size,
                                                      "./assets/shaders/frag.spv");

    VkShaderModule vertex_shader_module   = 
        create_shader_module(vertex_shader_code, vertex_shader_code_size);
    VkShaderModule fragment_shader_module = 
        create_shader_module(fragment_shader_code, fragment_shader_code_size);

    VkPipelineShaderStageCreateInfo vertex_shader_stage_info = {0};
    vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_info.module = vertex_shader_module;
    vertex_shader_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_stage_info = {0};
    fragment_shader_stage_info.sType  = 
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

    VkPipelineDynamicStateCreateInfo dynamic_state = {0};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = ARRAY_COUNT(dynamic_states);
    dynamic_state.pDynamicStates    = dynamic_states;

    // Prepare vertex input
    VkVertexInputBindingDescription binding_desc = get_binding_desc();
    VulkanAttributeDescriptions attrib_descs     = get_attrib_descs();
    
    VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount   = 1;
    vertex_input_info.pVertexBindingDescriptions      = &binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = 2;
    vertex_input_info.pVertexAttributeDescriptions    = attrib_descs.descs;

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
    rasterizer.depthClampEnable                       = VK_FALSE;
    rasterizer.rasterizerDiscardEnable                = VK_FALSE;
    rasterizer.polygonMode                            = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth                              = 1.0f;
    rasterizer.cullMode                               = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace                              = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable                        = VK_FALSE;
    rasterizer.depthBiasConstantFactor                = 0.0f;
    rasterizer.depthBiasClamp                         = 0.0f;
    rasterizer.depthBiasSlopeFactor                   = 0.0f;

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
    // TODO: This

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
    color_blending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable     = VK_FALSE;
    color_blending.logicOp           = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount   = 1;
    color_blending.pAttachments      = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount         = 1;
    pipeline_layout_info.pSetLayouts            = &descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges    = NULL;

    VkResult result = vkCreatePipelineLayout(g_core.device, &pipeline_layout_info,
                                             NULL, &pipeline.layout);
    
    VULKAN_VERIFY(result);

    // Pipeline creation
    VkGraphicsPipelineCreateInfo pipeline_info = {0};
    pipeline_info.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
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

    pipeline_info.layout = pipeline.layout;

    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass    = 0;

    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex  = -1;

    result = vkCreateGraphicsPipelines(g_core.device, VK_NULL_HANDLE, 1,
                                       &pipeline_info, NULL, &pipeline.graphics);
    VULKAN_VERIFY(result);

    vkDestroyShaderModule(g_core.device, vertex_shader_module, NULL);
    vkDestroyShaderModule(g_core.device, fragment_shader_module, NULL);

    return pipeline;
}

static VkCommandPool
create_command_pool(VulkanQueueFamilyIndices indices)
{
    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = indices.graphics_family_index;

    VkCommandPool command_pool;
    VkResult result = vkCreateCommandPool(g_core.device, &pool_info,
                                          NULL, &command_pool);
    VULKAN_VERIFY(result);

    return command_pool;
}

static VulkanCommandBuffers
create_command_buffers(VkCommandPool command_pool, u32 count)
{
    VulkanCommandBuffers command_buffers = {0};
    command_buffers.count = count;
    
    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = command_pool;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = count;

    VkResult result = vkAllocateCommandBuffers(g_core.device, &alloc_info,
                                               command_buffers.buffers);
    VULKAN_VERIFY(result);

    return command_buffers;
}

static void
record_command_buffer(VkCommandBuffer command_buffer, u32 image_index,
                      VkBuffer vertex_buffer, VkBuffer index_buffer)
{
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = NULL;

    VkResult result = vkBeginCommandBuffer(command_buffer, &begin_info);
    VULKAN_VERIFY(result);

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = g_render_pass;
    render_pass_info.framebuffer = g_framebuffers.framebuffers[image_index];
    render_pass_info.renderArea.offset.x = 0;
    render_pass_info.renderArea.offset.y = 0;
    render_pass_info.renderArea.extent   = g_swap_chain_info.extent;

    VkClearValue clear_color         = {{{0.2f, 0.2f, 0.2f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues    = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      g_pipeline.graphics);

    VkBuffer vertex_buffers[] = {vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);
    
    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)g_swap_chain_info.extent.width;
    viewport.height = (f32)g_swap_chain_info.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = g_swap_chain_info.extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            g_pipeline.layout, 0, 1,
                            &g_descriptor_sets.sets[g_current_frame], 0, NULL);
    vkCmdDrawIndexed(command_buffer, ARRAY_COUNT(g_indices), 1, 0, 0, 0);
    
    vkCmdEndRenderPass(command_buffer);

    result = vkEndCommandBuffer(command_buffer);
    VULKAN_VERIFY(result);
}

static void
init_sync_objects(void)
{
    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkResult result;
        result = vkCreateSemaphore(g_core.device, &semaphore_info, NULL,
                                   &g_core.sync_objs.image_available_semaphores[i]);
        VULKAN_VERIFY(result);

        result = vkCreateSemaphore(g_core.device, &semaphore_info, NULL,
                                   &g_core.sync_objs.render_finished_semaphores[i]);
        VULKAN_VERIFY(result);

        result = vkCreateFence(g_core.device, &fence_info, NULL,
                               &g_core.sync_objs.in_flight_fences[i]);
        VULKAN_VERIFY(result);
    }
}

static void clean_up_swap_chain()
{
    for (u32 i = 0; i < g_framebuffers.count; ++i) {
        vkDestroyFramebuffer(g_core.device,
                             g_framebuffers.framebuffers[i], NULL);
    }

    for (u32 i = 0; i < g_swap_chain_info.image_views.count; ++i) {
        vkDestroyImageView(g_core.device,
                           g_swap_chain_info.image_views.views[i], NULL);
    }

    vkDestroySwapchainKHR(g_core.device,
                          g_swap_chain_info.swap_chain, NULL);
}

void
recreate_swap_chain()
{
    // TODO: Handle minimization! If window is minimized, application should block!

    vkDeviceWaitIdle(g_core.device);
    clean_up_swap_chain();
    g_swap_chain_info = create_swap_chain(g_queue_family_indices);
    
    g_framebuffers =
        create_framebuffers(g_render_pass,
                            g_swap_chain_info.image_views,
                            g_swap_chain_info.extent);

}

static void
update_uniform_buffer(f64 dt, u32 current_image, VkExtent2D extent, UniformBuffers uniform_buffers)
{
    static f32 time = 0;
    time += (f32)dt;

    UniformBufferObject ubo = {0};
    ubo.model = m4f32_identity();
    /* ubo.model = m4f32_scale(ubo.model, (v3f32){{2.0f, 2.0f, 2.0f}}); */
    /* ubo.model = m4f32_translate(ubo.model, (v3f32){{-1.0, 0.0f, 0.0f}}); */
    ubo.model = m4f32_rotate(ubo.model, time * radians(90.0f), Z_AXIS);
    
    ubo.view = m4f32_look_at((v3f32){{0.0f, -2.0f, 1.0f}},
                             (v3f32){{0.0f, 0.0f, 0.0f}},
                             Z_AXIS);
    ubo.proj = m4f32_perspective(radians(45.0f), extent.width / (f32)extent.height,
                                 0.1f, 10.0f);
    ubo.proj.cols[1].c[1] *= -1;

    memcpy(uniform_buffers.mapped[current_image], &ubo, sizeof(ubo));
}

void
vulkan_draw_frame(f64 dt)
{
    vkWaitForFences(g_core.device, 1,
                    &g_core.sync_objs.in_flight_fences[g_current_frame],
                    VK_TRUE, UINT64_MAX);

    update_uniform_buffer(dt, g_current_frame,
                          g_swap_chain_info.extent,
                          g_uniform_buffers);
    
    u32 image_index;
    VkResult result;
    result = vkAcquireNextImageKHR(g_core.device,
                                   g_swap_chain_info.swap_chain,
                                   UINT64_MAX, g_core.sync_objs.image_available_semaphores[g_current_frame],
                                   VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swap_chain();
        return;
    }
    VULKAN_VERIFY(result);

    vkResetFences(g_core.device, 1,
                  &g_core.sync_objs.in_flight_fences[g_current_frame]);

    vkResetCommandBuffer(g_command_buffers.buffers[g_current_frame], 0);
    record_command_buffer(g_command_buffers.buffers[g_current_frame], image_index, g_vertex_buffer, g_index_buffer);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {
        g_core.sync_objs.image_available_semaphores[g_current_frame]
    };

    VkPipelineStageFlags wait_stages[] =
        { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores    = wait_semaphores;
    submit_info.pWaitDstStageMask  = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &g_command_buffers.buffers[g_current_frame];

    VkSemaphore signal_semaphores[] = {
        g_core.sync_objs.render_finished_semaphores[g_current_frame]
    };

    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    result = vkQueueSubmit(g_queues.graphics, 1, &submit_info,
                           g_core.sync_objs.in_flight_fences[g_current_frame]);
    VULKAN_VERIFY(result);

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = { g_swap_chain_info.swap_chain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;

    result = vkQueuePresentKHR(g_queues.present, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR) {
        recreate_swap_chain();
    } else {
        VULKAN_VERIFY(result);
    }

    g_current_frame = (g_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void
vulkan_wait_for_device_idle(void)
{
    vkDeviceWaitIdle(g_core.device);
}

static u32
find_memory_type(u32 type_filter, VkMemoryPropertyFlags props)
{
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(g_core.phy_device, &mem_props);

    for (u32 i = 0; i < mem_props.memoryTypeCount; ++i) {
        if ((type_filter & (1 << i)) &&
            (mem_props.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }

    FC_ERROR("Failed to find suitable memory type");
    return 0;
}

static void
create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props,
              VkBuffer* buffer, VkDeviceMemory* buffer_memory)
{
    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateBuffer(g_core.device, &buffer_info, NULL, buffer);
    VULKAN_VERIFY(result);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(g_core.device, *buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex =
        find_memory_type(mem_reqs.memoryTypeBits, props);

    result = vkAllocateMemory(g_core.device, &alloc_info, NULL, buffer_memory);
    VULKAN_VERIFY(result);

    vkBindBufferMemory(g_core.device, *buffer, *buffer_memory, 0);
}

static void
copy_buffer(VkCommandPool command_pool, VkQueue graphics_queue,
            VkBuffer src, VkBuffer dest, VkDeviceSize size)
{

    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool; // TODO: Create separate command pool for transient command buffers
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(g_core.device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(command_buffer, &begin_info);
    VkBufferCopy copy_region = {0};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src, dest, 1, &copy_region);
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(g_core.device, command_pool, 1, &command_buffer);
}

static void
create_vertex_buffer(VkCommandPool command_pool, VkQueue graphics_queue,
                                 VkBuffer* buffer, VkDeviceMemory* buffer_memory)
{
    VkDeviceSize buffer_size = sizeof(g_vertices);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &staging_buffer, &staging_buffer_memory);
    
    void* data;
    VkResult result = vkMapMemory(g_core.device, staging_buffer_memory, 0,
                                  buffer_size, 0, &data);
    VULKAN_VERIFY(result);
    
    memcpy(data, g_vertices, buffer_size);
    vkUnmapMemory(g_core.device, staging_buffer_memory);
    
    create_buffer(buffer_size,
                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, buffer_memory);

    copy_buffer(command_pool, graphics_queue,
                staging_buffer, *buffer, buffer_size);

    vkDestroyBuffer(g_core.device, staging_buffer, NULL);
    vkFreeMemory(g_core.device, staging_buffer_memory, NULL);
}

static void
create_index_buffer(VkCommandPool command_pool, VkQueue graphics_queue,
                    VkBuffer* buffer, VkDeviceMemory* buffer_memory)
{
    VkDeviceSize buffer_size = sizeof(g_indices);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &staging_buffer, &staging_buffer_memory);
    
    void* data;
    VkResult result = vkMapMemory(g_core.device, staging_buffer_memory, 0,
                                  buffer_size, 0, &data);
    VULKAN_VERIFY(result);
    memcpy(data, g_indices, buffer_size);
    vkUnmapMemory(g_core.device, staging_buffer_memory);
    
    create_buffer(buffer_size,
                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, buffer_memory);

    copy_buffer(command_pool, graphics_queue,
                staging_buffer, *buffer, buffer_size);

    vkDestroyBuffer(g_core.device, staging_buffer, NULL);
    vkFreeMemory(g_core.device, staging_buffer_memory, NULL);
}

static VkDescriptorSetLayout
create_descriptor_set_layout()
{
    VkDescriptorSetLayoutBinding ubo_layout_binding = {0};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo layout_info = {0};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &ubo_layout_binding;

    VkDescriptorSetLayout layout;
    VkResult result = vkCreateDescriptorSetLayout(g_core.device, &layout_info,
                                                  NULL, &layout);
    VULKAN_VERIFY(result);

    return layout;
}

static UniformBuffers
create_uniform_buffers()
{
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);

    UniformBuffers uniform_buffers = {0};
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        create_buffer(buffer_size,
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      &uniform_buffers.buffers[i],
                      &uniform_buffers.memories[i]);
        
        vkMapMemory(g_core.device, uniform_buffers.memories[i], 0,
                    buffer_size, 0, &uniform_buffers.mapped[i]);
    }

    return uniform_buffers;
}

static VkDescriptorPool
create_descriptor_pool()
{
    VkDescriptorPoolSize pool_size = {0};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = (u32)MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = (u32)MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPool descriptor_pool;
    VkResult result = vkCreateDescriptorPool(g_core.device, &pool_info,
                                             NULL, &descriptor_pool);
    VULKAN_VERIFY(result);
    
    return descriptor_pool;
}

static DescriptorSets
create_descriptor_sets(VkDescriptorSetLayout layout, VkDescriptorPool descriptor_pool,
                       UniformBuffers uniform_buffers)
{
    // Create as many copies of layout as there may be frames in flight
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        layouts[i] = layout;
    }

    VkDescriptorSetAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = (u32)MAX_FRAMES_IN_FLIGHT;
    alloc_info.pSetLayouts = layouts;

    DescriptorSets descriptor_sets;
    VkResult result = vkAllocateDescriptorSets(g_core.device, &alloc_info,
                                               descriptor_sets.sets);
    VULKAN_VERIFY(result);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo buffer_info = {0};
        buffer_info.buffer = uniform_buffers.buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptor_write = {0};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_sets.sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = NULL;
        descriptor_write.pTexelBufferView = NULL;

        vkUpdateDescriptorSets(g_core.device, 1, &descriptor_write, 0, NULL);
    }
    
    return descriptor_sets;
}

void
vulkan_init(ApplicationState* app_state)
{
    FC_INFO("Initializing Vulkan");

    create_instance(app_state);

#ifdef FINCH_LOGGING
    vulkan_init_debug_messenger();
#endif // FINCH_LOGGING
    
    g_core.surface = create_surface(g_core.instance);
    
    pick_physical_device(&g_queue_family_indices);
    setup_logical_device(g_queue_family_indices, &g_queues);
    
    g_swap_chain_info = create_swap_chain(g_queue_family_indices);
    
    g_render_pass =
        create_render_pass(g_swap_chain_info.image_format);

    g_descriptor_set_layout = create_descriptor_set_layout();
    
    g_pipeline =
        create_graphics_pipeline(g_render_pass,
                                 g_descriptor_set_layout);
    
    g_framebuffers =
        create_framebuffers(g_render_pass,
                                   g_swap_chain_info.image_views,
                                   g_swap_chain_info.extent);
    
    g_command_pool = create_command_pool(g_queue_family_indices);
    
    create_vertex_buffer(g_command_pool, g_queues.graphics,
                         &g_vertex_buffer,
                         &g_vertex_buffer_memory);
    
    create_index_buffer(g_command_pool, g_queues.graphics,
                        &g_index_buffer,
                        &g_index_buffer_memory);

    g_uniform_buffers = create_uniform_buffers();

    g_descriptor_pool = create_descriptor_pool();
    g_descriptor_sets =
        create_descriptor_sets(g_descriptor_set_layout,
                               g_descriptor_pool,
                               g_uniform_buffers);

    g_command_buffers =
        create_command_buffers(g_command_pool, MAX_FRAMES_IN_FLIGHT);
    
    init_sync_objects();    

    FC_INFO("Vulkan initialized");
}

void vulkan_deinit()
{
    FC_INFO("Deinitializing Vulkan");

    clean_up_swap_chain();

    vkDestroyPipeline(g_core.device, g_pipeline.graphics, NULL);
    vkDestroyPipelineLayout(g_core.device, g_pipeline.layout, NULL);
    
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroyBuffer(g_core.device, g_uniform_buffers.buffers[i], NULL);
        vkFreeMemory(g_core.device, g_uniform_buffers.memories[i], NULL);
    }

    vkDestroyDescriptorPool(g_core.device, g_descriptor_pool, NULL);
    vkDestroyDescriptorSetLayout(g_core.device, g_descriptor_set_layout, NULL);
    vkDestroyRenderPass(g_core.device, g_render_pass, NULL);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(g_core.device, g_core.sync_objs.image_available_semaphores[i], NULL);
        vkDestroySemaphore(g_core.device, g_core.sync_objs.render_finished_semaphores[i], NULL);
        vkDestroyFence(g_core.device, g_core.sync_objs.in_flight_fences[i], NULL);
    }

    vkDestroyBuffer(g_core.device, g_index_buffer, NULL);
    vkFreeMemory(g_core.device, g_index_buffer_memory, NULL);
    
    vkDestroyBuffer(g_core.device, g_vertex_buffer, NULL);
    vkFreeMemory(g_core.device, g_vertex_buffer_memory, NULL);
    
    vkDestroyCommandPool(g_core.device, g_command_pool, NULL);
    vkDestroyDevice(g_core.device, NULL);
    vkDestroySurfaceKHR(g_core.instance, g_core.surface, NULL);

#ifdef FINCH_LOGGING
    destroy_debug_utils_messenger_EXT(vulkan_debug_messenger, NULL);
#endif // FINCH_LOGGING
    
    vkDestroyInstance(g_core.instance, NULL);

    FC_INFO("Vulkan deinitialized");
}
