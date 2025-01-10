#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <array>
#include <kata/core/error.hpp>
#include <kata/rhi/context.hpp>
#include <spdlog/spdlog.h>
#include <vector>

namespace kata {
static Result<VkInstance> create_instance()
{
    std::vector<char const*> extensions {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };

    std::vector<char const*> layers {
        "VK_LAYER_KHRONOS_validation",
    };

    VkApplicationInfo application_info {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "kata",
        .applicationVersion = 1,
        .pEngineName = "kata",
        .engineVersion = 1,
        .apiVersion = VK_API_VERSION_1_3,
    };

    VkInstanceCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = uint32_t(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = uint32_t(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    VkInstance instance { VK_NULL_HANDLE };
    auto result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to create Vulkan instance");
    }

    volkLoadInstance(instance);

    return instance;
}

VkBool32 VKAPI_PTR debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
    void* pUserData)
{
    if (!pCallbackData) {
        return VK_FALSE;
    }

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        spdlog::error(pCallbackData->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        spdlog::warn(pCallbackData->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        spdlog::info(pCallbackData->pMessage);
    } else {
        spdlog::info(pCallbackData->pMessage);
    }

    return VK_FALSE;
}

static Result<VkDebugUtilsMessengerEXT> create_debug_utils_messenger(VkInstance instance)
{
    VkDebugUtilsMessageSeverityFlagsEXT severity = 0
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkDebugUtilsMessageTypeFlagsEXT message_type = 0
        | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = severity,
        .messageType = message_type,
        .pfnUserCallback = debug_messenger_callback,
        .pUserData = nullptr,
    };

    VkDebugUtilsMessengerEXT messenger { VK_NULL_HANDLE };
    auto result = vkCreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &messenger);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to create Vulkan debug messenger");
    }

    return messenger;
}

static Result<VkSurfaceKHR> create_surface(VkInstance instance, HWND hwnd)
{
    HINSTANCE hinstance = GetModuleHandleW(NULL);

    VkWin32SurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = hinstance,
        .hwnd = hwnd,
    };

    VkSurfaceKHR surface { VK_NULL_HANDLE };
    auto result = vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to create Vulkan surface");
    }

    return surface;
}

static Result<SelectedPhysicalDevice> select_physical_device(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t count { 0 };
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(count);
    vkEnumeratePhysicalDevices(instance, &count, physical_devices.data());

    for (auto device : physical_devices) {
        SelectedPhysicalDevice physical_device {};

        physical_device.device = device;

        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(device, &properties);

        physical_device.name = properties.deviceName;

        spdlog::info("trying {}", physical_device.name);

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
            spdlog::info("  rejected for being VIRTUAL_GPU");

            continue;
        }

        // Pick queue family

        uint32_t count { 0 };
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queue_family_properties.data());

        for (size_t i = 0; i < count; i++) {
            auto const& properties = queue_family_properties[i];

            bool has_graphics = properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool has_compute = properties.queueFlags & VK_QUEUE_COMPUTE_BIT;

            VkBool32 surface_supported { VK_FALSE };
            auto result = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &surface_supported);
            if (result != VK_SUCCESS) {
                return Error::with_message("can't get surface support status for queue");
            }

            bool has_present = surface_supported == VK_TRUE;

            if (has_graphics && has_compute && has_present) {
                physical_device.queue_family = i;
                break;
            }
        }

        if (physical_device.queue_family == VK_QUEUE_FAMILY_IGNORED) {
            spdlog::info("  rejected for having no graphics+compute+present queue");

            continue;
        }

        // Get surface parameters

        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
        std::vector<VkSurfaceFormatKHR> surface_formats(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, surface_formats.data());

        if (count == 0) {
            spdlog::info("  rejected for having no present formats");

            continue;
        }

        physical_device.surface_format = surface_formats[0];

        spdlog::info("selecting {} as physical device", physical_device.name);

        return physical_device;
    }

    return Error::with_message("no supported GPU found");
}

