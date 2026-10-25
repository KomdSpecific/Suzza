[Chinese](./README_zh.md)

# Suzza

A small 3D rendering engine written in C++ with Vulkan rendering backend.
The engine adopts Actor + Component architecture and has a built-in lightweight reflection system.

> Targeted at engine development learners and students, suitable for study, research and secondary development based on this framework.

## Features
- Vulkan rendering backend
- GLFW window & input
- ImGui (Docking branch) for debug UI and editor panels
- Support **modifying actor and component data at runtime in editor**
- Actor + Component entity component framework
- Lightweight reflection system: implemented by an independent LLVM reflection code generator
- GLM for mathematics
- stb_image for texture image loading
- cgltf for glTF model loading
- Separate reflection code generator tool
  - The reflection tool is an independent project.
  - Repository: [https://github.com/KomdSpecific/suzza-reflect-tool]
  - It depends on external LLVM/Clang libraries for AST parsing and code generation.
  - LLVM source code is **not included** in this repository.

## Third-party Libraries
All libraries below are vendored under `thirdparty/` with non-essential files stripped (examples, tests, docs etc.).
- Dear ImGui (Docking, MIT)
- GLFW (zlib/libpng)
- GLM (MIT)
- stb_image (Public Domain / MIT dual license)
- cgltf (MIT)

See [`NOTICE`](./NOTICE) for complete copyright and license information of third-party libraries.

## Build Requirements
- C++20 or newer
- CMake3.8+
- Vulkan SDK
- MSVC compiler (**Only MSVC is verified and supported for now; GCC / Clang compatibility is not tested**)
- For reflection tool: prebuilt LLVM + Clang development packages (external dependency)

## Build Instructions
```bash
# Example
mkdir build
cd build
cmake ..
cmake --build . --config Release

