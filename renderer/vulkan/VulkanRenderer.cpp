#include "VulkanRenderer.hpp"
#include "GLFW/glfw3.h"
#include "VkMesh.hpp"

using namespace renderer;

VulkanRenderer::VulkanRenderer() {
    INFO("Use Vulkan Renderer.");
    ResetProp();
}

VulkanRenderer::~VulkanRenderer() {
    ResetProp();
}

void VulkanRenderer::ResetProp() {
    // Fundation
    _Window = nullptr;
    _VkInstance = nullptr;
    _VkPhyDevice = nullptr;
    _SurfaceKHR = nullptr;
    _VkDevice = nullptr;
    _SwapchainKHR = nullptr;

    // Graphics
    _VkRenderPass = nullptr;
    _Pipeline = nullptr;
    _DepthImageView = nullptr;
    _FrameNumber = 0;
    _DescriptorPool = nullptr;
    _GlobalSetLayout = nullptr;
    _DrawLinePipeline = nullptr;

    // Compute
    _ComputePipeline = nullptr;
    _ComputePipelineLayout = nullptr;
    _ComputeSetLayout = nullptr;

    // Properties
    _bEnabledTexture = false;
    _bEnabledSampleShading = true;
}

bool VulkanRenderer::Init() { 
    INFO("Init Renderer.");

    // Fundation
    CreateInstance();
    PickupPhyDevice();
    CreateSurface();
    CreateDevice();
    InitQueue();
    CreateSwapchain();
    GetVkImages();
    GetVkImageViews();
    CreateRenderPass();
    CreateCmdPool();
    AllocateFrameCmdBuffer();
    CreateDepthImage();
    CreateFrameBuffers();
    InitSyncStructures();
    InitDescriptors();

    // Compute Pipeline
    InitComputeDescriptors();

    return true; 
}

void VulkanRenderer::Release(){
    INFO("Release Renderer");

    // compute
    _VkDevice.destroyDescriptorSetLayout(_ComputeSetLayout);
    _VkDevice.destroyPipelineLayout(_ComputePipelineLayout);
    _VkDevice.destroyPipeline(_ComputePipeline);

    // graphics
    _VkDevice.destroySampler(_TextureSampler);
    _VkDevice.destroyDescriptorSetLayout(_TextureSetLayout);
    _VkDevice.destroyFence(_UploadContext.uploadFence);
    _VkDevice.destroyCommandPool(_UploadContext.commandPool);
    _VkDevice.freeMemory(_SceneParameterBuffer.memory);
    _VkDevice.destroyBuffer(_SceneParameterBuffer.buffer);
    _VkDevice.destroyDescriptorPool(_DescriptorPool);
    _VkDevice.destroyDescriptorSetLayout(_GlobalSetLayout);
    _VkDevice.destroyImageView(_DepthImageView);
    _VkDevice.destroyImage(_DepthImage.image);
    _VkDevice.free(_DepthImage.memory);

    for (auto& frame : _Frames){
        _VkDevice.freeMemory(frame.cameraBuffer.memory);
        _VkDevice.destroyBuffer(frame.cameraBuffer.buffer);
        _VkDevice.destroyFence(frame.renderFence);
        _VkDevice.destroyFence(frame.computeFence);
        _VkDevice.destroySemaphore(frame.renderSemaphore);
        _VkDevice.destroySemaphore(frame.computeSemaphore);
        _VkDevice.destroySemaphore(frame.presentSemaphore);
        _VkDevice.destroyCommandPool(frame.commandPool);
    }
    
    _VkDevice.destroyRenderPass(_VkRenderPass);

    for (auto& frameBuffer : _FrameBuffers) { _VkDevice.destroyFramebuffer(frameBuffer); }
    for (auto& view : _ImageViews) { _VkDevice.destroyImageView(view); }
    
    _VkDevice.destroySwapchainKHR(_SwapchainKHR);
    _VkDevice.destroy();
    _VkInstance.destroySurfaceKHR(_SurfaceKHR);
    _VkInstance.destroy();
}


void VulkanRenderer::InitWindow(){
    _Window =  WsiWindow::GetInstance()->GetWindow();
    if (!_Window) {
        DEBUG("Error Window.");
        exit(-1);
    }
}

void VulkanRenderer::CreateInstance() {
    InitWindow();

    // Get GLFW instance extensions
    unsigned int extensionsCount;
    const char** extensionNames;
    extensionNames = glfwGetRequiredInstanceExtensions(&extensionsCount);

#ifdef __APPLE__
    // MacOS requirment
    extensionNames[extensionsCount] = "VK_KHR_get_physical_device_properties2";
    ++extensionsCount;
#endif


    std::vector<VkLayerProperties> enumLayers;
    uint32_t enumLayerCount;
    vkEnumerateInstanceLayerProperties(&enumLayerCount, enumLayers.data());
    enumLayers.resize(enumLayerCount);
    vkEnumerateInstanceLayerProperties(&enumLayerCount, enumLayers.data());

    //validation layers
    std::vector<const char*> layers;
    for (auto& enumLayer : enumLayers) {
        if (strcmp(enumLayer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
            layers.push_back("VK_LAYER_KHRONOS_validation");
        }
    }

    vk::InstanceCreateInfo info;
    info.setPpEnabledExtensionNames(extensionNames)
        .setPEnabledLayerNames(layers)
        .setEnabledExtensionCount(extensionsCount)
        .setEnabledLayerCount((uint32_t)layers.size());

#ifdef LEVEL_DEBUG
    std::vector<VkExtensionProperties> enumExtentions;
    uint32_t enumExtentionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &enumExtentionCount, nullptr);
    enumExtentions.resize(enumExtentionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &enumExtentionCount, enumExtentions.data());
    std::cout << "All Extentions:\n";
    for (auto& extention : enumExtentions) std::cout << extention.extensionName << std::endl;

    std::cout << "All Layers:\n";
    for (auto& layer : enumLayers) std::cout << layer.layerName << std::endl;

    std::cout << "Added Extensions:\n";
    for (unsigned int i = 0; i < extensionsCount; i++) std::cout << extensionNames[i] << std::endl;
    std::cout << "Added Layers:\n";
    for (auto& layer : layers) std::cout << layer << std::endl;
#endif

    _VkInstance = vk::createInstance(info);
}

void VulkanRenderer::PickupPhyDevice() {
   
    std::vector<vk::PhysicalDevice> physicalDevices = _VkInstance.enumeratePhysicalDevices();
    _VkPhyDevice = physicalDevices[0];

    _MaxSampleCount = GetMaxUsableSampleCount();

#ifdef LEVEL_DEBUG
    std::cout << "\nUsing GPU:  " << physicalDevices[0].getProperties().deviceName << std::endl;    //输出显卡名称
    
    vk::PhysicalDeviceFeatures features = physicalDevices[0].getFeatures();
    const char* enableWideLine = features.wideLines ? "true" : "false";
    std::cout << "Enable Wide line: " << enableWideLine << std::endl;
#endif
}

void VulkanRenderer::CreateSurface(){
    ASSERT(_Window);
    ASSERT(_VkInstance);

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(_VkInstance, _Window, nullptr, &surface) != VK_SUCCESS){
        INFO("Create surface failed.");
        return;
    }

    _SurfaceKHR = vk::SurfaceKHR(surface);
    ASSERT(_SurfaceKHR);
}

