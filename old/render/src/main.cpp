#include <helpers.hpp>
#include <print.hpp>
#include <render/vulkan/instance.hpp>

/*
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

int main()
{
    const char* input_file = R"(C:\Users\michal\Desktop\input\in\testYUVFrame0.yuv)";

    AVFormatContext* format_ctx = nullptr;
    const AVInputFormat* format = av_find_input_format("rawvideo");

    AVDictionary* options = nullptr;
//av_dict_set(&options, "framerate", "25", 0);
    av_dict_set(&options, "video_size", "1920x1080", 0);
    av_dict_set(&options, "pixel_format", "yuv420p10le", 0);

    if (avformat_open_input(&format_ctx, input_file, format, &options) < 0) {
        std::cerr << "Could not open input file: " << input_file << std::endl;
        return 2;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        std::cerr << "Could not find stream information" << std::endl;
        return 3;
    }

    // Find video stream
    int video_stream_idx = -1;
    for (unsigned int i = 0; i < format_ctx->nb_streams; ++i) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    if (video_stream_idx == -1) {
        std::cerr << "Could not find video stream" << std::endl;
        return 4;
    }

    // Get codec parameters and codec context
    AVCodecParameters* codecpar = format_ctx->streams[video_stream_idx]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecpar);
    codec_ctx->thread_count = std::thread::hardware_concurrency();

    // Open codec
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        return 5;
    }

    // Allocate frame and packet
    AVFrame* frame = av_frame_alloc();
    AVPacket packet;
    av_init_packet(&packet);

    using milliseconds = chrono::duration<double, std::milli>;

    milliseconds totalTime = milliseconds(0);
    size_t totalFrames = 0;

    // Read frames from the video stream
    while (av_read_frame(format_ctx, &packet) >= 0) {
        if (packet.stream_index == video_stream_idx) {
            int frames = 0;
            const auto start = chrono::high_resolution_clock::now();
            // Decode video frame
            int ret = avcodec_send_packet(codec_ctx, &packet);
            if (ret < 0) {
                std::cerr << "Error sending packet for decoding" << std::endl;
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    std::cerr << "Error during decoding" << std::endl;
                    break;
                }
                ++frames;
            }
            const auto end = chrono::high_resolution_clock::now();

            totalFrames += frames;
            totalTime += chrono::duration_cast<milliseconds>(end - start);
        }

        av_packet_unref(&packet);
    }

    print(cout, "Avg FPS: {}\n", 1000.0/(totalTime / totalFrames).count());
    print(cout, "Total frames: {}\n", totalFrames);

    // Free allocated resources
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    return 0;
}
 */

vector<char> readFile(filesystem::path path)
{
    ifstream file;
    file.open(path, ios::ate | ios::binary);
    assert(file.is_open());

    std::vector<char> buffer(file.tellg());

    file.seekg(0);
    file.read(buffer.data(), buffer.size());

    file.close();

    return buffer;
}



