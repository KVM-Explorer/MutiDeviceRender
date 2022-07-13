

## 项目简述

项目地址[github:KVM-Explorer/MutiDeviceRender](https://github.com/KVM-Explorer/MutiDeviceRender)

项目组织结构
```
│  .gitignore
│  CMakeLists.txt
│  CMakePresets.json
│  README.md
│
├─RayTrace
│  │  CMakeLists.txt
│  │  Main.cpp
│  │  Render.cpp
│  │  Render.hpp
│  │  test.hpp
│  │
│  └─shaders
│          Comman.frag
│          Comman.vert
│          RayTrace.comp
│          Texture.frag
│          Texture.vert
│
└─SFR
    │  CMakeLists.txt
    │  Main.cpp
    │  MultiRender.cpp
    │  MultiRender.hpp
    │  RAII.hpp
    │
    └─shaders
            Common.frag
            Common.vert
            RayTrace.comp
            Texture.frag
            Texture.vert
```
## 开发系统环境

- Windows10
- NVIDIA Geforce GTX 1065独立显卡
    - **驱动版本 30.0.14.7168**
    - 驱动日期 2021/8/5
- Intel UHD Graphics 630
    - 驱动版本 30.0.101.1070
    - 驱动日期 2021/12/10
## 依赖库安装

Windows10 下基于Visual Studio 2022开发并使用CMake构建项目，并实现自动编译vulkan Shader程序

> - CMake 3.10+
> - 安装vcpkg并配置环境变量 VCPKG_ROOT 内容为vcpk的安装路径 eg. VCPKG_ROOT="xx\xx\xx\vcpkg"
> - 安装Vulkan SDK(自动配置环境变量和glslc)

打开终端使用vcpkg安装依赖环境
```shell
vcpkg install glm:x64-windows glfw3:x64-windows
```
## 程序测试

切换git分支到OffscreenRender分支，在visual stduio 2022中启动下述测试程序

1. RayTrace——独显渲染光线追踪测试
2. SFR——独显+集显混合渲染同帧图像
## TODO

- [x]  CMAKE 自动拷贝调用shader生成SPV