void VulkanRenderer::CreateDevice(){
    if (!QueryQueueFamilyProp()){
        INFO("Query Queue Family properties failed.");
        return;
    }

    std::vector<vk::DeviceQueueCreateInfo> dqInfo;
    // Only one instance when graphicsIndex sames as presentIndex
    if (_QueueFamilyProp.graphicsIndex.value() == _QueueFamilyProp.presentIndex.value()){
        vk::DeviceQueueCreateInfo info;
        float priority = 1.0;
        info.setQueuePriorities(priority);
        info.setQueueCount(1);
        info.setQueueFamilyIndex(_QueueFamilyProp.graphicsIndex.value());    
        dqInfo.push_back(info);
    } else {
        vk::DeviceQueueCreateInfo info1;
        float priority = 1.0;
        info1.setQueuePriorities(priority); 
        info1.setQueueCount(1);
        info1.setQueueFamilyIndex(_QueueFamilyProp.graphicsIndex.value());

        vk::DeviceQueueCreateInfo info2;
        info2.setQueuePriorities(priority);
        info2.setQueueCount(1);
        info2.setQueueFamilyIndex(_QueueFamilyProp.presentIndex.value());

        dqInfo.push_back(info1);       
        dqInfo.push_back(info2);        
    }

    vk::DeviceCreateInfo dInfo;
    dInfo.setQueueCreateInfos(dqInfo);

#ifdef __APPLE__
    std::array<const char*,2> extensions{"VK_KHR_portability_subset",
                                          VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#elif _WIN32
    std::array<const char*, 1> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif
    dInfo.setPEnabledExtensionNames(extensions);

    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures = _VkPhyDevice.getFeatures();
    dInfo.setPEnabledFeatures(&deviceFeatures);

    _VkDevice = _VkPhyDevice.createDevice(dInfo);
    ASSERT(_VkDevice);
}

void VulkanRenderer::CreateSwapchain() {
    _SupportInfo.QuerySupportInfo(_VkPhyDevice, _SurfaceKHR);

    vk::SwapchainCreateInfoKHR scInfo;
    scInfo.setImageColorSpace(_SupportInfo.format.colorSpace);
    scInfo.setImageFormat(_SupportInfo.format.format);
    scInfo.setImageExtent(_SupportInfo.extent);
    scInfo.setMinImageCount(_SupportInfo.imageCount);
    scInfo.setPresentMode(_SupportInfo.presnetMode);
    scInfo.setPreTransform(_SupportInfo.capabilities.currentTransform);

    if (_QueueFamilyProp.graphicsIndex.value() == _QueueFamilyProp.presentIndex.value()) {
        scInfo.setQueueFamilyIndices(_QueueFamilyProp.graphicsIndex.value());
        scInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }
    else {
        std::array<uint32_t, 2> index{ _QueueFamilyProp.graphicsIndex.value(),
                                     _QueueFamilyProp.presentIndex.value() };
        scInfo.setQueueFamilyIndices(index);
        scInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
    }

    scInfo.setClipped(true);
    scInfo.setSurface(_SurfaceKHR);
    scInfo.setImageArrayLayers(1);
    scInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    scInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    _SwapchainKHR = _VkDevice.createSwapchainKHR(scInfo);
    ASSERT(_SwapchainKHR);
}

void VulkanRenderer::CreateRenderPass() {
    ASSERT(_VkDevice);
    ASSERT(_VkPhyDevice);
    
    //颜色附件
    vk::AttachmentDescription attchmentDesc;
    attchmentDesc.setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setFormat(_SupportInfo.format.format)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colRefer;
    colRefer.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    colRefer.setAttachment(0);

    //深度附件
    vk::AttachmentDescription depthAttchDesc;
    vk::Format depthFormat = FindSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eX8D24UnormPack32 },
        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    depthAttchDesc.setFormat(depthFormat)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal);
    vk::AttachmentReference depthRef;
    depthRef.setAttachment(1)
            .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
 
    vk::SubpassDescription subpassDesc;
    subpassDesc.setColorAttachmentCount(1);
    subpassDesc.setPColorAttachments(&colRefer);
    subpassDesc.setPDepthStencilAttachment(&depthRef);
    subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

    std::array<vk::AttachmentDescription, 2> attachments = {attchmentDesc, depthAttchDesc};
    
    vk::SubpassDependency dependency;
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    dependency.setSrcAccessMask(vk::AccessFlagBits::eNone);
    dependency.setDstSubpass(0);
    dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
    dependency.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

    vk::SubpassDependency depthDependency;
    depthDependency.setSrcStageMask(
        vk::PipelineStageFlagBits::eEarlyFragmentTests | 
        vk::PipelineStageFlagBits::eLateFragmentTests);
    depthDependency.setSrcAccessMask(vk::AccessFlagBits::eNone);
    depthDependency.setDstSubpass(0);
    depthDependency.setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests |
        vk::PipelineStageFlagBits::eLateFragmentTests);
    depthDependency.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
    depthDependency.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

    std::vector<vk::SubpassDependency> dependecies = { dependency, depthDependency };

    vk::RenderPassCreateInfo info;
    info.setSubpassCount(1)
        .setSubpasses(subpassDesc)
        .setAttachmentCount((uint32_t)attachments.size())
        .setAttachments(attachments)
        .setDependencies(dependecies)
        .setDependencyCount((uint32_t)dependecies.size());

    _VkRenderPass = _VkDevice.createRenderPass(info);
    ASSERT(_VkRenderPass);
}

void VulkanRenderer::CreateCmdPool() {
    ASSERT(_VkDevice);

    vk::CommandPoolCreateInfo info;
    info.setQueueFamilyIndex(_QueueFamilyProp.graphicsIndex.value())
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    for (int i = 0; i < FRAME_OVERLAP; ++i) {
        _Frames[i].commandPool = _VkDevice.createCommandPool(info);
        ASSERT(_Frames[i].commandPool);
    }

    _UploadContext.commandPool = _VkDevice.createCommandPool(info);
    ASSERT(_UploadContext.commandPool);
}

void VulkanRenderer::AllocateFrameCmdBuffer() {
    for (int i = 0; i < FRAME_OVERLAP; ++i) {
        ASSERT(_Frames[i].commandPool);
        vk::CommandBuffer& grapCmdBuffer = _Frames[i].mainCommandBuffer;
        vk::CommandBuffer& compCmdBuffer = _Frames[i].computeCommandBuffer;

        vk::CommandBufferAllocateInfo allocte;
        allocte.setCommandPool(_Frames[i].commandPool)
            .setCommandBufferCount(1)
            .setLevel(vk::CommandBufferLevel::ePrimary);

        grapCmdBuffer = _VkDevice.allocateCommandBuffers(allocte)[0];
        ASSERT(grapCmdBuffer);
        compCmdBuffer = _VkDevice.allocateCommandBuffers(allocte)[0];
        ASSERT(compCmdBuffer);
    }

    vk::CommandBufferAllocateInfo uploadAlloc;
    uploadAlloc.setCommandPool(_UploadContext.commandPool)
            .setCommandBufferCount(1)
            .setLevel(vk::CommandBufferLevel::ePrimary);
    _UploadContext.commandBuffer = _VkDevice.allocateCommandBuffers(uploadAlloc)[0];
}

vk::CommandBuffer VulkanRenderer::AllocateCmdBuffer() {
    ASSERT(_VkDevice);
    ASSERT(GetCurrentFrame().commandPool);
    if (!_VkDevice) {
        return nullptr;
    }

    vk::CommandBufferAllocateInfo allocte;
    allocte.setCommandPool(GetCurrentFrame().commandPool)
        .setCommandBufferCount(1)
        .setLevel(vk::CommandBufferLevel::ePrimary);

    vk::CommandBuffer CmdBuffer;
    CmdBuffer = _VkDevice.allocateCommandBuffers(allocte)[0];

    vk::CommandBufferBeginInfo info;
    info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    CmdBuffer.begin(info);

    return CmdBuffer;
}

void VulkanRenderer::EndCmdBuffer(vk::CommandBuffer cmdBuf) {
    ASSERT(cmdBuf)
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(cmdBuf);

    _Queue.GraphicsQueue.submit(submitInfo);
    _VkDevice.waitIdle();
    _VkDevice.freeCommandBuffers(GetCurrentFrame().commandPool, cmdBuf);
}

void VulkanRenderer::CreateFrameBuffers() {
    ASSERT(_SwapchainKHR);
    const int nSwapchainCount = (int)_Images.size();
    _FrameBuffers = std::vector<vk::Framebuffer>(nSwapchainCount);

    for (int i = 0; i < nSwapchainCount; ++i) {
        std::array<vk::ImageView, 2> attchments = { _ImageViews[i], _DepthImageView };
        vk::FramebufferCreateInfo info;
        info.setRenderPass(_VkRenderPass)
            .setAttachmentCount((uint32_t)attchments.size())
            .setPAttachments(attchments.data())
            .setWidth(_SupportInfo.GetWindowWidth())
            .setHeight(_SupportInfo.GetWindowHeight())
            .setLayers(1);
        _FrameBuffers[i] = _VkDevice.createFramebuffer(info);
        ASSERT(_FrameBuffers[i]);
    }
}

void VulkanRenderer::InitSyncStructures() {

    vk::FenceCreateInfo FenceInfo;
    FenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    vk::SemaphoreCreateInfo SemapInfo;

    // Frames
    for (int i = 0; i < FRAME_OVERLAP; ++i) {
        _Frames[i].renderFence = _VkDevice.createFence(FenceInfo);
        ASSERT(_Frames[i].renderFence);

        _Frames[i].computeFence = _VkDevice.createFence(FenceInfo);
        ASSERT(_Frames[i].computeFence);

        _Frames[i].renderSemaphore = _VkDevice.createSemaphore(SemapInfo);
        ASSERT(_Frames[i].renderSemaphore);

        _Frames[i].computeSemaphore = _VkDevice.createSemaphore(SemapInfo);
        ASSERT(_Frames[i].computeSemaphore);

        _Frames[i].presentSemaphore = _VkDevice.createSemaphore(SemapInfo);
        ASSERT(_Frames[i].presentSemaphore);
    }

    // UploadContext
    vk::FenceCreateInfo uploadFenceInfo;
    _UploadContext.uploadFence = _VkDevice.createFence(uploadFenceInfo);

}

