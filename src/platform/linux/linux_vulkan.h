#ifndef LINUX_VULKAN_H
#define LINUX_VULAN_H

#include "linux_finch.h"

#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct _VulkanQueueFamilyIndices {
    b32 has_graphics_family;
    u32 graphics_family_index;

    b32 has_present_family;
    u32 present_family_index;
} VulkanQueueFamilyIndices;

typedef struct _VulkanSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;

    u32                      format_count;
    VkSurfaceFormatKHR       formats[1024];

    u32                      present_mode_count;
    VkPresentModeKHR         present_modes[1024];
} VulkanSwapChainSupportDetails;

typedef struct _VulkanSwapChainInfo {
    VkSwapchainKHR swap_chain;
    VkFormat       image_format;
    VkExtent2D     extent;
    
    VkImage        images[100];
    u32            images_count;

    VkImageView    image_views[100];
    u32            image_views_count;
    
    VkFramebuffer  framebuffers[100];
    u32            framebuffers_count;    
} VulkanSwapChainInfo;

typedef struct _VulkanSyncPrimitives {    
    VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence     in_flight_fences[MAX_FRAMES_IN_FLIGHT];
} VulkanSyncPrimitives;

typedef struct _VulkanState {    
    VkInstance       instance;
    VkSurfaceKHR     surface;
    VkPhysicalDevice physical_device;
    VkDevice         device;
    VkQueue          graphics_queue;
    VkQueue          present_queue;

    VkRenderPass     render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline       graphics_pipeline;
    VkCommandPool    command_pool;
    VkCommandBuffer  command_buffers[MAX_FRAMES_IN_FLIGHT];

    VulkanSyncPrimitives     synchronization_primitives;
    VulkanSwapChainInfo      swap_chain_info;
    u32                      current_frame;
    b32                      framebuffer_resized;
} VulkanState;
 
b32 vulkan_queue_family_indices_is_complete(VulkanQueueFamilyIndices* indices);

void x11_vulkan_init(X11State* x11_state);
void x11_vulkan_deinit(X11State* x11_state);

#endif // LINUX_VULKAN_H
