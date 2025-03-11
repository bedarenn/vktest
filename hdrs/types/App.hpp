#pragma once

# define GLFW_INCLUDE_VULKAN
# include <GLFW/glfw3.h>
# define GLFW_EXPOSE_NATIVE_X11
# include <GLFW/glfw3native.h>

class	App;

# define W_VK_ARG_VERSION(f)	f(1, 0, 0)
# define W_VK_API_VERSION		VK_API_VERSION_1_0

# define WIDTH 500
# define HEIGHT 500
# define NAME "VK_Test"

# define COLOR_VERBOSE "\033[90m"
# define COLOR_INFO "\033[32m"
# define COLOR_WARNING "\033[33m"
# define COLOR_ERROR "\033[31m"
# define COLOR_STD "\033[0;0m"

# define COLOR_R_FLAG "\033[32m"
# define COLOR_WR_FLAG "\033[31m"
