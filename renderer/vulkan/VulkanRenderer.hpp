#pragma once

#ifdef DEBUG
#include <iostream>
#endif // DEBUG

#include <vector>
#include <vulkan/vulkan.hpp>
#include "../interface/IRendererImpl.hpp"
#include "../../application/Window.hpp"
#include "../../application/utils/EngineUtils.hpp"

#include "VkDevice.hpp"
#include "Swapchain.hpp"
/*
    Dont forget delete Init()
*/

namespace renderer {

    class PipelineBuilder;

    class VulkanRenderer : public IRendererImpl {
    public:
        VulkanRenderer();
        virtual ~VulkanRenderer();
        virtual bool Init() override;
        virtual void Release() override;
        virtual void InitWindow() override;
        virtual void CreateInstance() override;
        virtual void PickupPhyDevice() override;
        virtual void CreateSurface() override;
        virtual void CreateDevice() override;
        virtual void CreateSwapchain() override;
        virtual void CreateRenderPass() override;
        virtual void CreateCmdPool() override;
        virtual void CreateFrameBuffers() override;
        virtual void InitSyncStructures() override;
        virtual void CreatePipeline() override;
        virtual void DrawPerFrame() override;

    public:
        vk::CommandBuffer AllocateCmdBuffer();

    protected:
        bool QueryQueueFamilyProp();
        bool InitQueue();
        void GetVkImages();
        void GetVkImageViews();
        vk::ShaderModule CreateShaderModule(const char* shader_file);

    private:
        // Pipeline stages
        vk::PipelineShaderStageCreateInfo InitShaderStageCreateInfo(vk::ShaderStageFlagBits stage, vk::ShaderModule shader_module);
        vk::PipelineVertexInputStateCreateInfo InitVertexInputStateCreateInfo();
        vk::PipelineInputAssemblyStateCreateInfo InitAssemblyStateCreateInfo(vk::PrimitiveTopology topology);
        vk::PipelineRasterizationStateCreateInfo InitRasterizationStateCreateInfo(vk::PolygonMode polygonMode);
        vk::PipelineMultisampleStateCreateInfo InitMultisampleStateCreateInfo();
        vk::PipelineColorBlendAttachmentState InitColorBlendAttachmentState();
        vk::PipelineLayoutCreateInfo InitPipelineLayoutCreateInfo();

    protected:
        QueueFamilyProperty _QueueFamilyProp;
        SwapchainSupport _SupportInfo;
        Queue _Queue;

        SDL_Window* _Window;
        vk::Instance _VkInstance;
        vk::PhysicalDevice _VkPhyDevice;
        vk::SurfaceKHR _SurfaceKHR;
        vk::Device _VkDevice;
        vk::SwapchainKHR _SwapchainKHR;

        std::vector<vk::Image> _Images;
        std::vector<vk::ImageView> _ImageViews;
        std::vector<vk::Framebuffer> _FrameBuffers;

        vk::RenderPass _VkRenderPass;
        vk::CommandPool _VkCmdPool;
        vk::CommandBuffer _VkCmdBuffer;
        vk::Semaphore _RenderSemaphore;
        vk::Semaphore _PresentSemaphore;
        vk::Fence _RenderFence;
        vk::PipelineLayout _PipelineLayout;
        vk::Pipeline _Pipeline;

        vk::ShaderModule _VertShader;
        vk::ShaderModule _FragShader;

    };// class VulkanRenderer
}// namespace renderer