void VulkanRenderer::CreatePipeline(Material& mat, const char* vert_shader, const char* frag_shader, bool alpha) {

    PipelineBuilder pipelineBuilder;
    bool enableDepth = true, enableBlend = false;
    if (alpha) {
        enableDepth = !enableDepth;
        enableBlend = !enableBlend;
    }

    vk::ShaderModule vertShader;
    vk::ShaderModule fragShader;
    try{
        vertShader = CreateShaderModule(vert_shader);
        fragShader = CreateShaderModule(frag_shader);
    } catch(std::exception e){
        WARN("Create Pipeline failed! %s or %s is invalid direction!", vert_shader, frag_shader);
        return;
    }

    ASSERT(vertShader);
    ASSERT(fragShader);

    // INFO("Pipeline Shader Stages");
    pipelineBuilder._ShaderStages.clear();
    pipelineBuilder._ShaderStages.push_back(InitShaderStageCreateInfo(vk::ShaderStageFlagBits::eVertex, vertShader));
    pipelineBuilder._ShaderStages.push_back(InitShaderStageCreateInfo(vk::ShaderStageFlagBits::eFragment, fragShader));

    // INFO("Pipeline Bingding and Attribute Descroptions");
    vk::VertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();
    std::array<vk::VertexInputAttributeDescription, 4> attributeDescription = Vertex::GetAttributeDescription();

    //connect the pipeline builder vertex input info to the one we get from Vertex
    pipelineBuilder._VertexInputInfo.setVertexAttributeDescriptions(attributeDescription);
    pipelineBuilder._VertexInputInfo.setVertexAttributeDescriptionCount((uint32_t)attributeDescription.size());
    pipelineBuilder._VertexInputInfo.setVertexBindingDescriptions(bindingDescription);
    pipelineBuilder._VertexInputInfo.setVertexBindingDescriptionCount(1);

    // INFO("Pipeline Assembly");
    pipelineBuilder._InputAssembly = InitAssemblyStateCreateInfo(vk::PrimitiveTopology::eTriangleList);

    // INFO("Push constants");
    vk::PushConstantRange pushConstant;
    pushConstant.setOffset(0);
    pushConstant.setSize(sizeof(MeshPushConstants));
    pushConstant.setStageFlags(vk::ShaderStageFlagBits::eVertex);

    // INFO("Pipeline Layout");
    vk::PipelineLayoutCreateInfo defaultLayoutInfo = InitPipelineLayoutCreateInfo();
    
    std::vector<vk::DescriptorSetLayout> defaultSetLayouts = {_GlobalSetLayout};
    if (GetEnabledTexture()){
        defaultSetLayouts.push_back(_TextureSetLayout);
    }
    defaultLayoutInfo.setPushConstantRanges(pushConstant)
        .setPushConstantRangeCount(1)
        .setSetLayoutCount((uint32_t)defaultSetLayouts.size())
        .setSetLayouts(defaultSetLayouts);

    _PipelineLayout = _VkDevice.createPipelineLayout(defaultLayoutInfo);
    ASSERT(_PipelineLayout);
    pipelineBuilder._PipelineLayout = _PipelineLayout;

    // INFO("Pipeline Viewport");
    pipelineBuilder._Viewport.x = 0.0f;
    pipelineBuilder._Viewport.y = 0.0f;
    pipelineBuilder._Viewport.width = (float)_SupportInfo.extent.width;
    pipelineBuilder._Viewport.height = (float)_SupportInfo.extent.height;
    pipelineBuilder._Viewport.minDepth = 0.0f;
    pipelineBuilder._Viewport.maxDepth = 1.0f;

    // INFO("Pipeline Scissor");
    pipelineBuilder._Scissor.offset = vk::Offset2D{ 0, 0 };
    pipelineBuilder._Scissor.extent = _SupportInfo.extent;

    // INFO("Pipeline Rasterizer");
    pipelineBuilder._Rasterizer = InitRasterizationStateCreateInfo(vk::PolygonMode::eFill);

    // INFO("Multi SmpleState")
    pipelineBuilder._Mutisampling = InitMultisampleStateCreateInfo();

    // INFO("Blend")
    pipelineBuilder._ColorBlendAttachment = InitColorBlendAttachmentState(enableBlend);

    // INFO("Depth Stencil")
    pipelineBuilder._DepthStencilState = InitDepthStencilStateCreateInfo(enableDepth);

    _Pipeline = pipelineBuilder.BuildPipeline(_VkDevice, _VkRenderPass);
    ASSERT(_Pipeline);

    mat.pipeline = _Pipeline;
    mat.pipelineLayout = _PipelineLayout;

    _VkDevice.destroyShaderModule(vertShader);
    _VkDevice.destroyShaderModule(fragShader);
}

void VulkanRenderer::DrawPerFrame(RenderObject* first, size_t count, Particals* partical, size_t partical_count) {

    vk::Semaphore& renderSemaphore = GetCurrentFrame().renderSemaphore;
    vk::Semaphore& computeSemaphore = GetCurrentFrame().computeSemaphore;
    vk::Semaphore& presentSemaphore = GetCurrentFrame().presentSemaphore;

    auto res = _VkDevice.acquireNextImageKHR(_SwapchainKHR, std::numeric_limits<uint64_t>::max(), 
        GetCurrentFrame().presentSemaphore, nullptr);
    uint32_t nSwapchainImageIndex = res.value;

    DrawComputePipeline(partical, partical_count);

    vk::Semaphore& waitSemaphore = partical_count > 0 ? computeSemaphore : presentSemaphore;
    DrawGraphsicsPipeline(first, count, nSwapchainImageIndex, waitSemaphore, renderSemaphore);

    vk::PresentInfoKHR PresentInfo;
    PresentInfo.setSwapchainCount(1)
        .setSwapchains(_SwapchainKHR)
        .setWaitSemaphoreCount(1)
        .setWaitSemaphores(renderSemaphore)
        .setImageIndices(nSwapchainImageIndex);

    if (_Queue.GraphicsQueue.presentKHR(PresentInfo) != vk::Result::eSuccess) {
        return;
    }

    _FrameNumber++;
}

