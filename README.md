

## ��Ŀ����

��Ŀ��ַ[github:KVM-Explorer/MutiDeviceRender](https://github.com/KVM-Explorer/MutiDeviceRender)

��Ŀ��֯�ṹ
```
��  .gitignore
��  CMakeLists.txt
��  CMakePresets.json
��  README.md
��
����RayTrace
��  ��  CMakeLists.txt
��  ��  Main.cpp
��  ��  Render.cpp
��  ��  Render.hpp
��  ��  test.hpp
��  ��
��  ����shaders
��          Comman.frag
��          Comman.vert
��          RayTrace.comp
��          Texture.frag
��          Texture.vert
��
����SFR
    ��  CMakeLists.txt
    ��  Main.cpp
    ��  MultiRender.cpp
    ��  MultiRender.hpp
    ��  RAII.hpp
    ��
    ����shaders
            Common.frag
            Common.vert
            RayTrace.comp
            Texture.frag
            Texture.vert
```
## ����ϵͳ����

- Windows10
- NVIDIA Geforce GTX 1065�����Կ�
    - **�����汾 30.0.14.7168**
    - �������� 2021/8/5
- Intel UHD Graphics 630
    - �����汾 30.0.101.1070
    - �������� 2021/12/10
## �����ⰲװ

Windows10 �»���Visual Studio 2022������ʹ��CMake������Ŀ����ʵ���Զ�����vulkan Shader����

> - CMake 3.10+
> - ��װvcpkg�����û������� VCPKG_ROOT ����Ϊvcpk�İ�װ·�� eg. VCPKG_ROOT="xx\xx\xx\vcpkg"
> - ��װVulkan SDK(�Զ����û���������glslc)

���ն�ʹ��vcpkg��װ��������
```shell
vcpkg install glm:x64-windows glfw3:x64-windows
```
## �������

�л�git��֧��OffscreenRender��֧����visual stduio 2022�������������Գ���

1. RayTrace����������Ⱦ����׷�ٲ���
2. SFR��������+���Ի����Ⱦͬ֡ͼ��
## TODO

- [x]  CMAKE �Զ���������shader����SPV