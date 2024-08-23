#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

typedef struct {
	float pos[2];
	float color[3];
} vk_vertex;

vk_vertex verts[] = {
	{{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
};
uint32_t vert_count = 3;

#define vk_generic_enum(func, first, type, name)\
uint32_t name ## _count = 0;\
func ( first , & name ## _count, NULL);\
type * name ## s = malloc(sizeof(type) * name ## _count);\
func ( first , & name ## _count, name ## s)

#define vk_zero_check_enum(func, first, type, name)\
uint32_t name ## _count = 0;\
func ( first , & name ## _count, NULL);\
if ( name ## _count == 0) vk(VK_ERROR_UNKNOWN);\
type * name ## s = malloc(sizeof(type) * name ## _count);\
func ( first , & name ## _count, name ## s)

#define vk_zero_check_enum_2(func, first, second, type, name)\
uint32_t name ## _count = 0;\
func ( first , second , & name ## _count, NULL);\
if ( name ## _count == 0) vk(VK_ERROR_UNKNOWN);\
type * name ## s = malloc(sizeof(type) * name ## _count);\
func ( first , second , & name ## _count, name ## s)

typedef struct {
	bool has_graphics;
	uint32_t graphics;
	
	bool has_present;
	uint32_t present;
} vk_queue_family_indices_t;

typedef struct {
	VkSurfaceCapabilitiesKHR capabilities;
	
	uint32_t format_count;
	VkSurfaceFormatKHR* formats;
	
	uint32_t present_mode_count;
	VkPresentModeKHR* present_modes;
} vk_swapchain_details_t;

typedef struct {
	uint32_t swapchain_image_count;
	VkImage* swapchain_images;
	VkFormat format;
	VkExtent2D extent;
	VkSwapchainKHR swapchain;
} vk_swapchain_t;

typedef struct {
	uint32_t view_count;
	VkImageView* views;
} vk_image_views_t;

typedef struct {
	uint32_t count;
	VkFramebuffer* data;
} vk_framebuffers_t;

typedef struct {
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkSurfaceKHR surface;
	vk_queue_family_indices_t family_indices;
	vk_swapchain_t swapchain;
	vk_image_views_t image_views;
	VkShaderModule vert_shader;
	VkShaderModule frag_shader;
	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;
	vk_framebuffers_t framebuffers;
	VkCommandPool command_pool;
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_mem;
	VkCommandBuffer command_buffers[2];
	VkSemaphore image_available[2];
	VkSemaphore render_finished[2];
	VkFence in_flight[2];
	uint32_t frame;
} vk_data_t;

vk_data_t vk_data = {0};

// commented out values trigger windows defender for some reason
const char* vk_result_table(VkResult r) {
	switch (r) {
	case VK_SUCCESS: return "VK_SUCCESS";
	case VK_NOT_READY: return "VK_NOT_READY";
	case VK_TIMEOUT: return "VK_TIMEOUT";
	case VK_EVENT_SET: return "VK_EVENT_SET";
	case VK_EVENT_RESET: return "VK_EVENT_RESET";
	case VK_INCOMPLETE: return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
	case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
	case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED";
	case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
//	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
//	case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
//	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
//	case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
//	case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
//	case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
//	case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
//	case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
//	case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
//	case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
//	case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
//	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
//	case VK_ERROR_NOT_PERMITTED_KHR: return "VK_ERROR_NOT_PERMITTED_KHR";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
	case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
	case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
	case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
	case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
	case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
	case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
	case VK_INCOMPATIBLE_SHADER_BINARY_EXT: return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
	default: return "Unknown";
	}
}

#define vk(r) vk_impl(r, __FILE__, __LINE__)
void vk_impl(VkResult r, const char* file, int line) {
	if (r != VK_SUCCESS) {
		const char* errstr = vk_result_table(r);
		printf("Vulkan Error %s at %s:%d", errstr, file, line);
		fflush(stdout);
		volatile char* lol = NULL;
		lol[0] = 0; // CRASH
	}
}

vk_queue_family_indices_t vk_find_queue_family(VkPhysicalDevice device) {
		vk_queue_family_indices_t indices = {0};
		
		vk_zero_check_enum(vkGetPhysicalDeviceQueueFamilyProperties, device, VkQueueFamilyProperties, queue_family);
		
		for (uint32_t i = 0; i < queue_family_count; i++) {
			VkBool32 present = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vk_data.surface, &present);
			if (present) {
				indices.present = i;
				indices.has_present = true;
			}
			if (queue_familys[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphics = i;
				indices.has_graphics = true;
			}
		}
		
		free(queue_familys);
		
		return indices;
}

vk_swapchain_details_t vk_query_swapchain_details(VkPhysicalDevice device) {
	vk_swapchain_details_t details = {0};
	
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vk_data.surface, &details.capabilities);
	
	vk_zero_check_enum_2(vkGetPhysicalDeviceSurfaceFormatsKHR, device, vk_data.surface, VkSurfaceFormatKHR, format);
	
	details.format_count = format_count;
	details.formats = formats;
	
	vk_zero_check_enum_2(vkGetPhysicalDeviceSurfacePresentModesKHR, device, vk_data.surface, VkPresentModeKHR, present_mode);
	
	details.present_mode_count = present_mode_count;
	details.present_modes = present_modes;
	
	return details;
}

VkSurfaceFormatKHR vk_get_surface_format(vk_swapchain_details_t details) {
	for (uint32_t i = 0; i < details.format_count; i++) {
		if (details.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && details.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return details.formats[i];
		}
	}
	vk(VK_ERROR_UNKNOWN);
	return details.formats[0]; // unreached
}

VkPresentModeKHR vk_get_present_mode(vk_swapchain_details_t details) {
	for (uint32_t i = 0; i < details.present_mode_count; i++) {
		if (details.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			return details.present_modes[i];
	}
	
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkInstance vk_create_instance(void) {
	VkApplicationInfo appinfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "vktest",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "vktest",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0,
	};
	
	const char* instance_extensions[] = {
		"VK_KHR_surface", "VK_KHR_win32_surface",
	};
	
	VkInstanceCreateInfo createinfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appinfo,
		.enabledExtensionCount = 2,
		.ppEnabledExtensionNames = instance_extensions,
	};
	
	VkInstance r;
	vk(vkCreateInstance(&createinfo, NULL, &r));
	return r;
}

VkPhysicalDevice vk_create_physical_device(void) {
	vk_zero_check_enum(vkEnumeratePhysicalDevices, vk_data.instance, VkPhysicalDevice, device);
	
	// just choose the last discrete device, or the last overall device if there is no discrete device
	
	uint32_t chosen_device = 0;
	bool has_chosen = false;
	for (uint32_t i = 0; i < device_count; i++) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(devices[i], &props);
		
		if (vk_find_queue_family(devices[chosen_device]).has_graphics) {
			has_chosen = true;
			chosen_device = i;
		}
		
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			chosen_device = i;
			break;
		}
	}
	vk_queue_family_indices_t indices = vk_find_queue_family(devices[chosen_device]);
	if (!indices.has_graphics) vk(VK_ERROR_UNKNOWN);
	if (!indices.has_present) vk(VK_ERROR_UNKNOWN);
	
	VkPhysicalDevice r = devices[chosen_device];
	
	free(devices);
	
	return r;
}

VkDevice vk_create_logical_device(void) {
	vk_queue_family_indices_t indices = vk_find_queue_family(vk_data.physical_device);
	
	float queue_priority = 1.0f;
	
	VkDeviceQueueCreateInfo queue_createinfo[] = {
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = indices.graphics,
			.queueCount = 1,
			.pQueuePriorities = &queue_priority,
		},
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = indices.present,
			.queueCount = 1,
			.pQueuePriorities = &queue_priority,
		}
	};
	
	VkPhysicalDeviceFeatures device_features = {0};
	
	const char* device_extensions[] = {
		"VK_KHR_swapchain"
	};
	
	VkDeviceCreateInfo device_createinfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pQueueCreateInfos = queue_createinfo,
		.queueCreateInfoCount = 1,
		.pEnabledFeatures = &device_features,
		.enabledExtensionCount = 1,
		.ppEnabledExtensionNames = device_extensions,
	};
	
	if (indices.graphics == indices.present)
		device_createinfo.queueCreateInfoCount = 1;
	else
		device_createinfo.queueCreateInfoCount = 2;
	
	VkDevice r;
	
	vk(vkCreateDevice(vk_data.physical_device, &device_createinfo, NULL, &r));
	
	return r;
}

VkQueue vk_create_queue(uint32_t family) {
	VkQueue r;
	vkGetDeviceQueue(vk_data.device, family, 0, &r);
	return r;
}

VkSurfaceKHR vk_create_surface(void* hWnd, void* hInstance) {
	VkWin32SurfaceCreateInfoKHR surface_createinfo = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hwnd = hWnd,
		.hinstance = hInstance,
	};
	
	VkSurfaceKHR r;
	vk(vkCreateWin32SurfaceKHR(vk_data.instance, &surface_createinfo, NULL, &r));
	return r;
}

vk_swapchain_t vk_create_swapchain(uint32_t w, uint32_t h, vk_queue_family_indices_t indices) {
	vk_swapchain_details_t details = vk_query_swapchain_details(vk_data.physical_device);
	
	VkSurfaceFormatKHR format = vk_get_surface_format(details);
	VkPresentModeKHR present_mode = vk_get_present_mode(details);
	VkExtent2D extent = { w, h };
	
	uint32_t image_count = details.capabilities.minImageCount + 1;
	if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount)
		image_count = details.capabilities.maxImageCount;
	
	VkSwapchainCreateInfoKHR createinfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = vk_data.surface,
		.minImageCount = image_count,
		.imageFormat = format.format,
		.imageColorSpace = format.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = details.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};
	
	uint32_t family_indices[] = { indices.graphics, indices.present };
	
	if (indices.graphics == indices.present) {
		createinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	} else {
		createinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createinfo.queueFamilyIndexCount = 2;
		createinfo.pQueueFamilyIndices = family_indices;
	}
	
	vk_swapchain_t r;
	vk(vkCreateSwapchainKHR(vk_data.device, &createinfo, NULL, &r.swapchain));
	
	free(details.formats);
	free(details.present_modes);
	
	r.format = format.format;
	r.extent = extent;
	
	vkGetSwapchainImagesKHR(vk_data.device, r.swapchain, &r.swapchain_image_count, NULL);
	r.swapchain_images = malloc(r.swapchain_image_count * sizeof(VkImage));
	vkGetSwapchainImagesKHR(vk_data.device, r.swapchain, &r.swapchain_image_count, r.swapchain_images);
	
	return r;
}

vk_image_views_t vk_create_image_views(void) {
	vk_image_views_t r;
	r.view_count = vk_data.swapchain.swapchain_image_count;
	r.views = malloc(r.view_count * sizeof(VkImageView));
	
	for (uint32_t i = 0; i < r.view_count; i++) {
		VkImageViewCreateInfo createinfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = vk_data.swapchain.swapchain_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = vk_data.swapchain.format,
			.components = { VK_COMPONENT_SWIZZLE_IDENTITY },
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		
		vk(vkCreateImageView(vk_data.device, &createinfo, NULL, &r.views[i]));
	}
	
	return r;
}

VkShaderModule vk_create_shader(const char* src) {
	uint32_t length = 0;
	unsigned char* data = NULL;
	
	FILE* fp = fopen(src, "rb");
	
	assert(fp != NULL);
	
	fseek(fp, 0, SEEK_END);
	
	length = ftell(fp);
	
	rewind(fp);
	
	data = malloc(length);
	assert(data != NULL);
	
	assert(fread(data, 1, length, fp) == length);
	
	fclose(fp);
	
	VkShaderModuleCreateInfo createinfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = length,
		.pCode = (const uint32_t*)data,
	};
	
	VkShaderModule r;
	vk(vkCreateShaderModule(vk_data.device, &createinfo, NULL, &r));
	
	return r;
}