void VulkanRenderer::DrawGraphsicsPipeline(RenderObject* objects, size_t count, int swapchain_index, 
    vk::Semaphore& wait_semapore, vk::Semaphore& signal_semaphore){
    if (objects == nullptr || count == 0){
        return;
    }

    vk::CommandBuffer& cmdBuffer = GetCurrentFrame().mainCommandBuffer;
    ASSERT(cmdBuffer);
    cmdBuffer.reset();

    vk::CommandBufferBeginInfo info;
    info.setPInheritanceInfo(nullptr)
        .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
    cmdBuffer.begin(info);

    // Config
    std::array<vk::ClearValue, 2> ClearValues;
    vk::ClearColorValue color = std::array<float, 4>{0.f, 0.f, 0.f, 0.1f};
    ClearValues[0].setColor(color);
    ClearValues[1].setDepthStencil({ 1.0, 0 });

    vk::Rect2D scissor;
    scissor.extent = _SupportInfo.extent;
    cmdBuffer.setScissor(1, scissor);

    vk::Viewport viewport;
    viewport.setWidth((float)_SupportInfo.extent.width)
        .setHeight((float)_SupportInfo.extent.height)
        .setX(0)
        .setY(0)
        .setMinDepth(0)
        .setMaxDepth(1);
    cmdBuffer.setViewport(1, viewport);

    // Renderpass
    vk::RenderPassBeginInfo rpInfo;
    rpInfo.setRenderPass(_VkRenderPass)
        .setRenderArea(vk::Rect2D({ 0, 0 }, _SupportInfo.extent))
        .setFramebuffer(_FrameBuffers[swapchain_index])
        .setClearValueCount((uint32_t)ClearValues.size())
        .setClearValues(ClearValues);

    cmdBuffer.beginRenderPass(rpInfo, vk::SubpassContents::eInline);

    DrawObjects(cmdBuffer, objects, count);

    cmdBuffer.endRenderPass();
    cmdBuffer.end();

    vk::PipelineStageFlags WaitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput | 
        vk::PipelineStageFlagBits::eVertexInput;
    vk::SubmitInfo graphicsSubmit;
    graphicsSubmit.setWaitSemaphoreCount(1)
        .setWaitSemaphores(wait_semapore)
        .setSignalSemaphoreCount(1)
        .setSignalSemaphores(signal_semaphore)
        .setCommandBufferCount(1)
        .setCommandBuffers(cmdBuffer)
        .setWaitDstStageMask(WaitStage);

    _VkDevice.resetFences(GetCurrentFrame().renderFence);
    if (_Queue.GraphicsQueue.submit(1, &graphicsSubmit, GetCurrentFrame().renderFence) != vk::Result::eSuccess) {
        return;
    }

    if (_VkDevice.waitForFences(GetCurrentFrame().renderFence, true, 
        std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
        ERROR("ERROR WAIT FOR RENDER FENCES");
        return;
    }
}

void VulkanRenderer::DrawComputePipeline(Particals* particals, size_t partical_count){ 

    if (particals == nullptr || partical_count == 0){
        return;
    }

    vk::CommandBuffer& compCmdBuffer = GetCurrentFrame().mainCommandBuffer;
    ASSERT(compCmdBuffer);
    compCmdBuffer.reset();

    vk::CommandBufferBeginInfo info;
    info.setPInheritanceInfo(nullptr)
        .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
    compCmdBuffer.begin(info);

    // compute pipeline
    for (size_t i = 0; i < partical_count; ++i){
        const Particals& partical = particals[i];
        compCmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, partical.material->pipeline);
        compCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, partical.material->pipelineLayout, 0, 1, &_ComputeSet, 0, nullptr);
        compCmdBuffer.dispatch((uint32_t)partical.particals.size() / 256, 1, 1);
    }

    compCmdBuffer.end();

    vk::PipelineStageFlags WaitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput | 
        vk::PipelineStageFlagBits::eVertexInput;
    vk::SubmitInfo computeSubmit;
    computeSubmit.setWaitSemaphoreCount(1)
        .setWaitSemaphores(GetCurrentFrame().presentSemaphore)
        .setSignalSemaphoreCount(1)
        .setSignalSemaphores(GetCurrentFrame().computeSemaphore)
        .setCommandBufferCount(1)
        .setCommandBuffers(compCmdBuffer)
        .setWaitDstStageMask(WaitStage);
    
    _VkDevice.resetFences(GetCurrentFrame().computeFence);
    if (_Queue.ComputeQueue.submit(1, &computeSubmit, GetCurrentFrame().computeFence) != vk::Result::eSuccess) {
        return;
    }

    if (_VkDevice.waitForFences(GetCurrentFrame().computeFence, true,
        std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
        ERROR("ERROR WAIT FOR COMPUTE FENCES");
        return;
    }

    WaitIdel();
    _Queue.ComputeQueue.waitIdle();

    for (size_t i = 0; i < partical_count; ++i){

        Particals& partical = particals[i];
        size_t computeStorageBufferSize = partical.GetParticalCount() * sizeof(ParticalData);

        MemoryMap(partical.writeStorageBuffer, (void*)partical.writeData.data(), 0, computeStorageBufferSize);

        void* computeData = _VkDevice.mapMemory(partical.readStorageBuffer.memory, 0, computeStorageBufferSize);
        memcpy(computeData, partical.writeData.data(), computeStorageBufferSize);
        _VkDevice.unmapMemory(partical.readStorageBuffer.memory);
    }
}

void VulkanRenderer::DrawObjects(vk::CommandBuffer& cmd, RenderObject* first, size_t count) {
    
    Mesh* lastMesh = nullptr;
    Material* lastMaterial = nullptr;

    for (size_t i = 0; i < count; i++)
    {
        const RenderObject& object = first[i];
        int curFrame = _FrameNumber % FRAME_OVERLAP;

        Material* curMaterial = object.GetMaterial();
        Mesh* curMesh = object.GetMesh();
        if (curMaterial == nullptr || curMesh == nullptr) {
            continue;
        }

        //only bind the pipeline if it doesn't match with the already bound one
        if (curMaterial != lastMaterial) {
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, curMaterial->pipeline);
            lastMaterial = curMaterial;

            uint32_t uniform_offset = (uint32_t)PadUniformBuffeSize(sizeof(SceneData)) * curFrame;
            // object.material->pipelineLayout
            std::vector<vk::DescriptorSet> descSets = { _Frames[curFrame].globalDescriptor };
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, curMaterial->pipelineLayout, 0, (uint32_t)descSets.size(),
                descSets.data(), 1, &uniform_offset);

            if (curMaterial->textureSet){
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, curMaterial->pipelineLayout, 1, 1,
                    &curMaterial->textureSet, 0, nullptr);
            }
        }

        //upload the mesh to the GPU via push constants
        _PushConstants.model = object.GetTransform();
        cmd.pushConstants(curMaterial->pipelineLayout,
            vk::ShaderStageFlagBits::eVertex, 0, sizeof(MeshPushConstants), &_PushConstants);

        //only bind the mesh if it's a different one from last bind
        if (curMesh != lastMesh) {
            //bind the mesh vertex buffer with offset 0
            VkDeviceSize offset = 0;
            cmd.bindVertexBuffers(0, curMesh->vertexBuffer.buffer, offset);
            cmd.bindIndexBuffer(curMesh->indexBuffer.buffer, offset, vk::IndexType::eUint32);
            lastMesh = curMesh;
        }

        //we can now draw
        cmd.drawIndexed((uint32_t)curMesh->indices.size(), 1, 0, 0, 0);
    }
}

