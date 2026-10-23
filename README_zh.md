[English](./README.md)

# Vulkan 小型3D引擎

一个基于 Vulkan 渲染后端、使用 C++ 编写的学习型小型3D渲染引擎。

> 本项目仅用于学习与研究，不适合商用。

## 功能简介
- Vulkan 渲染后端
- GLFW 窗口管理与输入处理
- ImGui（Docking分支），用于调试UI与简易编辑器面板
- GLM 数学库，提供向量、矩阵运算
- stb_image 用于纹理图片加载
- cgltf 用于 glTF 模型解析加载
- 独立反射代码生成工具
  - 反射工具是一个独立项目
  - 依赖外部 LLVM/Clang 库进行AST解析与代码生成
  - 本仓库**不包含 LLVM 源码**

## 第三方依赖库
所有第三方库源码放置在 `thirdparty/` 目录内；为精简仓库体积，已移除示例、测试、文档等非必要文件。
- Dear ImGui（Docking分支，MIT协议）
- GLFW（zlib/libpng协议）
- GLM（MIT协议）
- stb_image（公有领域 / MIT 双协议）
- cgltf（MIT协议）

第三方库完整版权与许可声明，请查看 [`NOTICE`](./NOTICE) 文件。

## 编译依赖环境
- C++20 及以上标准编译器
- CMake3.8+
- Vulkan SDK
- 支持 Vulkan 的编译器（MSVC / GCC / Clang）
- 反射工具：需要预先部署好外部 LLVM + Clang 开发库

## 编译步骤
```bash
# 示例命令
cmake -S . -B build
cmake --build build
