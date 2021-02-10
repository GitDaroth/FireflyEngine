# FireflyEngine

The FireflyEngine is an ambitious project of developing a game engine from scratch for learning purposes. This early-stage engine is currently solely developed by me. The rendering is done optionally with Vulkan or OpenGL.

![pistol_1](/showcase/pistol_1.gif)
![pistol_2](/showcase/pistol_2.gif)
![roughness_vs_metalness](/showcase/roughness_vs_metalness.gif)

**Engine Features:**
- simple graphics API abstraction layer for OpenGL and Vulkan
- forward rendering
- input (mouse, keyboard, gamepad) and window events with [GLFW](https://github.com/glfw/glfw)
- normal mapping
- parallax mapping
- cubemaps
- basic real-time PBR shader and material
- image based lighting (IBL) for PBR
- texture loading with mipmaps and anisotropic filtering
- anti-aliasing (MSAA)
- mesh loading with [assimp](https://github.com/assimp/assimp)
- simple resource registry for meshes, textures, shaders and materials
- entity, component, system (ECS) with [EnTT](https://github.com/skypjack/entt)
- logger with [spdlog](https://github.com/gabime/spdlog)

**Upcoming Features:**
- lights
- deferred rendering pass
- scenegraph
- batch rendering
- shadow mapping
- skeletal animation
- multithreaded command recording with Vulkan
- scripting
- editor
- support for DirectX12
- rigid body physics integration with [NVIDIA PhysX](https://github.com/NVIDIAGameWorks/PhysX)
- cross-platfrom support (Windows, Linux, Mac)

## Build Instructions
The FireflyEngine only supports **Windows** at the moment!

For Visual Studio 2019:
```
git clone --recursive https://github.com/GitDaroth/FireflyEngine
cd FireflyEngine
cmake_generate_VS2019.bat
```
Open the generated Visual Studio solution in the "build" folder and build the "Sandbox" target.

## Dependencies
- min. OpenGL 4.5 or Vulkan 1.2
- [GLFW](https://github.com/glfw/glfw)
- [Glad](https://glad.dav1d.de)
- [glm](https://github.com/g-truc/glm)
- [stb_image](https://github.com/nothings/stb)
- [assimp](https://github.com/assimp/assimp)
- [spdlog](https://github.com/gabime/spdlog)
- [ImGui](https://github.com/ocornut/imgui)
- [EnTT](https://github.com/skypjack/entt)
- [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect)