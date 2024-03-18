#include <helpers.hpp>
#include <print.hpp>

constexpr auto requiredInstanceExtensions = {
        "VK_KHR_surface",
        "VK_KHR_get_physical_device_properties2",
        "VK_KHR_get_surface_capabilities2",
        "VK_KHR_external_fence_capabilities",
        "VK_KHR_external_memory_capabilities",
        "VK_KHR_external_semaphore_capabilities",
};

constexpr auto requiredDeviceExtensions = {
    "VK_KHR_spirv_1_4",
    "VK_KHR_vulkan_memory_model",

    "VK_KHR_8bit_storage",
    "VK_KHR_16bit_storage",
    "VK_EXT_4444_formats",

    "VK_EXT_memory_budget",
    "VK_EXT_memory_priority",

    "VK_EXT_multi_draw",

    "VK_EXT_nested_command_buffer",

    "VK_KHR_external_fence",
    "VK_KHR_external_memory",
    "VK_KHR_external_semaphore",

    "VK_KHR_maintenance1",
    "VK_KHR_maintenance2",
    "VK_KHR_maintenance3",
    "VK_KHR_maintenance4",
    "VK_KHR_maintenance5",
    "VK_KHR_maintenance6",

    "VK_KHR_video_decode_h264",
    "VK_KHR_video_decode_h265",
    "VK_KHR_video_decode_queue",

    "VK_KHR_video_encode_h264",
    "VK_KHR_video_encode_h265",
    "VK_KHR_video_encode_queue",

    "VK_KHR_video_maintenance1",
    "VK_KHR_video_queue",

    "VK_KHR_swapchain",

    "VK_KHR_synchronization2",

    "VK_KHR_present_wait",

    "VK_KHR_sampler_ycbcr_conversion",
    "VK_EXT_ycbcr_2plane_444_formats",
    "VK_EXT_ycbcr_image_arrays",

    "VK_KHR_shader_clock",
};

template<typename Dispatch>
bool checkPhysicalDeviceExtensions(const vk::PhysicalDevice& physicalDevice, const initializer_list<const char*>& requiredExtensions, const Dispatch& dispatchLoader)
{
    const auto deviceProperties = physicalDevice.getProperties(dispatchLoader);

    std::vector<string_view> missingExtensions{};

    const auto extensions = physicalDevice.enumerateDeviceExtensionProperties(nullptr_t{}, dispatchLoader);
    std::set<string_view> deviceExtensions{};
    for(const auto& extension : extensions)
    {
        deviceExtensions.insert(extension.extensionName.data());
    }

    cout << "Checking device: " << deviceProperties.deviceName.data() << endl;
    for(const auto& requiredExtension : requiredExtensions)
    {
        if(!deviceExtensions.contains(string_view{requiredExtension}))
            missingExtensions.emplace_back(requiredExtension);
    }

    if(missingExtensions.empty())
    {
        cout << "Checking device: " << deviceProperties.deviceName.data() << " - OK" << endl;
    }
    else
    {
        cout << "Checking device: " << deviceProperties.deviceName.data() << " - MISSING EXTENSIONS:" << endl;
        for(const auto missingExtension: missingExtensions)
        {
            cout << "\t- " << missingExtension << endl;
        }
    }

    return missingExtensions.empty();
}

struct queueFamilyIndices_t
{
public:
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
    uint32_t videoDecode;
    uint32_t videoEncode;
};

