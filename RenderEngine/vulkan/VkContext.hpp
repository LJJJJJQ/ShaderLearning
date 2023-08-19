#pragma once
#include "interface/IContext.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include "vulkan/vulkan_handles.hpp"

namespace VkCore{
class VkContext : public IContext{
public:
    VkContext();
    virtual ~VkContext();
    virtual bool InitContext();
    virtual void Release();

public:
    bool CreateInstance();
    bool CreatePhysicalDevice();
    bool CreateDevice();
    bool CreateSurface();

    SDL_Window* GetWindow() const {return _Window;}
    vk::Instance GetInstance() const {return _VkInstance;} 
    vk::Device GetDevice() const {return _VkDevice;}
    vk::SurfaceKHR GetSurface() const {return _VkSurface;}

private:
    bool InitWindow();

private:
    Instance* _Instance;
    Device* _Device;
    Surface* _Surface;

};
}