Result<VkDevice> create_device(SelectedPhysicalDevice physical_device)
{
    float single_queue_priority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queue_create_info = {
        VkDeviceQueueCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = physical_device.queue_family,
            .queueCount = 1,
            .pQueuePriorities = &single_queue_priority,
        }
    };

    std::vector<char const*> extensions {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    };

    VkPhysicalDeviceVulkan12Features features12 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .timelineSemaphore = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features features13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features12,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };

    VkDeviceCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features13,
        .queueCreateInfoCount = uint32_t(queue_create_info.size()),
        .pQueueCreateInfos = queue_create_info.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = uint32_t(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
        .pEnabledFeatures = nullptr,
    };

    VkDevice device { VK_NULL_HANDLE };
    auto result = vkCreateDevice(physical_device.device, &create_info, nullptr, &device);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to create device");
    }

    volkLoadDevice(device);

    return device;
}

struct SwapchainCreateInfo {
    uint32_t width { 0 };
    uint32_t height { 0 };
    VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;
};

Result<VkSwapchainKHR> create_swapchain(
    VkDevice device,
    SelectedPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    SwapchainCreateInfo info)
{
    // FIXME: Available surface extent should be checked before creating swapchain.
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device.device, surface, &capabilities);

    VkSwapchainCreateInfoKHR create_info {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = 2,
        .imageFormat = physical_device.surface_format.format,
        .imageColorSpace = physical_device.surface_format.colorSpace,
        .imageExtent = VkExtent2D {
            .width = info.width,
            .height = info.height,
        },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &physical_device.queue_family,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = info.old_swapchain,
    };

    VkSwapchainKHR swapchain { VK_NULL_HANDLE };
    auto result = vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to create swapchain");
    }

    return swapchain;
}

Result<std::vector<SwapchainFrame>> create_swapchain_frames(
    VkDevice device,
    VkSwapchainKHR swapchain,
    VkCommandPool pool,
    VkFormat image_format)
{
    uint32_t count = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
    std::vector<VkImage> swapchain_images(count);
    vkGetSwapchainImagesKHR(device, swapchain, &count, swapchain_images.data());

    std::vector<SwapchainFrame> frames;

    for (int i = 0; i < count; i++) {
        SwapchainFrame frame;

        frame.swapchain_image = swapchain_images[i];

        VkImageViewCreateInfo image_view_create_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = frame.swapchain_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = image_format,
            .components = VkComponentMapping {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        auto result = vkCreateImageView(device, &image_view_create_info, nullptr, &frame.swapchain_image_view);
        if (result != VK_SUCCESS) {
            return Error::with_message("unable to create image view for swapchain image");
        }

        VkSemaphoreCreateInfo semaphore_create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        result = vkCreateSemaphore(device, &semaphore_create_info, nullptr, &frame.acquire_semaphore);
        if (result != VK_SUCCESS) {
            return Error::with_message("unable to create acquire semaphore");
        }

        auto [command_list, command_list_err] = GPUCommandList::create(device, pool)
            OR_RETURN(command_list_err);

        frame.command_list = std::move(command_list);

        frames.push_back(std::move(frame));
    }

    return frames;
}

Result<QueueSync> create_queue_sync(VkDevice device)
{
    QueueSync queue_sync {};

    VkSemaphoreTypeCreateInfo timeline_type_create_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 0,
    };

    VkSemaphoreCreateInfo timeline_create_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &timeline_type_create_info,
    };

    auto result = vkCreateSemaphore(device, &timeline_create_info, nullptr, &queue_sync.timeline_semaphore);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to create timeline semaphore");
    }

    VkSemaphoreCreateInfo semaphore_create_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    result = vkCreateSemaphore(device, &semaphore_create_info, nullptr, &queue_sync.present_semaphore);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to create present semaphore");
    }

    result = vkCreateSemaphore(device, &semaphore_create_info, nullptr, &queue_sync.next_acquire_semaphore);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to create next acquire semaphore");
    }

    return queue_sync;
}

Result<VkCommandPool> create_command_pool(VkDevice device, uint32_t queue_family_index)
{
    VkCommandPoolCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_index,
    };

    VkCommandPool command_pool { VK_NULL_HANDLE };
    auto result = vkCreateCommandPool(device, &create_info, nullptr, &command_pool);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to create command pool");
    }

    return command_pool;
}