template<typename Dispatch = vk::DispatchLoaderStatic>
optional<queueFamilyIndices_t> findPhysicalDeviceQueueFamilies(const vk::PhysicalDevice& physicalDevice, const Dispatch& dispatchLoader)
{
    const auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    optional<uint32_t> graphics{};
    optional<uint32_t> compute{};
    optional<uint32_t> transfer{};
    optional<uint32_t> videoDecode{};
    optional<uint32_t> videoEncode{};

    uint32_t index = 0;
    for(const auto& queueFamilyProperty : queueFamilyProperties)
    {
        //cout << format("QueueFamily:") << endl;
        //cout << format("\t- count: {}", queueFamilyProperty.queueCount) << endl;
        //cout << format("\t- flags: {}", queueFamilyProperty.queueFlags) << endl;

        if(queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics && !graphics)
            graphics = index;

        if(queueFamilyProperty.queueFlags & vk::QueueFlagBits::eCompute && !compute)
            compute = index;

        if(queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer && !transfer)
            transfer = index;

        if(queueFamilyProperty.queueFlags & vk::QueueFlagBits::eVideoDecodeKHR && !videoDecode)
            videoDecode = index;

        if(queueFamilyProperty.queueFlags & vk::QueueFlagBits::eVideoEncodeKHR && !videoEncode)
            videoEncode = index;

        index++;
    }

    if(graphics && compute && transfer && videoDecode && videoEncode)
        return queueFamilyIndices_t
        {
            .graphics = *graphics,
            .compute = *compute,
            .transfer = *transfer,
            .videoDecode = *videoDecode,
            .videoEncode = *videoEncode,
        };
    else
        return nullopt;
}

template<typename Dispatch>
optional<tuple<vk::PhysicalDevice, queueFamilyIndices_t>> findSuitablePhysicalDevice(const vk::Instance& instance, const initializer_list<const char*>& requiredExtensions, const Dispatch& dispatchLoader)
{
    const auto physicalDevices = instance.enumeratePhysicalDevices(dispatchLoader);

    for(const auto& physicalDevice : physicalDevices)
    {
        if(!checkPhysicalDeviceExtensions(physicalDevice, requiredExtensions, dispatchLoader))
            continue;

        const auto physicalDeviceProperties = physicalDevice.getProperties(dispatchLoader);

        if(physicalDeviceProperties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
            continue;

        const auto queueFamilyIndices = findPhysicalDeviceQueueFamilies(physicalDevice, dispatchLoader);

        if(queueFamilyIndices)
            return make_tuple(physicalDevice, *queueFamilyIndices);
    }

    return nullopt;
}

int main()
{
    const auto dispatchLoader = vk::DispatchLoaderStatic();
    const auto allocationCallbacks = vk::createDefaultAllocationCallbacks();

    printInstanceExtensions(dispatchLoader);
    printLayersWithInstanceExtensions(dispatchLoader);

    const auto applicationInfo = vk::ApplicationInfo(
        "Test",
        VK_MAKE_VERSION(1, 0, 0),
        "render",
        VK_MAKE_VERSION(0, 0, 1),
        VK_API_VERSION_1_3
    );

    const auto instanceCreateInfo = vk::InstanceCreateInfo(
        {},
        &applicationInfo,
        {},
        requiredInstanceExtensions
    );

    const auto instance = vk::createInstance(instanceCreateInfo, allocationCallbacks, dispatchLoader);

    printAllPhysicalDevicesWithExtensionsAndProperties(instance, dispatchLoader);

    {
        cout << "Selecting device..." << endl;
        const auto physicalDeviceAndQueueFamilyIndices = findSuitablePhysicalDevice(instance, requiredDeviceExtensions, dispatchLoader);

        if(!physicalDeviceAndQueueFamilyIndices)
        {
            throw runtime_error("Couldn't find device with required extensions!");
        }
        else{
            const auto& [physicalDevice, queueFamilyIndices] = *physicalDeviceAndQueueFamilyIndices;

            const auto physicalDeviceProperties = physicalDevice.getProperties(dispatchLoader);
            cout << "Selected device = " << physicalDeviceProperties.deviceName.data() << endl;

            const float priority = 1.0f;
            const auto deviceQueueCreateInfo = vk::DeviceQueueCreateInfo(
                {},
                queueFamilyIndices.graphics,
                1,
                &priority
            );

            const auto usedPhysicalDeviceFeatures = vk::PhysicalDeviceFeatures();

            const auto deviceCreateInfo = vk::DeviceCreateInfo(
                {},
                1,
                &deviceQueueCreateInfo,
                0,
                nullptr,
                size(requiredDeviceExtensions),
                data(requiredDeviceExtensions),
                &usedPhysicalDeviceFeatures
            );

            const auto logicalDevice = physicalDevice.createDevice(deviceCreateInfo, allocationCallbacks, dispatchLoader);


            

            logicalDevice.destroy(allocationCallbacks, dispatchLoader);
        }
    }
    instance.destroy(allocationCallbacks, dispatchLoader);

    return 0;
}