VkPipeline vk_create_graphics_pipeline(void) {
	VkPipelineShaderStageCreateInfo vert_shader_createinfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vk_data.vert_shader,
		.pName = "main",
	};
	
	VkPipelineShaderStageCreateInfo frag_shader_createinfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = vk_data.frag_shader,
		.pName = "main",
	};
	
	VkPipelineShaderStageCreateInfo stages[] = { vert_shader_createinfo, frag_shader_createinfo };
	
	VkDynamicState states[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	
	VkPipelineDynamicStateCreateInfo state_createinfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = states,
	};
	
	VkVertexInputBindingDescription vertex_input_binding_desc = {
		.binding = 0,
		.stride = sizeof(vk_vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	
	VkVertexInputAttributeDescription vertex_input_attrs[2] = {
		{
			.binding = 0,
			.location = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(vk_vertex, pos),
		},
		{
			.binding = 0,
			.location = 1,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(vk_vertex, color),
		},
	};
	
	VkPipelineVertexInputStateCreateInfo vertex_input_createinfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &vertex_input_binding_desc,
		.vertexAttributeDescriptionCount = 2,
		.pVertexAttributeDescriptions = vertex_input_attrs,
	};
	
	VkPipelineInputAssemblyStateCreateInfo input_assembly_createinfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};
	
	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = vk_data.swapchain.extent.width,
		.height = vk_data.swapchain.extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	
	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = vk_data.swapchain.extent,
	};
	
	VkPipelineViewportStateCreateInfo viewport_state_createinfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};
	
	VkPipelineRasterizationStateCreateInfo rasterization_state_createinfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.lineWidth = 1.0f,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
	};
	
	VkPipelineMultisampleStateCreateInfo sampling_state_createinfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.sampleShadingEnable = VK_FALSE,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.minSampleShading = 1.0f,
		.pSampleMask = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};
	
	VkPipelineColorBlendAttachmentState blend_attachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE,
	};
	
	VkPipelineColorBlendStateCreateInfo blend_createinfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1,
		.pAttachments = &blend_attachment,
	};
	
	VkGraphicsPipelineCreateInfo pipeline_create_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = stages,
		.pVertexInputState = &vertex_input_createinfo,
		.pInputAssemblyState = &input_assembly_createinfo,
		.pViewportState = &viewport_state_createinfo,
		.pRasterizationState = &rasterization_state_createinfo,
		.pMultisampleState = &sampling_state_createinfo,
		.pDepthStencilState = NULL,
		.pColorBlendState = &blend_createinfo,
		.pDynamicState = &state_createinfo,
		.layout = vk_data.pipeline_layout,
		.renderPass = vk_data.render_pass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};
	
	VkPipeline r;
	vk(vkCreateGraphicsPipelines(vk_data.device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &r));
	return r;
}

