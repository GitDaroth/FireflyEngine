# FireflyEngine

The FireflyEngine is an ambitious project of developing a game engine from scratch for learning purposes. This early-stage engine is currently solely developed by me.

![scene1](/showcase/scene1.gif)
![scene1_close](/showcase/scene1_close.gif)
![scene2](/showcase/scene2.gif)

**Engine Features:**
- simple graphics API abstraction layer
- OpenGL forward rendering
- input (mouse + keyboard + gamepad) and window events
- normal mapping
- parallax mapping
- basic real-time PBR shader and material
- texture loading with mipmaps and anisotropic filtering
- mesh loading
- logger

**Upcoming Features:**
- resource managers for meshes, textures, shaders, ...
- deferred rendering pass
- scenegraph
- entity, component, system (ECS)
- batch rendering
- screen-space ambient occlusion (SSAO)
- cubemaps
- image based lighting (IBL) for PBR
- shadow mapping
- anti-aliasing
- skeletal animation
- support for multithreading
- scripting
- editor
- support for DirectX12 or Vulkan
- shader variants generation
- rigid body physics integration
- cross-platfrom support (Windows, Linux, Mac)

## Build Instructions
The FireflyEngine only supports Windows at the moment!

For Visual Studio 2019:
```
git clone --recursive https://github.com/GitDaroth/FireflyEngine
cd FireflyEngine
cmake_generate_VS2019.bat
```
Open the generated Visual Studio solution in the "build" folder and build the "Sandbox" target.

## Dependencies
- [GLFW](https://github.com/glfw/glfw)
- [Glad](https://glad.dav1d.de)
- [glm](https://github.com/g-truc/glm)
- [stb_image](https://github.com/nothings/stb)
- [assimp](https://github.com/assimp/assimp)
- [spdlog](https://github.com/gabime/spdlog)
- [ImGui](https://github.com/ocornut/imgui)
- [EnTT](https://github.com/skypjack/entt)