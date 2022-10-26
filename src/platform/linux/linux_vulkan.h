#ifndef LINUX_VULKAN_H
#define LINUX_VULAN_H

#include "linux_finch.h"

#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>

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

b32 vulkan_queue_family_indices_is_complete(VulkanQueueFamilyIndices* indices);

void x11_vulkan_init(X11State* x11_state);
void x11_vulkan_deinit(X11State* x11_state);

#endif // LINUX_VULKAN_H