VkPipelineLayout vk_create_pipeline_layout(void) {
	VkPipelineLayoutCreateInfo layout_createinfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	};
	
	VkPipelineLayout r;
	vk(vkCreatePipelineLayout(vk_data.device, &layout_createinfo, NULL, &r));
	return r;
}

VkRenderPass vk_create_render_pass(void) {
	VkAttachmentDescription color = {
		.format = vk_data.swapchain.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};
	
	VkAttachmentReference reference = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	
	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &reference,
	};
	
	VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
	};
	
	VkRenderPassCreateInfo render_pass_create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &color,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};
	
	VkRenderPass r;
	vk(vkCreateRenderPass(vk_data.device, &render_pass_create_info, NULL, &r));
	return r;
}

vk_framebuffers_t vk_create_framebuffers(void) {
	vk_framebuffers_t r;
	r.count = vk_data.image_views.view_count;
	r.data = malloc(sizeof(VkFramebuffer) * r.count);
	
	for (uint32_t i = 0; i < r.count; i++) {
		VkImageView attachments[] = {
			vk_data.image_views.views[i]
		};
		
		VkFramebufferCreateInfo framebuffer_createinfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = vk_data.render_pass,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = vk_data.swapchain.extent.width,
			.height = vk_data.swapchain.extent.height,
			.layers = 1,
		};
		
		vk(vkCreateFramebuffer(vk_data.device, &framebuffer_createinfo, NULL, &r.data[i]));
	}
	
	return r;
}

