[Chinese](./README_zh.md)

# Suzza

A small 3D rendering engine written in C++ with a Vulkan rendering backend.
It uses an Actor + Component architecture and ships a lightweight reflection system.

> Aimed at engine-development learners and students; suitable for study, research and building on top of this framework.

## Documentation

| Document | Contents |
| --- | --- |
| [Build & run guide](docs/BUILD_en.md) | From a clean machine to a running window, step by step, including Windows 10 / 11 differences and troubleshooting ([中文](docs/BUILD_zh.md)) |
| [Source reading guide](docs/codeReader_en.docx) | 25-page reading map: a seven-stop route through the code, the core mechanisms, hands-on exercises ([PDF](docs/codeReader_en.pdf) / [中文](docs/codeReader.docx)) |

## Features
- Vulkan rendering backend
- GLFW window & input
- ImGui (Docking branch) for the debug UI and editor panels
- **Edit actor and component data at runtime in the editor**
- Actor + Component entity framework
- Lightweight reflection system, implemented by a stand-alone LLVM-based code generator
- GLM for mathematics
- stb_image for texture image loading
- cgltf for glTF model loading
- Separate reflection code generator tool
  - The reflection tool is an independent project.
  - Repository: [https://github.com/KomdSpecific/suzza-reflect-tool](https://github.com/KomdSpecific/suzza-reflect-tool)
  - Its **source** depends on external LLVM/Clang libraries for AST parsing and code generation.
  - A prebuilt generator is committed under `tool/ReflectGenerated/`, so building this repository does **not** require LLVM.

## Third-party libraries
All libraries below are vendored under `thirdparty/` with non-essential files stripped (examples, tests, docs etc.).
- Dear ImGui (Docking, MIT)
- GLFW (zlib/libpng)
- GLM (MIT)
- stb_image (Public Domain / MIT dual license)
- cgltf (MIT)

See [`NOTICE.txt`](./NOTICE.txt) for complete copyright and license information of third-party libraries.

## Requirements
- Windows 10 / 11 on x64
- **Visual Studio 2026 or 2022** (Community or Build Tools) with the bundled MSVC compiler
- CMake 3.21+ (VS 2026 ships 4.1, which is fine) and Ninja
- Vulkan SDK 1.3.x, including `glslc`
- MSVC is the only verified toolchain; **GCC and Clang are untested**

## Build
Run this in an **x64 Native Tools Command Prompt** (the 2026 entry is *... for VS*, the 2022 one is *... for VS 2022*).
A Ninja build is required: the reflection step reads `compile_commands.json`, which only Ninja / Makefile-style generators produce.

```bat
git clone https://github.com/KomdSpecific/Suzza.git
cd Suzza
cmake -S . -B out/build/x64-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build out/build/x64-debug
```

The result is `out/build/x64-debug/SuzzaEngine.exe`, with `assets/` copied next to it after the build.

Full setup instructions, a self-check script and a troubleshooting table live in [docs/BUILD_en.md](docs/BUILD_en.md).
