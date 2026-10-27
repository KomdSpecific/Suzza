[English](./README.md)

# Suzza

一个基于 Vulkan 渲染后端、使用 C++ 编写的小型 3D 渲染引擎。
引擎采用 Actor + Component 架构，内置简易反射系统。

> 面向引擎开发学习者、学生群体，适合学习研究以及基于该框架二次开发。

## 文档

| 文档 | 内容 |
| --- | --- |
| [构建与运行指南](docs/BUILD_zh.md) | 从一台干净机器开始，一步步装好环境并跑起来；含 Win10 / Win11 差异与常见报错排查（[English](docs/BUILD_en.md)） |
| [源码导读](docs/codeReader.docx) | 25 页阅读地图：七站式阅读路线、核心机制、动手练习（[PDF](docs/codeReader.pdf) / [English](docs/codeReader_en.docx)） |

## 功能简介
- Vulkan 渲染后端
- GLFW 窗口管理与输入处理
- ImGui（Docking分支），用于调试UI与简易编辑器面板
- 简单支持**编辑器运行时修改实体、组件数据**
- Actor + Component 实体组件框架
- 简易反射系统：由独立 LLVM 反射代码生成器实现
- GLM 数学库，提供向量、矩阵运算
- stb_image 用于纹理图片加载
- cgltf 用于 glTF 模型解析加载
- 独立反射代码生成工具
  - 反射工具是一个独立项目
  - 仓库地址: [https://github.com/KomdSpecific/suzza-reflect-tool](https://github.com/KomdSpecific/suzza-reflect-tool)
  - 它的**源码**依赖外部 LLVM/Clang 库进行 AST 解析与代码生成
  - 本仓库已提交预编译好的生成器（`tool/ReflectGenerated/`），因此**编译本项目不需要安装 LLVM**

## 第三方依赖库
所有第三方库源码放置在 `thirdparty/` 目录内；为精简仓库体积，已移除示例、测试、文档等非必要文件。
- Dear ImGui（Docking分支，MIT协议）
- GLFW（zlib/libpng协议）
- GLM（MIT协议）
- stb_image（公有领域 / MIT 双协议）
- cgltf（MIT协议）

第三方库完整版权与许可声明，请查看 [`NOTICE.txt`](./NOTICE.txt) 文件。

## 编译依赖环境
- Windows 10 / 11（x64）
- **Visual Studio 2026 或 2022**（Community / Build Tools 均可），使用其中自带的 MSVC 编译器
- CMake 3.21+（VS 2026 自带 4.1，可直接用）与 Ninja
- Vulkan SDK 1.3.x（需包含 `glslc`）
- MSVC 是唯一验证过的工具链，**GCC / Clang 尚未测试**

## 编译步骤
请在 **x64 Native Tools Command Prompt**（VS 2026 叫 *... for VS*，VS 2022 叫 *... for VS 2022*）里执行。
必须使用 Ninja 生成器：反射代码生成步骤需要读取 `compile_commands.json`，而只有 Ninja / Makefile 这类生成器才会产出它。

```bat
git clone https://github.com/KomdSpecific/Suzza.git
cd Suzza
cmake -S . -B out/build/x64-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build out/build/x64-debug
```

产物为 `out/build/x64-debug/SuzzaEngine.exe`，`assets/` 会在构建结束后自动拷贝到旁边。

完整的安装步骤、环境自检脚本与故障速查表见 [docs/BUILD_zh.md](docs/BUILD_zh.md)。
