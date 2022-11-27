#ifndef _FINCH_VULKAN_H
#define _FINCH_VULKAN_H

#include "finch/core.h"
#include "finch/math.h"

#ifdef FINCH_LINUX
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

void vkresult_to_string(char* buf, VkResult result);

#define VULKAN_VERIFY(result) vulkan_verify(result)
void vulkan_verify(VkResult result);

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

typedef struct _VulkanFramebuffers {
    VkFramebuffer framebuffers[100];
    u32           count;    
} VulkanFramebuffers;

typedef struct _VulkanImageViews {
    VkImageView views[100];
    u32         count;    
} VulkanImageViews;

typedef struct _VulkanImages {
    VkImage images[100];
    u32     count;    
} VulkanImages;

typedef struct _VulkanSwapChainInfo {
    VkSwapchainKHR     swap_chain;
    VkFormat           image_format;
    VkExtent2D         extent;
    VulkanImages       images;
    VulkanImageViews   image_views;
} VulkanSwapChainInfo;

typedef struct _VulkanSyncPrimitives {    
    VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence     in_flight_fences[MAX_FRAMES_IN_FLIGHT];
} VulkanSyncPrimitives;

typedef struct _VulkanQueues {    
    VkQueue          graphics;
    VkQueue          present;
} VulkanQueues;

typedef struct _VulkanPipeline {
    VkPipelineLayout layout;
    VkPipeline       graphics;    
} VulkanPipeline;

typedef struct _VulkanCommandBuffers {
    VkCommandBuffer buffers[100];
    u32             count;
} VulkanCommandBuffers;

typedef struct _Vertex {
    v2f32 position;
    v3f32 color;
} Vertex;

typedef struct _UniformBuffers {
    VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory memories[MAX_FRAMES_IN_FLIGHT];
    void* mapped[MAX_FRAMES_IN_FLIGHT];
} UniformBuffers;

typedef struct _DescriptorSets {
    VkDescriptorSet sets[MAX_FRAMES_IN_FLIGHT];    
} DescriptorSets;

typedef struct _VulkanCore {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice phy_device;
    VkDevice device;
    
    VulkanSyncPrimitives sync_objs;
} VulkanCore;

typedef struct _VulkanAttributeDescriptions {
    VkVertexInputAttributeDescription descs[2];
} VulkanAttributeDescriptions;

typedef struct _UniformBufferObject {
    m4f32 model;
    m4f32 view;
    m4f32 proj;
} UniformBufferObject;

#endif // _FINCH_VULKAN_H