void VulkanRenderer::CreateDepthImage(){
    // Depth Image
    vk::Format depthFormat = FindSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eX8D24UnormPack32 },
        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    uint32_t width = _SupportInfo.GetWindowWidth();
    uint32_t height = _SupportInfo.GetWindowHeight();
    vk::Extent3D depthImageExtent = { width, height, 1 };

    _DepthImage.image = CreateImage(vk::Format::eD32Sfloat,
        vk::ImageUsageFlagBits::eDepthStencilAttachment, depthImageExtent);
    ASSERT(_DepthImage.image);
    MemRequiredInfo memInfo = QueryImgReqInfo(_DepthImage.image, vk::MemoryPropertyFlagBits::eDeviceLocal);
    _DepthImage.memory = AllocateMemory(memInfo);
    ASSERT(_DepthImage.memory);
    _VkDevice.bindImageMemory(_DepthImage.image, _DepthImage.memory, 0);

    _DepthImageView = CreateImageView(depthFormat, _DepthImage.image, vk::ImageAspectFlagBits::eDepth);
    TransitionImageLayout(_DepthImage.image, depthFormat, vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

size_t VulkanRenderer::PadUniformBuffeSize(size_t origin_size){
    // Get aligned size
    size_t minUboAlignment = _VkPhyDevice.getProperties().limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = origin_size;
    if (minUboAlignment > 0) {
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    return alignedSize;
}

void VulkanRenderer::BindTextureDescriptor(Material* mat, Texture* texture) {
    vk::DescriptorSetAllocateInfo imageAllocateInfo;
    imageAllocateInfo.setDescriptorPool(_DescriptorPool)
        .setDescriptorSetCount(1)
        .setSetLayouts(_TextureSetLayout);
    if (_VkDevice.allocateDescriptorSets(&imageAllocateInfo, &mat->textureSet) != vk::Result::eSuccess) {
        ASSERT(mat->textureSet);
    }

    vk::DescriptorImageInfo descImageInfo;
    descImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(texture->imageView)
        .setSampler(_TextureSampler);

    vk::WriteDescriptorSet writeSamplerSet = InitWriteDescriptorImage(vk::DescriptorType::eCombinedImageSampler,
        mat->textureSet, &descImageInfo, 0);
    std::vector<vk::WriteDescriptorSet> writeDescSets = { writeSamplerSet };

    _VkDevice.updateDescriptorSets(writeDescSets, nullptr);
}

void VulkanRenderer::BindBufferDescriptor(Material* mat, Particals* partical) {
    // Compute storage buffer
    const size_t computeStorageBufferSize = partical->GetParticalCount() * sizeof(ParticalData);
    if (computeStorageBufferSize == 0) {
        WARN("Buffer size is 0");
        return;
    }

    partical->readStorageBuffer.buffer = CreateBuffer(computeStorageBufferSize, vk::BufferUsageFlagBits::eStorageBuffer);
    ASSERT(partical->readStorageBuffer.buffer);
    MemRequiredInfo computeInMemInfo = QueryMemReqInfo(partical->readStorageBuffer.buffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent);
    partical->readStorageBuffer.memory = AllocateMemory(computeInMemInfo);
    ASSERT(partical->readStorageBuffer.memory);
    _VkDevice.bindBufferMemory(partical->readStorageBuffer.buffer, partical->readStorageBuffer.memory, 0);

    void* computeData = _VkDevice.mapMemory(partical->readStorageBuffer.memory, 0, computeStorageBufferSize);
    memcpy(computeData, partical->particals.data(), computeStorageBufferSize);
    _VkDevice.unmapMemory(partical->readStorageBuffer.memory);

    partical->writeStorageBuffer.buffer = CreateBuffer(computeStorageBufferSize, vk::BufferUsageFlagBits::eStorageBuffer);
    ASSERT(partical->writeStorageBuffer.buffer);
    MemRequiredInfo computeOutMemInfo = QueryMemReqInfo(partical->writeStorageBuffer.buffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent);
    partical->writeStorageBuffer.memory = AllocateMemory(computeOutMemInfo);
    ASSERT(partical->writeStorageBuffer.memory);
    _VkDevice.bindBufferMemory(partical->writeStorageBuffer.buffer, partical->writeStorageBuffer.memory, 0);
    
    ASSERT(_ComputeSetLayout);
    vk::DescriptorSetAllocateInfo computeDescAllocateInfo;
    computeDescAllocateInfo.setDescriptorPool(_DescriptorPool)
        .setDescriptorSetCount(1)
        .setSetLayouts(_ComputeSetLayout);
    if (_VkDevice.allocateDescriptorSets(&computeDescAllocateInfo, &_ComputeSet) != vk::Result::eSuccess) {
        ASSERT(_ComputeSet);
    }

    vk::DescriptorBufferInfo computeInBufferInfo;
    computeInBufferInfo.setBuffer(partical->readStorageBuffer.buffer)
        .setOffset(0)
        .setRange(computeStorageBufferSize);
    vk::DescriptorBufferInfo computeOutBufferInfo;
    computeOutBufferInfo.setBuffer(partical->writeStorageBuffer.buffer)
        .setOffset(0)
        .setRange(computeStorageBufferSize);
    vk::WriteDescriptorSet computeInWriteSet = InitWriteDescriptorBuffer(vk::DescriptorType::eStorageBuffer,
        _ComputeSet, &computeInBufferInfo, 0);
    vk::WriteDescriptorSet computeOutWriteSet = InitWriteDescriptorBuffer(vk::DescriptorType::eStorageBuffer,
        _ComputeSet, &computeOutBufferInfo, 1);
    std::vector<vk::WriteDescriptorSet> computeWriteDescSets = { computeInWriteSet, computeOutWriteSet };
    _VkDevice.updateDescriptorSets(computeWriteDescSets, nullptr);
}

void VulkanRenderer::InitDescriptors() {
    ASSERT(_VkDevice);

    //Descriptor Pool
    std::vector<vk::DescriptorPoolSize> sizes = { 
        {vk::DescriptorType::eUniformBuffer, 10},
        {vk::DescriptorType::eUniformBufferDynamic, 10},
        {vk::DescriptorType::eCombinedImageSampler, 10},
        {vk::DescriptorType::eStorageBuffer, 10}
    };

    vk::DescriptorPoolCreateInfo descPoolInfo;
    descPoolInfo.setMaxSets(10)
        .setPoolSizeCount((uint32_t)sizes.size())
        .setPoolSizes(sizes);
    _DescriptorPool = _VkDevice.createDescriptorPool(descPoolInfo);
    ASSERT(_DescriptorPool);

    // DescriptorLayout
    vk::DescriptorSetLayoutBinding cameraUniformBuffer= 
        InitDescriptorSetLayoutBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 0);
    vk::DescriptorSetLayoutBinding sceneDynamicBuffer =
        InitDescriptorSetLayoutBinding(vk::DescriptorType::eUniformBufferDynamic, 
                                       vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 1);
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {cameraUniformBuffer, sceneDynamicBuffer};

    vk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
    descSetLayoutInfo.setBindingCount((uint32_t)bindings.size())
        .setBindings(bindings);
    _GlobalSetLayout =  _VkDevice.createDescriptorSetLayout(descSetLayoutInfo);
    ASSERT(_GlobalSetLayout);

    // single layout for texture
    vk::DescriptorSetLayoutBinding textureBinding = 
        InitDescriptorSetLayoutBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 0);
    vk::DescriptorSetLayoutCreateInfo set3Info;
    set3Info.setBindingCount(1)
        .setBindings(textureBinding);
    _TextureSetLayout = _VkDevice.createDescriptorSetLayout(set3Info);
    ASSERT(_TextureSetLayout);

    // Scene dynamic buffer
    const size_t sceneParamBufferSize = FRAME_OVERLAP * PadUniformBuffeSize(sizeof(SceneData));
    _SceneParameterBuffer.buffer = CreateBuffer(sceneParamBufferSize, vk::BufferUsageFlagBits::eUniformBuffer);
    ASSERT(_SceneParameterBuffer.buffer);
    MemRequiredInfo sceneMemInfo = QueryMemReqInfo(_SceneParameterBuffer.buffer,
        vk::MemoryPropertyFlagBits::eHostVisible | 
        vk::MemoryPropertyFlagBits::eHostCoherent);
    _SceneParameterBuffer.memory = AllocateMemory(sceneMemInfo);
    ASSERT(_SceneParameterBuffer.memory);
    _VkDevice.bindBufferMemory(_SceneParameterBuffer.buffer, _SceneParameterBuffer.memory, 0);

    // Sampler
    vk::SamplerCreateInfo samplerInfo = InitSamplerCreateInfo(
        vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat);
    _TextureSampler = _VkDevice.createSampler(samplerInfo);

    // Camera Uniform buffer
    for (int i = 0; i < FRAME_OVERLAP; i++) {
        // Camera buffer
        _Frames[i].cameraBuffer.buffer = CreateBuffer(sizeof(CamerData),
            vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
        ASSERT(_Frames[i].cameraBuffer.buffer);

        MemRequiredInfo memInfo = QueryMemReqInfo(_Frames[i].cameraBuffer.buffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);
        _Frames[i].cameraBuffer.memory = AllocateMemory(memInfo);
        ASSERT(_Frames[i].cameraBuffer.memory);

        _VkDevice.bindBufferMemory(_Frames[i].cameraBuffer.buffer, _Frames[i].cameraBuffer.memory, 0);

        vk::DescriptorSetAllocateInfo descAllocateInfo;
        descAllocateInfo.setDescriptorPool(_DescriptorPool)
            .setDescriptorSetCount(1)
            .setSetLayouts(_GlobalSetLayout);
       if (_VkDevice.allocateDescriptorSets(&descAllocateInfo, &(_Frames[i].globalDescriptor)) != vk::Result::eSuccess){
           ASSERT(_Frames[i].globalDescriptor);
        }

        //  make it point into our camera buffer
        vk::DescriptorBufferInfo cameraBufferInfo;
        cameraBufferInfo.setBuffer(_Frames[i].cameraBuffer.buffer)
            .setOffset(0)
            .setRange(sizeof(CamerData));

        vk::DescriptorBufferInfo sceneBufferInfo;
        sceneBufferInfo.setBuffer(_SceneParameterBuffer.buffer)
            .setOffset(0)
            .setRange(sizeof(SceneData));

        vk::WriteDescriptorSet camerWriteSet = InitWriteDescriptorBuffer(vk::DescriptorType::eUniformBuffer,
            _Frames[i].globalDescriptor, &cameraBufferInfo, 0);

        vk::WriteDescriptorSet sceneWriteSet = InitWriteDescriptorBuffer(vk::DescriptorType::eUniformBufferDynamic,
            _Frames[i].globalDescriptor, &sceneBufferInfo, 1);

        std::vector<vk::WriteDescriptorSet> writeDescSets = {camerWriteSet, sceneWriteSet };

        _VkDevice.updateDescriptorSets(writeDescSets, nullptr);
    }
}

void VulkanRenderer::InitComputeDescriptors() {
    // compute layout
    vk::DescriptorSetLayoutBinding computeInStorageBuffer =
        InitDescriptorSetLayoutBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 0);
    vk::DescriptorSetLayoutBinding computeOutStorageBuffer =
        InitDescriptorSetLayoutBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1);
    std::vector<vk::DescriptorSetLayoutBinding> computeBindings = { computeInStorageBuffer, computeOutStorageBuffer };
    
    vk::DescriptorSetLayoutCreateInfo computeSetLayoutInfo;
    computeSetLayoutInfo.setBindingCount((uint32_t)computeBindings.size())
        .setBindings(computeBindings);
    _ComputeSetLayout = _VkDevice.createDescriptorSetLayout(computeSetLayoutInfo);
    ASSERT(_ComputeSetLayout);

}

/*
 *  Utils functions
 * */