VkCommandPool vk_create_command_pool(vk_queue_family_indices_t indices) {
	VkCommandPoolCreateInfo pool_createinfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = indices.graphics,
	};
	
	VkCommandPool r;
	vk(vkCreateCommandPool(vk_data.device, &pool_createinfo, NULL, &r));
	return r;
}

uint32_t vk_find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags props) {
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(vk_data.physical_device, &mem_props);
	
	for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & props) == props) {
			return i;
		}
	}
	
	vk(VK_ERROR_UNKNOWN);
	return 0;
}

VkBuffer vk_create_vertex_buffer(void) {
	VkBufferCreateInfo buffer_createinfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = sizeof(verts[0]) * vert_count,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	
	VkBuffer r;
	vk(vkCreateBuffer(vk_data.device, &buffer_createinfo, NULL, &r));
	return r;
}

VkDeviceMemory vk_alloc_buffer_mem(VkBuffer buffer) {
	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(vk_data.device, buffer, &mem_reqs);
	
	VkMemoryAllocateInfo allocinfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_reqs.size,
		.memoryTypeIndex = vk_find_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
	};
	
	VkDeviceMemory r;
	vk(vkAllocateMemory(vk_data.device, &allocinfo, NULL, &r));
	
	vkBindBufferMemory(vk_data.device, buffer, r, 0);
	
	return r;
}

