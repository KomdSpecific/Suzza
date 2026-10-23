[ÖÐÎÄ](./README_zh.md)

# Suzza

A small 3D rendering engine implemented with Vulkan backend, written in C++.

> This is a learning-focused mini engine, not intended for production use.

## Features
- Vulkan rendering backend
- GLFW window & input
- ImGui (Docking branch) for debug UI and editor panels
- GLM for mathematics
- stb_image for texture image loading
- cgltf for glTF model loading
- Separate reflection code generator tool
  - The reflection tool is an independent project.
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
- A C++ compiler supporting Vulkan (MSVC / GCC / Clang)
- For reflection tool: prebuilt LLVM + Clang development packages (external dependency)

## Build Instructions
```bash
# Example
cmake -S . -B build
cmake --build build