bool VulkanRenderer::QueryQueueFamilyProp(){
    ASSERT(_VkPhyDevice);
    ASSERT(_SurfaceKHR);
     
    auto families = _VkPhyDevice.getQueueFamilyProperties();
    uint32_t index = 0;
    for(auto &family:families){
        //Pickup Graph command
        if(family.queueFlags | vk::QueueFlagBits::eGraphics){
            _QueueFamilyProp.graphicsIndex = index;
        }
        //Pickup Compute command
        if (family.queueFlags | vk::QueueFlagBits::eCompute) {
            _QueueFamilyProp.computeIndex = index;
        }
        //Pickup Surface command
        if(_VkPhyDevice.getSurfaceSupportKHR(index, _SurfaceKHR)){
            _QueueFamilyProp.presentIndex = index;
        }
        //if no command, break
        if(_QueueFamilyProp.graphicsIndex && _QueueFamilyProp.presentIndex) {
            break;
        }
        index++;
    }

#ifdef LEVEL_DEBUG 
    std::cout << "Graphics Index:" << _QueueFamilyProp.graphicsIndex.value() << 
        "\nPresent Index: " << _QueueFamilyProp.presentIndex.value() << 
        "\nCompute Index:" << _QueueFamilyProp.computeIndex.value() << std::endl;
#endif
    return _QueueFamilyProp.graphicsIndex && _QueueFamilyProp.presentIndex;
}

bool VulkanRenderer::InitQueue() {
    ASSERT(_VkDevice);
    return _Queue.InitQueue(_VkDevice, _QueueFamilyProp);
}

void VulkanRenderer::GetVkImages() {
    _Images = _VkDevice.getSwapchainImagesKHR(_SwapchainKHR);
}

void VulkanRenderer::GetVkImageViews() {
    _ImageViews.resize(_Images.size());
    for (int i = 0; i < _Images.size(); i++) {
        vk::ImageViewCreateInfo info;
        info.setImage(_Images[i]);     // Texture
        info.setFormat(_SupportInfo.format.format);
        info.setViewType(vk::ImageViewType::e2D);
        vk::ImageSubresourceRange range;
        range.setBaseMipLevel(0);
        range.setLevelCount(1);
        range.setLayerCount(1);
        range.setBaseArrayLayer(0);
        range.setAspectMask(vk::ImageAspectFlagBits::eColor);
        info.setSubresourceRange(range);
        vk::ComponentMapping mapping;
        info.setComponents(mapping);
        _ImageViews[i] = _VkDevice.createImageView(info);
    }
}

vk::ShaderModule VulkanRenderer::CreateShaderModule(const char* shader_file) {
    // Load file by binary
    std::ifstream file(shader_file, std::ios::binary | std::ios::in);
    std::vector<char> content((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();

    vk::ShaderModuleCreateInfo info;
    info.setCodeSize(content.size())
        .setPCode((uint32_t*)content.data());

    return _VkDevice.createShaderModule(info);
}

/*
    Vulkan Buffer
*/
MemRequiredInfo VulkanRenderer::QueryMemReqInfo(vk::Buffer buf, vk::MemoryPropertyFlags flag) {
    ASSERT(_VkPhyDevice);
    ASSERT(_VkDevice);
    
    MemRequiredInfo info;
    vk::PhysicalDeviceMemoryProperties property = _VkPhyDevice.getMemoryProperties();
    vk::MemoryRequirements requirement = _VkDevice.getBufferMemoryRequirements(buf);
    info.size = requirement.size;

    for (uint32_t i = 0; i < property.memoryTypeCount; i++) {
        if (requirement.memoryTypeBits & (1 << i) &&
            property.memoryTypes[i].propertyFlags & (flag)) {
            info.index = i;
        }
    }
    return info;
}

MemRequiredInfo VulkanRenderer::QueryImgReqInfo(vk::Image image, vk::MemoryPropertyFlags flag) {
    ASSERT(_VkPhyDevice);
    ASSERT(_VkDevice);

    MemRequiredInfo info;
    vk::PhysicalDeviceMemoryProperties property = _VkPhyDevice.getMemoryProperties();
    vk::MemoryRequirements requirement = _VkDevice.getImageMemoryRequirements(image);
    info.size = requirement.size;

    for (uint32_t i = 0; i < property.memoryTypeCount; i++) {
        if (requirement.memoryTypeBits & (1 << i) &&
            property.memoryTypes[i].propertyFlags & (flag)) {
            info.index = i;
        }
    }
    return info;
}

vk::Buffer VulkanRenderer::CreateBuffer(uint64_t size, vk::BufferUsageFlags flag, vk::SharingMode mode) {
    ASSERT(_VkDevice);
    vk::BufferCreateInfo info;
    info.setSharingMode(mode)
        .setQueueFamilyIndices(_QueueFamilyProp.graphicsIndex.value())
        .setSize(size)
        .setUsage(flag);
    return _VkDevice.createBuffer(info);
}

vk::DeviceMemory VulkanRenderer::AllocateMemory(MemRequiredInfo memInfo) {
    vk::MemoryAllocateInfo info;
    info.setAllocationSize(memInfo.size)
        .setMemoryTypeIndex(memInfo.index);
    return _VkDevice.allocateMemory(info);
}

void VulkanRenderer::UpLoadMeshes(Mesh& mesh) {
    //Allocate vertex buffer
    vk::DeviceSize size = (uint32_t)mesh.vertices.size() * sizeof(Vertex);
    vk::Buffer vertBuffer = CreateBuffer(size,
        vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
    ASSERT(vertBuffer);

    MemRequiredInfo vertMemInfo = QueryMemReqInfo(vertBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent);
    vk::DeviceMemory vertMemory = AllocateMemory(vertMemInfo);
    ASSERT(vertMemory);

    mesh.vertexBuffer.buffer = CreateBuffer(size, vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive);
    ASSERT(mesh.vertexBuffer.buffer);

    vertMemInfo = QueryMemReqInfo(mesh.vertexBuffer.buffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
    mesh.vertexBuffer.memory = AllocateMemory(vertMemInfo);
    ASSERT(mesh.vertexBuffer.memory);

    _VkDevice.bindBufferMemory(vertBuffer, vertMemory, 0);
    _VkDevice.bindBufferMemory(mesh.vertexBuffer.buffer, mesh.vertexBuffer.memory, 0);

    void* data = _VkDevice.mapMemory(vertMemory, 0, size);
    memcpy(data, mesh.vertices.data(), size);
    _VkDevice.unmapMemory(vertMemory);

    ImmediateSubmit([=](vk::CommandBuffer cmd){
        vk::BufferCopy regin;
        regin.setSize(size)
            .setSrcOffset(0)
            .setDstOffset(0);
        cmd.copyBuffer(vertBuffer, mesh.vertexBuffer.buffer, regin);
    });
    
    // Free mem
    _VkDevice.destroyBuffer(vertBuffer);
    _VkDevice.freeMemory(vertMemory);

    //Allocate index buffer
    vk::DeviceSize indexSize = sizeof(uint32_t) * mesh.indices.size();
    vk::Buffer indexBuffer = CreateBuffer(indexSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive);
    ASSERT(indexBuffer);

    MemRequiredInfo indexMemInfo = QueryMemReqInfo(indexBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent);
    vk::DeviceMemory indexMemory = AllocateMemory(indexMemInfo);
    ASSERT(indexMemory);

    mesh.indexBuffer.buffer = CreateBuffer(size, vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eIndexBuffer, vk::SharingMode::eExclusive);
    ASSERT(mesh.indexBuffer.buffer);

    indexMemInfo = QueryMemReqInfo(mesh.vertexBuffer.buffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
    mesh.indexBuffer.memory = AllocateMemory(indexMemInfo);
    ASSERT(mesh.indexBuffer.memory);
    
    _VkDevice.bindBufferMemory(indexBuffer, indexMemory, 0);
    _VkDevice.bindBufferMemory(mesh.indexBuffer.buffer, mesh.indexBuffer.memory, 0);

    void* indexData = _VkDevice.mapMemory(indexMemory, 0, indexSize);
    memcpy(indexData, mesh.indices.data(), indexSize);
    _VkDevice.unmapMemory(indexMemory);

    ImmediateSubmit([=](vk::CommandBuffer cmd) {
        vk::BufferCopy regin;
        regin.setSize(indexSize)
            .setSrcOffset(0)
            .setDstOffset(0);
        cmd.copyBuffer(indexBuffer, mesh.indexBuffer.buffer, regin);
        });

    // Free mem
    _VkDevice.destroyBuffer(indexBuffer);
    _VkDevice.freeMemory(indexMemory);
}

vk::Format VulkanRenderer::FindSupportedFormat(const std::vector<vk::Format>& candidates,
    vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
    for (vk::Format format : candidates) {
        vk::FormatProperties props;
        _VkPhyDevice.getFormatProperties(format, &props);
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
        else {
            WARN("Find Supported Format Failed...");
        }
    }

    return vk::Format();
}

void VulkanRenderer::TransitionImageLayout(vk::Image image, vk::Format format,
    vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {

    vk::CommandBuffer cmdBuf = AllocateCmdBuffer();
    ASSERT(cmdBuf);

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags dstStage;
    vk::ImageMemoryBarrier barrier;
    vk::ImageSubresourceRange subRange;
    subRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseArrayLayer(0)
        .setBaseMipLevel(0)
        .setLayerCount(1)
        .setLevelCount(1);
    barrier.setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
        .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
        .setImage(image)
        .setSubresourceRange(subRange);

    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);
        if (IsContainStencilComponent(format)) {
            barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
        }
        else {
            barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);
        }
    }
    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
            .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eNone);
        barrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead |
            vk::AccessFlagBits::eDepthStencilAttachmentWrite);

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else {
        throw std::runtime_error("Transfer ImageLayout Failed...");
    }

    cmdBuf.pipelineBarrier(sourceStage, dstStage, vk::DependencyFlagBits::eByRegion, 0, nullptr, 0, nullptr, 1, &barrier);
    cmdBuf.end();
    EndCmdBuffer(cmdBuf);
}

void VulkanRenderer::UpdatePushConstants(glm::mat4 view_matrix) {
    _Camera.view = view_matrix;

    // Uniform Buffer
    _Camera.proj = glm::perspective(glm::radians(45.f),
        _SupportInfo.GetWindowWidth() / (float)_SupportInfo.GetWindowHeight(), 0.1f, 200.0f);
    _Camera.proj[1][1] *= -1;

    void* data = _VkDevice.mapMemory(GetCurrentFrame().cameraBuffer.memory, 0, sizeof(CamerData));
    memcpy(data, &_Camera, sizeof(CamerData));
    _VkDevice.unmapMemory(GetCurrentFrame().cameraBuffer.memory);
}

void VulkanRenderer::UpdateUniformBuffer(){

    _Camera.proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10000.0f);
    _Camera.proj[1][1] *= -1;
    _Camera.view = { 1,0,0,0,
                 0,1,0,0,
                 0,0,1,0,
                 0,0,0,1 };
}