void vk_buffer_set_data(VkDeviceMemory mem, void* data, size_t len) {
	void* dst;
	vkMapMemory(vk_data.device, mem, 0, len, 0, &dst);
	memcpy(dst, data, len);
	vkUnmapMemory(vk_data.device, mem);
}

VkCommandBuffer vk_create_command_buffer(void) {
	VkCommandBufferAllocateInfo allocinfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = vk_data.command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	
	VkCommandBuffer r;
	vk(vkAllocateCommandBuffers(vk_data.device, &allocinfo, &r));
	return r;
}

void vk_record(VkCommandBuffer command_buffer, uint32_t index) {
	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = NULL,
	};
	
	vk(vkBeginCommandBuffer(command_buffer, &begin_info));
	
	VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	VkRenderPassBeginInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = vk_data.render_pass,
		.framebuffer = vk_data.framebuffers.data[index],
		.renderArea = {
			.offset = { 0, 0 },
			.extent = vk_data.swapchain.extent,
		},
		.clearValueCount = 1,
		.pClearValues = &clear_color,
	};
	
	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_data.pipeline);
	
	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = vk_data.swapchain.extent.width,
		.height = vk_data.swapchain.extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	
	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = vk_data.swapchain.extent,
	};
	
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_data.vertex_buffer, offsets);
	
	vkCmdDraw(command_buffer, 3, 1, 0, 0);
	
	vkCmdEndRenderPass(command_buffer);
	vk(vkEndCommandBuffer(command_buffer));
}

VkSemaphore vk_create_semaphore(void) {
	VkSemaphoreCreateInfo createinfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	
	VkSemaphore r;
	vk(vkCreateSemaphore(vk_data.device, &createinfo, NULL, &r));
	return r;
}

VkFence vk_create_fence(bool signaled) {
	VkFenceCreateInfo createinfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0,
	};
	
	VkFence r;
	vk(vkCreateFence(vk_data.device, &createinfo, NULL, &r));
	return r;
}

void vk_draw(void) {
	vkWaitForFences(vk_data.device, 1, &vk_data.in_flight[vk_data.frame], VK_TRUE, UINT64_MAX);
	vkResetFences(vk_data.device, 1, &vk_data.in_flight[vk_data.frame]);
	
	uint32_t image_index;
	vkAcquireNextImageKHR(vk_data.device, vk_data.swapchain.swapchain, UINT64_MAX, vk_data.image_available[vk_data.frame], VK_NULL_HANDLE, &image_index);
	
	vkResetCommandBuffer(vk_data.command_buffers[vk_data.frame], 0);
	vk_record(vk_data.command_buffers[vk_data.frame], image_index);
	
	VkSemaphore wait_semaphores[] = { vk_data.image_available[vk_data.frame] };
	VkPipelineStageFlags wait_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	
	VkSemaphore signals[] = { vk_data.render_finished[vk_data.frame] };
	
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = &wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &vk_data.command_buffers[vk_data.frame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signals,
	};
	
	vk(vkQueueSubmit(vk_data.graphics_queue, 1, &submit_info, vk_data.in_flight[vk_data.frame]));
	
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signals,
		.swapchainCount = 1,
		.pSwapchains = &vk_data.swapchain.swapchain,
		.pImageIndices = &image_index,
		.pResults = NULL,
	};
	
	vkQueuePresentKHR(vk_data.present_queue, &present_info);
	
	vk_data.frame = (vk_data.frame + 1) % 2;
}

void vk_update_swapchain(int w, int h) {
	vkDeviceWaitIdle(vk_data.device);
	
	for (uint32_t i = 0; i < vk_data.framebuffers.count; i++) {
		vkDestroyFramebuffer(vk_data.device, vk_data.framebuffers.data[i], NULL);
	}
	for (uint32_t i = 0; i < vk_data.image_views.view_count; i++) {
		vkDestroyImageView(vk_data.device, vk_data.image_views.views[i], NULL);
	}
	vkDestroySwapchainKHR(vk_data.device, vk_data.swapchain.swapchain, NULL);
	
	vk_data.swapchain = vk_create_swapchain(w, h, vk_data.family_indices);
	vk_data.image_views = vk_create_image_views();
	vk_data.framebuffers = vk_create_framebuffers();
	
}