int main()
{
    constexpr uint32_t localSize = 32;
    constexpr size_t length = 14024704;

    const auto context = vk::raii::Context();
    const auto appCreateInfo = vk::ApplicationInfo(
        "Compute", 1, "Compute", 1, VK_API_VERSION_1_3
    );
    const auto enabledLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const auto instance = vk::raii::Instance(
        context,
        vk::InstanceCreateInfo({}, & appCreateInfo, enabledLayers)
    );
    const auto physicalDevices = instance.enumeratePhysicalDevices();
    const auto physicalDevice = physicalDevices[0];

    const auto queuePriorities = {1.0f};
    const auto deviceQueueCreateInfos = {
        vk::DeviceQueueCreateInfo({}, 0, queuePriorities)
    };
    const auto device = physicalDevice.createDevice(
        vk::DeviceCreateInfo({}, deviceQueueCreateInfos)
    );
    const auto queue = device.getQueue(0, 0);

    const auto vmAllocInfo = VmaAllocatorCreateInfo(
        {}, *physicalDevice, *device, {}, {}, {}, {}, {}, *instance,
        VK_API_VERSION_1_3, {}
    );
    VmaAllocator vmAlloc;
    vmaCreateAllocator(&vmAllocInfo, &vmAlloc);

    const auto axpyCode = readFile("shaders/compute/axpy.spv");
    const auto axpyModule = device.createShaderModule(
        vk::ShaderModuleCreateInfo(
            {},
            size(axpyCode),
            reinterpret_cast<const uint32_t*>(axpyCode.data())
            )
    );

    const auto inputDescriptorSetLayoutBindings = {
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute),
        vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute),
        vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute),
    };
    const auto outputDescriptorSetLayoutOutputBindings = {
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute),
    };

    const auto inputDescriptorSetLayoutInfo = vk::DescriptorSetLayoutCreateInfo({}, inputDescriptorSetLayoutBindings);
    const auto outputDescriptorSetLayoutInfo = vk::DescriptorSetLayoutCreateInfo({}, outputDescriptorSetLayoutOutputBindings);

    const auto inputDescriptorSetLayout = device.createDescriptorSetLayout(inputDescriptorSetLayoutInfo);
    const auto outputDescriptorSetLayout = device.createDescriptorSetLayout(outputDescriptorSetLayoutInfo);

    const auto descriptorSetLayouts = {
        *inputDescriptorSetLayout,
        *outputDescriptorSetLayout
    };

    const array specializationMapEntries = {
        vk::SpecializationMapEntry(0, 0, sizeof(uint32_t))
    };
    const array specializationData = {
        localSize
    };
    const auto specializationInfo = vk::SpecializationInfo(
        size(specializationMapEntries),
        data(specializationMapEntries),
        size(specializationData) * sizeof(decltype(specializationData)::value_type),
        data(specializationData)
    );
    const auto computePipelineLayoutStageInfo = vk::PipelineShaderStageCreateInfo(
        {},
        vk::ShaderStageFlagBits::eCompute,
        axpyModule,
        "main",
        &specializationInfo
    );
    const auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo(
        {}, descriptorSetLayouts
    );

    const auto computePipelineLayout = device.createPipelineLayout(
        pipelineLayoutInfo
    );

    const auto computePipeline = device.createComputePipeline(
        nullptr,
        vk::ComputePipelineCreateInfo({}, computePipelineLayoutStageInfo, computePipelineLayout)
    );

    const auto queueFamilyIndices = {0u};
    const auto bufferGpuInputInfo = static_cast<VkBufferCreateInfo>(vk::BufferCreateInfo(
        {}, length * sizeof(float), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive, queueFamilyIndices
    ));
    const auto bufferStagingInputInfo = static_cast<VkBufferCreateInfo>(vk::BufferCreateInfo(
        {}, length * sizeof(float), vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive, queueFamilyIndices
    ));
    const auto bufferGpuOutputInfo = static_cast<VkBufferCreateInfo>(vk::BufferCreateInfo(
        {}, length * sizeof(float), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive, queueFamilyIndices
    ));
    const auto bufferStagingOutputInfo = static_cast<VkBufferCreateInfo>(vk::BufferCreateInfo(
        {}, length * sizeof(float), vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive, queueFamilyIndices
    ));
    const auto allocationInfoGpu = VmaAllocationCreateInfo(
        {},
        VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eDeviceLocal),
        static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eDeviceLocal),
        {}, {}, {}, {}
    );
    const auto allocationInfoStaging = VmaAllocationCreateInfo(
        {},
        VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
        static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
        {}, {}, {}, {}
    );

    VkBuffer tmpBufferHandle;

    VkResult result;
    VmaAllocation inputBufferAAllocation;
    result = vmaCreateBuffer((VmaAllocator) vmAlloc, &bufferGpuInputInfo, &allocationInfoGpu, &tmpBufferHandle, &inputBufferAAllocation, {});
    const auto inputBufferA = vk::Buffer(tmpBufferHandle);
    VmaAllocation inputBufferBAllocation;
    result = vmaCreateBuffer((VmaAllocator) vmAlloc, &bufferGpuInputInfo, &allocationInfoGpu, &tmpBufferHandle, &inputBufferBAllocation, {});
    const auto inputBufferB = vk::Buffer(tmpBufferHandle);
    VmaAllocation inputBufferCAllocation;
    result = vmaCreateBuffer((VmaAllocator) vmAlloc, &bufferGpuInputInfo, &allocationInfoGpu, &tmpBufferHandle, &inputBufferCAllocation, {});
    const auto inputBufferC = vk::Buffer(tmpBufferHandle);
    VmaAllocation outputBufferAllocation;
    result = vmaCreateBuffer((VmaAllocator) vmAlloc, &bufferGpuOutputInfo, &allocationInfoGpu, &tmpBufferHandle, &outputBufferAllocation, {});
    const auto outputBuffer = vk::Buffer(tmpBufferHandle);

    VmaAllocation inputBufferAStagingAllocation;
    VmaAllocationInfo inputBufferAStagingAllocationInfo;
    result = vmaCreateBuffer((VmaAllocator) vmAlloc, &bufferStagingInputInfo, &allocationInfoStaging, &tmpBufferHandle, &inputBufferAStagingAllocation, &inputBufferAStagingAllocationInfo);
    const auto inputBufferAStaging = vk::Buffer(tmpBufferHandle);
    VmaAllocation inputBufferBStagingAllocation;
    VmaAllocationInfo inputBufferBStagingAllocationInfo;
    result = vmaCreateBuffer((VmaAllocator) vmAlloc, &bufferStagingInputInfo, &allocationInfoStaging, &tmpBufferHandle, &inputBufferBStagingAllocation, &inputBufferBStagingAllocationInfo);
    const auto inputBufferBStaging = vk::Buffer(tmpBufferHandle);
    VmaAllocation inputBufferCStagingAllocation;
    VmaAllocationInfo inputBufferCStagingAllocationInfo;
    result = vmaCreateBuffer((VmaAllocator) vmAlloc, &bufferStagingInputInfo, &allocationInfoStaging, &tmpBufferHandle, &inputBufferCStagingAllocation, &inputBufferCStagingAllocationInfo);
    const auto inputBufferCStaging = vk::Buffer(tmpBufferHandle);
    VmaAllocation outputBufferStagingAllocation;
    VmaAllocationInfo outputBufferStagingAllocationInfo;
    result = vmaCreateBuffer((VmaAllocator) vmAlloc, &bufferStagingOutputInfo, &allocationInfoStaging, &tmpBufferHandle, &outputBufferStagingAllocation, &outputBufferStagingAllocationInfo);
    const auto outputBufferStaging = vk::Buffer(tmpBufferHandle);

    const auto commandPool = device.createCommandPool(vk::CommandPoolCreateInfo({}, 0));
    const auto commandBuffers = device.allocateCommandBuffers(
        vk::CommandBufferAllocateInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 4)
    );

    const auto& commandBuffer = commandBuffers[0];
    const auto& commandBufferIn = commandBuffers[1];
    const auto& commandBufferCompute = commandBuffers[2];
    const auto& commandBufferOut = commandBuffers[3];

    const auto descriptorPoolSizes = {
        vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 4)
    };
    const auto descriptorPool = device.createDescriptorPool(
        vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 2, descriptorPoolSizes)
    );
    const auto descriptorSets = device.allocateDescriptorSets(
        vk::DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayouts)
    );

    const auto inputABufferWrites = {
        vk::DescriptorBufferInfo(inputBufferA, 0, vk::DeviceSize(length * sizeof(float)))
    };
    const auto inputADescriptorSetWrite = vk::WriteDescriptorSet(
        descriptorSets[0], 0, 0, vk::DescriptorType::eStorageBuffer, {}, inputABufferWrites, {}
    );

    const auto inputBBufferWrites = {
        vk::DescriptorBufferInfo(inputBufferB, 0, vk::DeviceSize(length * sizeof(float)))
    };
    const auto inputBDescriptorSetWrite = vk::WriteDescriptorSet(
        descriptorSets[0], 1, 0, vk::DescriptorType::eStorageBuffer, {}, inputBBufferWrites, {}
    );

    const auto inputCBufferWrites = {
        vk::DescriptorBufferInfo(inputBufferC, 0, vk::DeviceSize(length * sizeof(float)))
    };
    const auto inputCDescriptorSetWrite = vk::WriteDescriptorSet(
        descriptorSets[0], 2, 0, vk::DescriptorType::eStorageBuffer, {}, inputCBufferWrites, {}
    );

    const auto outputBufferWrites = {
        vk::DescriptorBufferInfo(inputBufferC, 0, vk::DeviceSize(length * sizeof(float)))
    };
    const auto outputDescriptorSetWrite = vk::WriteDescriptorSet(
        descriptorSets[1], 0, 0, vk::DescriptorType::eStorageBuffer, {}, outputBufferWrites, {}
    );

    const auto descriptorWrites = {
        inputADescriptorSetWrite,
        inputBDescriptorSetWrite,
        inputCDescriptorSetWrite,
        outputDescriptorSetWrite,
    };

    device.updateDescriptorSets(descriptorWrites, {});

    const auto computeInFlight = device.createFence(vk::FenceCreateInfo());

    void* inputBufferAPtr = (*device).mapMemory(inputBufferAStagingAllocationInfo.deviceMemory, 0, length * sizeof(float));
    void* inputBufferBPtr = (*device).mapMemory(inputBufferBStagingAllocationInfo.deviceMemory, 0, length * sizeof(float));
    void* inputBufferCPtr = (*device).mapMemory(inputBufferCStagingAllocationInfo.deviceMemory, 0, length * sizeof(float));
    void* outputBufferPtr = (*device).mapMemory(outputBufferStagingAllocationInfo.deviceMemory, 0, length * sizeof(float));

    const auto regions = {
        vk::BufferCopy(0, 0, length * sizeof(float))
    };
    const auto descriptorSetHandles = {
        *descriptorSets[0],
        *descriptorSets[1],
    };

    const auto inputAStagingToGpuBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferRead,
        0, 0, inputBufferAStaging, 0, length * sizeof(float)
    );
    const auto inputBStagingToGpuBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferRead,
        0, 0, inputBufferBStaging, 0, length * sizeof(float)
    );
    const auto inputCStagingToGpuBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferRead,
        0, 0, inputBufferCStaging, 0, length * sizeof(float)
    );
    const auto inputAGpuPrepareBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite,
        0, 0, inputBufferA, 0, length * sizeof(float)
    );
    const auto inputBGpuPrepareBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite,
        0, 0, inputBufferB, 0, length * sizeof(float)
    );
    const auto inputCGpuPrepareBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite,
        0, 0, inputBufferC, 0, length * sizeof(float)
    );
    const auto stagingToGpuBarriers = {
        inputAStagingToGpuBarrier,
        inputBStagingToGpuBarrier,
        inputCStagingToGpuBarrier,
        inputAGpuPrepareBarrier,
        inputBGpuPrepareBarrier,
        inputCGpuPrepareBarrier,
    };

    const auto inputAGpuToComputeBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
        0, 0, inputBufferA, 0, length * sizeof(float)
    );
    const auto inputBGpuToComputeBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
        0, 0, inputBufferB, 0, length * sizeof(float)
    );
    const auto inputCGpuToComputeBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
        0, 0, inputBufferC, 0, length * sizeof(float)
    );
    const auto outputGpuToComputeBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eNone, vk::AccessFlagBits::eShaderWrite,
        0, 0, outputBuffer, 0, length * sizeof(float)
    );
    const auto gpuToShaderBarriers = {
        inputAGpuToComputeBarrier,
        inputBGpuToComputeBarrier,
        inputCGpuToComputeBarrier,
        outputGpuToComputeBarrier,
    };

    const auto outputComputeToGpuBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eTransferRead,
        0, 0, outputBuffer, 0, length * sizeof(float)
    );
    const auto outputStagingComputeToGpuBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite,
        0, 0, outputBufferStaging, 0, length * sizeof(float)
    );
    const auto computeToGpuBarriers = {
        outputComputeToGpuBarrier,
        outputStagingComputeToGpuBarrier,
    };

    const auto outputStagingGpuToReadBarrier = vk::BufferMemoryBarrier(
        vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eHostRead,
        0, 0, outputBufferStaging, 0, length * sizeof(float)
    );
    const auto gpuToReadBarriers = {
        outputStagingGpuToReadBarrier,
    };

    commandBuffer.begin(vk::CommandBufferBeginInfo());

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, computePipelineLayout, 0, descriptorSetHandles, {});

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer,
        {}, {}, stagingToGpuBarriers, {}
    );
    commandBuffer.copyBuffer(inputBufferAStaging, inputBufferA, regions);
    commandBuffer.copyBuffer(inputBufferBStaging, inputBufferB, regions);
    commandBuffer.copyBuffer(inputBufferCStaging, inputBufferC, regions);
    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader,
        {}, {}, gpuToShaderBarriers, {}
    );
    commandBuffer.dispatch(length / localSize, 1, 1);
    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eTransfer,
        {}, {}, computeToGpuBarriers, {}
    );
    commandBuffer.copyBuffer(outputBuffer, outputBufferStaging, regions);
    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eHost,
        {}, {}, gpuToReadBarriers, {}
    );
    commandBuffer.end();

    commandBufferIn.begin(vk::CommandBufferBeginInfo());
    commandBufferIn.copyBuffer(inputBufferAStaging, inputBufferA, regions);
    commandBufferIn.copyBuffer(inputBufferBStaging, inputBufferB, regions);
    commandBufferIn.copyBuffer(inputBufferCStaging, inputBufferC, regions);
    commandBufferIn.end();

    commandBufferCompute.begin(vk::CommandBufferBeginInfo());
    commandBufferCompute.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline);
    commandBufferCompute.bindDescriptorSets(vk::PipelineBindPoint::eCompute, computePipelineLayout, 0, descriptorSetHandles, {});
    commandBufferCompute.dispatch(length / localSize, 1, 1);
    commandBufferCompute.end();

    commandBufferOut.begin(vk::CommandBufferBeginInfo());
    commandBufferOut.copyBuffer(outputBuffer, outputBufferStaging, regions);
    commandBufferOut.end();

    using milliseconds = chrono::duration<double, milli>;

    std::vector<float> inputAData(length, 0.0f);
    std::vector<float> inputBData(length, 0.0f);
    std::vector<float> inputCData(length, 0.0f);
    std::vector<float> outputData(length, 0.0f);
    random_device rd{};
    mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0f, 10.0f);
    for(size_t i=0;i<length;i++)
    {
        inputAData[i] = dis(gen);
        inputBData[i] = dis(gen);
        inputCData[i] = dis(gen);
    }

    const auto start = chrono::high_resolution_clock::now();

    memcpy(inputBufferAPtr, inputAData.data(), length * sizeof(float));
    memcpy(inputBufferBPtr, inputBData.data(), length * sizeof(float));
    memcpy(inputBufferCPtr, inputCData.data(), length * sizeof(float));

    /*const auto submitCommandBuffers = { *commandBuffer };
    const auto computingFences = {*computeInFlight};
    const auto submits = {
        vk::SubmitInfo({}, {}, submitCommandBuffers, {})
    };
    device.resetFences(computingFences);
    queue.submit(submits, *computeInFlight);
    device.waitForFences(computingFences, vk::True, std::numeric_limits<uint64_t>::max());*/

    const auto inCommandBuffers = { *commandBufferIn };
    const auto inSubmits = { vk::SubmitInfo({}, {}, inCommandBuffers, {}) };
    queue.submit(inSubmits, {});
    device.waitIdle();

    const auto computeCommandBuffers = { *commandBufferCompute };
    const auto computeSubmits = { vk::SubmitInfo({}, {}, computeCommandBuffers, {}) };
    queue.submit(computeSubmits, {});
    device.waitIdle();

    const auto outCommandBuffers = { *commandBufferOut };
    const auto outSubmits = { vk::SubmitInfo({}, {}, outCommandBuffers, {}) };
    queue.submit(outSubmits, {});
    device.waitIdle();

    memcpy(outputData.data(), outputBufferPtr, length * sizeof(float));

    const auto end = chrono::high_resolution_clock::now();

    cout << format("Time taken: {}ms", chrono::duration_cast<milliseconds>(end - start).count());

    for(size_t i=0;i<length;i++)
    {
        float reference = inputAData[i] * inputBData[i] + inputCData[i];
        float error = abs(reference - outputData[i]);
        if(error > numeric_limits<float>::epsilon())
        {
            print(cout, "Discrepancy at {}: result={}, reference={}, error={}\n", i,  outputData[i], reference, error);
            break;
        }
    }

    vmaDestroyBuffer(vmAlloc, inputBufferA, inputBufferAAllocation);
    vmaDestroyBuffer(vmAlloc, inputBufferB, inputBufferBAllocation);
    vmaDestroyBuffer(vmAlloc, inputBufferC, inputBufferCAllocation);
    vmaDestroyBuffer(vmAlloc, outputBuffer, outputBufferAllocation);
    vmaDestroyBuffer(vmAlloc, inputBufferAStaging, inputBufferAStagingAllocation);
    vmaDestroyBuffer(vmAlloc, inputBufferBStaging, inputBufferBStagingAllocation);
    vmaDestroyBuffer(vmAlloc, inputBufferCStaging, inputBufferCStagingAllocation);
    vmaDestroyBuffer(vmAlloc, outputBufferStaging, outputBufferStagingAllocation);
    vmaDestroyAllocator(vmAlloc);

    return 0;
}