void VulkanRenderer::UpdateDynamicBuffer(){
    float framed = (_FrameNumber / 3600.f);
    _SceneData.ambientColor = { 1, 1, 1, 0.5 };
    _SceneData.fogColor = { 1, 1, 1, 0 };
    _SceneData.fogDistances = { 0, 1, 0, 0 };
    _SceneData.sunlightColor = { 1, 1, 1, 0 };
    //_SceneData.sunlightDirection = { sin(framed), 0, cos(framed), 0 };
    _SceneData.sunlightDirection = { 1, -1, 1, 0 };
    _SceneData.pointLightPos = { 0, 0, -1, 1 };
    _SceneData.lightSpecular = { 0.5, 0.5, 0.5, 1 };
    int frameIndex = _FrameNumber % FRAME_OVERLAP;

    size_t memOffset = PadUniformBuffeSize(sizeof(SceneData)) * frameIndex;

    void* sceneData = _VkDevice.mapMemory(_SceneParameterBuffer.memory, memOffset, sizeof(_SceneData));
    memcpy(sceneData, &_SceneData, sizeof(SceneData));
    _VkDevice.unmapMemory(_SceneParameterBuffer.memory);
}

void VulkanRenderer::ImmediateSubmit(std::function<void(vk::CommandBuffer cmd)>&& function){

    vk::CommandBuffer cmd = _UploadContext.commandBuffer;
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setPInheritanceInfo(nullptr)
        .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    // Execute function
    cmd.begin(beginInfo);
    function(cmd);
    cmd.end();

    vk::SubmitInfo submit;
    submit.setWaitSemaphoreCount(0)
        .setWaitSemaphores(nullptr)
        .setSignalSemaphoreCount(0)
        .setSignalSemaphores(nullptr)
        .setCommandBufferCount(0)
        .setCommandBuffers(cmd)
        .setWaitDstStageMask(nullptr);
    if (_Queue.GraphicsQueue.submit(1, &submit, _UploadContext.uploadFence) != vk::Result::eSuccess){
        ERROR("Submit command failed");
    }

    if (_VkDevice.waitForFences(_UploadContext.uploadFence, true, 
        std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
        ERROR("ERROR WAIT FOR FENCES");
        return;
    }

    _VkDevice.resetFences(_UploadContext.uploadFence);
    _VkDevice.resetCommandPool(_UploadContext.commandPool);
}

/*
*  Pipeline
*  All Stage
*/
vk::PipelineShaderStageCreateInfo VulkanRenderer::InitShaderStageCreateInfo(vk::ShaderStageFlagBits stage, vk::ShaderModule shader_module) {
    vk::PipelineShaderStageCreateInfo info;
    info.setStage(stage)
        .setModule(shader_module)
        .setPName("main");

    return info;
}

vk::PipelineVertexInputStateCreateInfo VulkanRenderer::InitVertexInputStateCreateInfo() {
    vk::PipelineVertexInputStateCreateInfo info;
    info.setVertexBindingDescriptionCount(0)
        .setVertexAttributeDescriptionCount(0);

    return info;
}

vk::PipelineInputAssemblyStateCreateInfo VulkanRenderer::InitAssemblyStateCreateInfo(vk::PrimitiveTopology topology) {
    vk::PipelineInputAssemblyStateCreateInfo info;
    info.setTopology(topology)
        .setPrimitiveRestartEnable(VK_FALSE);

    return info;
}

vk::PipelineRasterizationStateCreateInfo VulkanRenderer::InitRasterizationStateCreateInfo(vk::PolygonMode polygonMode) {
    vk::PipelineRasterizationStateCreateInfo info;
    info.setLineWidth(1.0f)
        // No back cull
        .setFrontFace(vk::FrontFace::eClockwise)
        .setCullMode(vk::CullModeFlagBits::eNone)
        // Depth
        .setDepthBiasClamp(0.0f)
        .setDepthBiasEnable(VK_FALSE)
        .setDepthClampEnable(VK_FALSE)
        .setDepthBiasSlopeFactor(0.0f)
        .setDepthBiasConstantFactor(0.0f)
        .setPolygonMode(polygonMode)
        .setRasterizerDiscardEnable(VK_FALSE);

    return info;
}

vk::PipelineMultisampleStateCreateInfo VulkanRenderer::InitMultisampleStateCreateInfo() {
    vk::PipelineMultisampleStateCreateInfo info;
    info.setSampleShadingEnable(_bEnabledSampleShading)
        .setMinSampleShading(.2f)
        .setAlphaToOneEnable(VK_FALSE)
        .setAlphaToCoverageEnable(VK_FALSE)
        .setPSampleMask(nullptr)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1);

    return info;
}

vk::PipelineColorBlendAttachmentState VulkanRenderer::InitColorBlendAttachmentState(bool enable_Blend) {
    vk::PipelineColorBlendAttachmentState info;
    info.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    info.setBlendEnable(enable_Blend);

    if (enable_Blend) {
        info.setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    }

    return info;
}

vk::PipelineLayoutCreateInfo VulkanRenderer::InitPipelineLayoutCreateInfo() {
    vk::PipelineLayoutCreateInfo info;
    info.setSetLayoutCount(0)
        .setPSetLayouts(nullptr)
        .setPushConstantRangeCount(0)
        .setPushConstantRanges(nullptr);

    return info;
}

vk::PipelineDepthStencilStateCreateInfo VulkanRenderer::InitDepthStencilStateCreateInfo(bool enable_depth) {
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
    depthStencilInfo.setDepthTestEnable(enable_depth)    //Compare coming depth buffer, abandon it or not
        .setDepthWriteEnable(enable_depth)   //Test write new depth buffer or not
        .setDepthCompareOp(vk::CompareOp::eLess)
        .setDepthBoundsTestEnable(VK_FALSE)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f)
        .setStencilTestEnable(VK_FALSE);

    return depthStencilInfo;
}

/*
    Vulkan DescriptorSet 
 */
vk::DescriptorSetLayoutBinding VulkanRenderer::InitDescriptorSetLayoutBinding(vk::DescriptorType type, 
    vk::ShaderStageFlags stage, uint32_t binding){

    vk::DescriptorSetLayoutBinding bindingInfo;
    bindingInfo.setBinding(binding)
        .setDescriptorCount(1)
        .setDescriptorType(type)
        .setPImmutableSamplers(nullptr)
        .setStageFlags(stage);

    return bindingInfo;
}