void vk_init(void* hWnd, void* hInstance, uint32_t w, uint32_t h) {
	vk(volkInitialize());
	vk_data.instance = vk_create_instance();
	volkLoadInstance(vk_data.instance);
	
	vk_data.surface = vk_create_surface(hWnd, hInstance);
	vk_data.physical_device = vk_create_physical_device();
	vk_data.device = vk_create_logical_device();
	
	vk_data.family_indices = vk_find_queue_family(vk_data.physical_device);
	
	vk_data.graphics_queue = vk_create_queue(vk_data.family_indices.graphics);
	vk_data.present_queue = vk_create_queue(vk_data.family_indices.present);
	
	vk_data.swapchain = vk_create_swapchain(w, h, vk_data.family_indices);
	vk_data.image_views = vk_create_image_views();
	
	vk_data.vert_shader = vk_create_shader("monke.vert.spv");
	vk_data.frag_shader = vk_create_shader("monke.frag.spv");
	
	vk_data.render_pass = vk_create_render_pass();
	vk_data.pipeline_layout = vk_create_pipeline_layout();
	vk_data.pipeline = vk_create_graphics_pipeline();
	vk_data.framebuffers = vk_create_framebuffers();
	vk_data.command_pool = vk_create_command_pool(vk_data.family_indices);
	
	vk_data.vertex_buffer = vk_create_vertex_buffer();
	vk_data.vertex_buffer_mem = vk_alloc_buffer_mem(vk_data.vertex_buffer);
	vk_buffer_set_data(vk_data.vertex_buffer_mem, verts, sizeof(verts[0]) * vert_count);
	
	vk_data.command_buffers[0] = vk_create_command_buffer();
	vk_data.image_available[0] = vk_create_semaphore();
	vk_data.render_finished[0] = vk_create_semaphore();
	vk_data.in_flight[0] = vk_create_fence(true);
	
	vk_data.command_buffers[1] = vk_create_command_buffer();
	vk_data.image_available[1] = vk_create_semaphore();
	vk_data.render_finished[1] = vk_create_semaphore();
	vk_data.in_flight[1] = vk_create_fence(true);
}

void vk_cleanup(void) {
	vkDeviceWaitIdle(vk_data.device);
	for (uint32_t i = 0; i < 2; i++) {
		vkDestroySemaphore(vk_data.device, vk_data.image_available[i], NULL);
		vkDestroySemaphore(vk_data.device, vk_data.render_finished[i], NULL);
		vkDestroyFence(vk_data.device, vk_data.in_flight[i], NULL);
	}
	vkDestroyBuffer(vk_data.device, vk_data.vertex_buffer, NULL);
	vkFreeMemory(vk_data.device, vk_data.vertex_buffer_mem, NULL);
	vkDestroyCommandPool(vk_data.device, vk_data.command_pool, NULL);
	for (uint32_t i = 0; i < vk_data.framebuffers.count; i++) {
		vkDestroyFramebuffer(vk_data.device, vk_data.framebuffers.data[i], NULL);
	}
	free(vk_data.framebuffers.data);
	vkDestroyPipeline(vk_data.device, vk_data.pipeline, NULL);
	vkDestroyPipelineLayout(vk_data.device, vk_data.pipeline_layout, NULL);
	vkDestroyRenderPass(vk_data.device, vk_data.render_pass, NULL);
	vkDestroyShaderModule(vk_data.device, vk_data.vert_shader, NULL);
	vkDestroyShaderModule(vk_data.device, vk_data.frag_shader, NULL);
	for (uint32_t i = 0; i < vk_data.image_views.view_count; i++) {
		vkDestroyImageView(vk_data.device, vk_data.image_views.views[i], NULL);
	}
	vkDestroySwapchainKHR(vk_data.device, vk_data.swapchain.swapchain, NULL);
	vkDestroyDevice(vk_data.device, NULL);
	vkDestroySurfaceKHR(vk_data.instance, vk_data.surface, NULL);
	vkDestroyInstance(vk_data.instance, NULL);
}
