#include "finch/vulkan.h"
#include "finch/linux_finch.h"

extern X11State* x11_get_ptr_to_state(void);

VkSurfaceKHR vulkan_create_surface(VkInstance instance)
{
    X11State* x11_state = x11_get_ptr_to_state();
    
    VkXlibSurfaceCreateInfoKHR create_info = {0};
    create_info.sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    create_info.dpy    = x11_state->display;
    create_info.window = x11_state->window;

    VkSurfaceKHR surface;
    VkResult result = vkCreateXlibSurfaceKHR(instance, &create_info,
                                             NULL, &surface);
    
    VULKAN_VERIFY(result);

    return surface;
}