Result<GPUContext> GPUContext::with_window(Window const& window)
{
    if (auto result = volkInitialize(); result != VK_SUCCESS) {
        return Error::with_message("unable to load Vulkan");
    }

    auto [instance, err] = create_instance()
        OR_RETURN(err);
    auto [messenger, msg_err] = create_debug_utils_messenger(instance)
        OR_RETURN(msg_err);
    auto [surface, surface_err] = create_surface(instance, static_cast<HWND>(window.hwnd()))
        OR_RETURN(surface_err);
    auto [physical_device, pd_err] = select_physical_device(instance, surface)
        OR_RETURN(pd_err);
    auto [device, device_err] = create_device(physical_device)
        OR_RETURN(device_err);

    VkQueue queue { VK_NULL_HANDLE };
    vkGetDeviceQueue(device, physical_device.queue_family, 0, &queue);

    auto [command_pool, command_pool_err] = create_command_pool(device, physical_device.queue_family)
        OR_RETURN(command_pool_err);

    auto window_size = window.inner_size();

    SwapchainCreateInfo swapchain_info {
        .width = window_size.width,
        .height = window_size.height,
    };

    auto [swapchain, swapchain_err] = create_swapchain(device, physical_device, surface, swapchain_info);

    auto [swapchain_frames, frames_err] = create_swapchain_frames(device, swapchain, command_pool, physical_device.surface_format.format)
        OR_RETURN(frames_err);

    auto [queue_sync, queue_sync_err] = create_queue_sync(device)
        OR_RETURN(queue_sync_err);

    return GPUContext(instance, messenger, surface, physical_device, device, queue, command_pool, swapchain, std::move(swapchain_frames), queue_sync);
}

GPUContext::~GPUContext()
{
    if (m_device) {
        vkDeviceWaitIdle(m_device);

        for (auto& frame : m_swapchain_frames) {
            vkDestroySemaphore(m_device, frame.acquire_semaphore, nullptr);
            vkDestroyImageView(m_device, frame.swapchain_image_view, nullptr);
        }

        m_swapchain_frames.clear(); // RAII drop command lists

        vkDestroySemaphore(m_device, m_queue_sync.present_semaphore, nullptr);
        vkDestroySemaphore(m_device, m_queue_sync.timeline_semaphore, nullptr);
        vkDestroySemaphore(m_device, m_queue_sync.next_acquire_semaphore, nullptr);

        if (m_swapchain) {
            vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        }

        vkDestroyCommandPool(m_device, m_command_pool, nullptr);

        vkDestroyDevice(m_device, nullptr);
    }

    if (m_instance) {
        if (m_surface) {
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        }

        if (m_debug_messenger) {
            vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
        }

        vkDestroyInstance(m_instance, nullptr);
    }
}

Result<CurrentFrame> GPUContext::begin_frame()
{
    constexpr uint64_t TIMEOUT = 5'000'000'000;

    uint32_t frame_index { 0 };
    auto result = vkAcquireNextImageKHR(m_device, m_swapchain, TIMEOUT, m_queue_sync.next_acquire_semaphore, VK_NULL_HANDLE, &frame_index);
    if (result != VK_SUCCESS) {
        return Error::with_message("unable to acquire next swapchain image");
    }

    auto& frame = m_swapchain_frames[frame_index];

    std::swap(m_queue_sync.next_acquire_semaphore, frame.acquire_semaphore);

    // If this blocks, CPU side is going too fast for swapchain
    std::array<VkSemaphore, 1> semaphores { m_queue_sync.timeline_semaphore };
    std::array<uint64_t, 1> values { frame.prev_progress };

    VkSemaphoreWaitInfo wait_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = uint32_t(semaphores.size()),
        .pSemaphores = semaphores.data(),
        .pValues = values.data(),
    };

    vkWaitSemaphores(m_device, &wait_info, TIMEOUT);

    frame.command_list.reset();
    frame.command_list.begin();

    return CurrentFrame(frame_index);
}