vk::WriteDescriptorSet VulkanRenderer::InitWriteDescriptorBuffer(vk::DescriptorType type, vk::DescriptorSet dstSet, 
    vk::DescriptorBufferInfo* bufferInfo, uint32_t binding){

    vk::WriteDescriptorSet writeSet;
    writeSet.setDstBinding(binding)
        .setDstSet(dstSet)
        .setDescriptorCount(1)
        .setDescriptorType(type)
        .setPBufferInfo(bufferInfo); 

    return writeSet;
}

vk::WriteDescriptorSet VulkanRenderer::InitWriteDescriptorImage(vk::DescriptorType type, vk::DescriptorSet dstSet,
    vk::DescriptorImageInfo* imageInfo, uint32_t binding) {
    vk::WriteDescriptorSet writeSet;
    writeSet.setDstSet(dstSet)
        .setDstBinding(binding)
        .setPImageInfo(imageInfo)
        .setDescriptorCount(1)
        .setDescriptorType(type);   //vk::DescriptorType::eCombinedImageSampler
    return writeSet;
}

vk::SamplerCreateInfo VulkanRenderer::InitSamplerCreateInfo(vk::Filter filter, vk::SamplerAddressMode samplerAddressMode) {
    vk::SamplerCreateInfo info;
    info.setAddressModeU(samplerAddressMode)  // vk::SamplerAddressMode::eRepeat
        .setAddressModeV(samplerAddressMode)
        .setAddressModeW(samplerAddressMode)
        .setMagFilter(filter)     // vk::Filter::eLinear
        .setMinFilter(filter)
        .setAnisotropyEnable(VK_FALSE)
        .setMaxAnisotropy(1)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
        .setUnnormalizedCoordinates(VK_FALSE)
        .setCompareEnable(VK_FALSE)
        .setCompareOp(vk::CompareOp::eAlways)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setMipLodBias(0.0f)
        .setMaxLod(0.0f)
        .setMinLod(0.0f);
    return info;
}

/*
    Image
*/
vk::Image VulkanRenderer::CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::Extent3D extent) {
    ASSERT(_VkDevice);

    vk::ImageCreateInfo info;
    info.setImageType(vk::ImageType::e2D)
        .setMipLevels(1)
        .setFormat(format)
        .setExtent(extent)
        .setArrayLayers(1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setUsage(usage)
        .setSamples(vk::SampleCountFlagBits::e1);
    return _VkDevice.createImage(info);
}

vk::ImageView VulkanRenderer::CreateImageView(vk::Format format, vk::Image image, vk::ImageAspectFlags aspect) {
    ASSERT(_VkDevice);

    vk::ImageSubresourceRange subresourceRange;
    subresourceRange.setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1)
        .setAspectMask(aspect);
    
    vk::ImageViewCreateInfo info;
    info.setViewType(vk::ImageViewType::e2D)
        .setFormat(format)
        .setImage(image)
        .setSubresourceRange(subresourceRange);
    return _VkDevice.createImageView(info);
}

void VulkanRenderer::CreateDrawLinePipeline(Material& mat, const char* vert_shader, const char* frag_shader) {
    INFO("Create Drawline Pipeline");

    PipelineBuilder pipelineBuilder;

    vk::ShaderModule vertShader = CreateShaderModule(vert_shader);
    vk::ShaderModule fragShader = CreateShaderModule(frag_shader);
    ASSERT(vertShader);
    ASSERT(fragShader);

    // INFO("Pipeline Shader Stages");
    pipelineBuilder._ShaderStages.clear();
    pipelineBuilder._ShaderStages.push_back(InitShaderStageCreateInfo(vk::ShaderStageFlagBits::eVertex, vertShader));
    pipelineBuilder._ShaderStages.push_back(InitShaderStageCreateInfo(vk::ShaderStageFlagBits::eFragment, fragShader));

    // INFO("Pipeline Bingding and Attribute Descroptions");
    vk::VertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();
    std::array<vk::VertexInputAttributeDescription, 4> attributeDescription = Vertex::GetAttributeDescription();

    //connect the pipeline builder vertex input info to the one we get from Vertex
    pipelineBuilder._VertexInputInfo.setVertexAttributeDescriptions(attributeDescription);
    pipelineBuilder._VertexInputInfo.setVertexAttributeDescriptionCount((uint32_t)attributeDescription.size());
    pipelineBuilder._VertexInputInfo.setVertexBindingDescriptions(bindingDescription);
    pipelineBuilder._VertexInputInfo.setVertexBindingDescriptionCount(1);

    // INFO("Pipeline Assembly");
    pipelineBuilder._InputAssembly = InitAssemblyStateCreateInfo(vk::PrimitiveTopology::eLineStrip);

    // INFO("Push constants");
    vk::PushConstantRange pushConstant;
    pushConstant.setOffset(0);
    pushConstant.setSize(sizeof(MeshPushConstants));
    pushConstant.setStageFlags(vk::ShaderStageFlagBits::eVertex);

    // INFO("Pipeline Layout");
    vk::PipelineLayoutCreateInfo defaultLayoutInfo = InitPipelineLayoutCreateInfo();
    std::vector<vk::DescriptorSetLayout> defaultSetLayouts = { _GlobalSetLayout };
    defaultLayoutInfo.setPushConstantRanges(pushConstant)
        .setPushConstantRangeCount(1)
        .setSetLayoutCount((uint32_t)defaultSetLayouts.size())
        .setSetLayouts(defaultSetLayouts);

    _PipelineLayout = _VkDevice.createPipelineLayout(defaultLayoutInfo);
    ASSERT(_PipelineLayout);
    pipelineBuilder._PipelineLayout = _PipelineLayout;

    // INFO("Pipeline Viewport");
    pipelineBuilder._Viewport.x = 0.0f;
    pipelineBuilder._Viewport.y = 0.0f;
    pipelineBuilder._Viewport.width = (float)_SupportInfo.extent.width;
    pipelineBuilder._Viewport.height = (float)_SupportInfo.extent.height;
    pipelineBuilder._Viewport.minDepth = 0.0f;
    pipelineBuilder._Viewport.maxDepth = 1.0f;

    // INFO("Pipeline Scissor");
    pipelineBuilder._Scissor.offset = vk::Offset2D{ 0, 0 };
    pipelineBuilder._Scissor.extent = _SupportInfo.extent;

    // INFO("Pipeline Rasterizer");
    pipelineBuilder._Rasterizer = InitRasterizationStateCreateInfo(vk::PolygonMode::eLine);
    pipelineBuilder._Mutisampling = InitMultisampleStateCreateInfo();
    pipelineBuilder._ColorBlendAttachment = InitColorBlendAttachmentState(false);
    pipelineBuilder._DepthStencilState = InitDepthStencilStateCreateInfo(true);

    _DrawLinePipeline = pipelineBuilder.BuildPipeline(_VkDevice, _VkRenderPass);
    ASSERT(_DrawLinePipeline);

    mat.pipeline = _DrawLinePipeline;
    mat.pipelineLayout = _PipelineLayout;

    _VkDevice.destroyShaderModule(vertShader);
    _VkDevice.destroyShaderModule(fragShader);
}

void VulkanRenderer::CreateComputePipeline(Material& mat, const char* comp_shader) {
    INFO("Create Compute Pipeline.");

    vk::ShaderModule computeShader;
    try{
        computeShader = CreateShaderModule(comp_shader);
    } catch(std::exception e){
        WARN("Create Compute Pipeline failed!");
        return;
    }

    vk::PipelineShaderStageCreateInfo shaderStageInfo =
        InitShaderStageCreateInfo(vk::ShaderStageFlagBits::eCompute, computeShader);

    vk::PipelineLayoutCreateInfo defaultLayoutInfo = InitPipelineLayoutCreateInfo();
    std::vector<vk::DescriptorSetLayout> defaultSetLayouts = { _ComputeSetLayout };
    defaultLayoutInfo.setSetLayoutCount((uint32_t)defaultSetLayouts.size())
        .setSetLayouts(defaultSetLayouts);
    mat.pipelineLayout = _VkDevice.createPipelineLayout(defaultLayoutInfo);

    vk::ComputePipelineCreateInfo info;
    info.setStage(shaderStageInfo)
        .setLayout(mat.pipelineLayout);

    auto res = _VkDevice.createComputePipeline(nullptr, info);
    if (res.result != vk::Result::eSuccess) {
        WARN("Create compute pipeline failed.");
    }

    mat.pipeline = res.value;

    _VkDevice.destroyShaderModule(computeShader);
}
