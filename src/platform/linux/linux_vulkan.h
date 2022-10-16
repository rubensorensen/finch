#ifndef LINUX_VULKAN_H
#define LINUX_VULAN_H

#include "linux_finch.h"

typedef struct _VulkanQueueFamilyIndices {
    b32 has_graphics_family;
    u32 graphics_family;
} VulkanQueueFamilyIndices;

void x11_vulkan_init(X11State* x11_state);
void x11_vulkan_deinit(X11State* x11_state);

#endif // LINUX_VULKAN_H