void GPUContext::end_frame(CurrentFrame current_frame)
{
    auto& frame = m_swapchain_frames[current_frame.m_index];

    VkImageMemoryBarrier2 image_barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = 0,
        .srcAccessMask = 0,
        .dstStageMask = 0,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = 0,
        .dstQueueFamilyIndex = 0,
        .image = frame.swapchain_image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VkDependencyInfo dependency_info {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .memoryBarrierCount = 0,
        .pMemoryBarriers = nullptr,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = nullptr,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(frame.command_list.m_buffer, &dependency_info);

    frame.command_list.finish();

    m_queue_sync.progress++;

    // FIXME: variable amount of parallel command buffers
    std::array<VkCommandBuffer, 1> command_buffers {
        frame.command_list.raw(),
    };

    std::array<VkSemaphore, 1> wait_semaphores {
        frame.acquire_semaphore,
    };
    std::array<VkSemaphore, 2> signal_semaphores {
        m_queue_sync.timeline_semaphore,
        m_queue_sync.present_semaphore,
    };

    std::array<uint64_t, 1> wait_values { 0 };
    std::array<uint64_t, 2> signal_values { m_queue_sync.progress, 0 };

    std::array<VkPipelineStageFlags, 1> wait_stages {
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    };

    VkTimelineSemaphoreSubmitInfo timeline_submit_info {
        .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        .waitSemaphoreValueCount = uint32_t(wait_values.size()),
        .pWaitSemaphoreValues = wait_values.data(),
        .signalSemaphoreValueCount = uint32_t(signal_values.size()),
        .pSignalSemaphoreValues = signal_values.data(),
    };

    VkSubmitInfo submit_info {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = &timeline_submit_info,
        .waitSemaphoreCount = uint32_t(wait_semaphores.size()),
        .pWaitSemaphores = wait_semaphores.data(),
        .pWaitDstStageMask = wait_stages.data(),
        .commandBufferCount = uint32_t(command_buffers.size()),
        .pCommandBuffers = command_buffers.data(),
        .signalSemaphoreCount = uint32_t(signal_semaphores.size()),
        .pSignalSemaphores = signal_semaphores.data(),
    };

    auto result = vkQueueSubmit(m_queue, 1, &submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        spdlog::error("unable to submit command buffers");
    }

    VkPresentInfoKHR present_info {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_queue_sync.present_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain,
        .pImageIndices = &current_frame.m_index,
        .pResults = nullptr,
    };

    result = vkQueuePresentKHR(m_queue, &present_info);
    if (result != VK_SUCCESS) {
        spdlog::error("unable to present image");
    }

    frame.prev_progress = m_queue_sync.progress;
}

GPUCommandList& GPUContext::get_command_list_for_frame(CurrentFrame const& frame)
{
    return m_swapchain_frames[frame.m_index].command_list;
}

TextureView GPUContext::get_texture_view_for_frame(CurrentFrame const& frame)
{
    auto const& swapchain_frame = m_swapchain_frames[frame.m_index];

    return TextureView {
        .image_view = swapchain_frame.swapchain_image_view,
        .image = swapchain_frame.swapchain_image,
    };
}

void GPUContext::resize_swapchain(uint32_t width, uint32_t height)
{
    spdlog::info("recreating swapchain: {}x{}", width, height);

    // Wait until render queue is finished before destroying old swapchain
    vkQueueWaitIdle(m_queue);

    // Get rid of old frames
    for (auto& frame : m_swapchain_frames) {
        vkDestroySemaphore(m_device, frame.acquire_semaphore, nullptr);
        vkDestroyImageView(m_device, frame.swapchain_image_view, nullptr);
    }

    m_swapchain_frames.clear();

    // Create new swapchain

    SwapchainCreateInfo swapchain_info {
        .width = width,
        .height = height,
        .old_swapchain = m_swapchain,
    };

    auto [swapchain, swapchain_err] = create_swapchain(m_device, m_physical_device, m_surface, swapchain_info);
    if (swapchain_err) {
        panic(swapchain_err);
    }

    auto [swapchain_frames, frames_err] = create_swapchain_frames(m_device, swapchain, m_command_pool, m_physical_device.surface_format.format);
    if (frames_err) {
        panic(frames_err);
    }

    m_swapchain_frames = std::move(swapchain_frames);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    m_swapchain = swapchain;

    spdlog::info("  swapchain resized to {}x{}", width, height);
}

Result<GPURenderPipeline> GPUContext::create_render_pipeline(GPURenderPipelineDesc desc)
{
    return GPURenderPipeline::create(m_device, desc);
}
}
