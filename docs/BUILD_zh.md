# SuzzaEngine 构建与运行指南（Windows 10 / 11）

> **适用对象**：第一次接触本项目，也第一次用 CMake + Vulkan 的人。
> **目标**：从一台干净的 Windows 机器，走到窗口里出现三个彩色立方体。
> **预计耗时**：40～60 分钟，其中大部分时间在下载安装包。
> **配套阅读**：[源码导读](codeReader.docx)（`docs/codeReader.docx`，另有 [中文 PDF](codeReader.pdf) 与 [英文版](codeReader_en.docx) / [英文 PDF](codeReader_en.pdf)）。

---

## 目录

1. [先读这一页](#1-先读这一页)
2. [依赖清单](#2-依赖清单)
3. [关于 LLVM / Clang：你可能被误导了](#3-关于-llvm--clang你可能被误导了)
4. [工具链支持矩阵](#4-工具链支持矩阵)
5. [分步安装](#5-分步安装)
6. [一次成功的构建](#6-一次成功的构建)
7. [三个必知的构建细节](#7-三个必知的构建细节)
8. [跑起来之后应该看到什么](#8-跑起来之后应该看到什么)
9. [环境自检脚本](#9-环境自检脚本)
10. [常见问题速查](#10-常见问题速查)
11. [Windows 10 与 Windows 11 的差异](#11-windows-10-与-windows-11-的差异)
12. [清理与重来](#12-清理与重来)
13. [附录](#13-附录)

---

## 1. 先读这一页

这个项目**不是**双击 `.sln` 就能编译的那种工程。它靠 CMake 驱动，并且在构建过程中会自动跑三个额外步骤（编译着色器、生成反射代码、拷贝资源）。第一次构建失败，99% 的原因是下面几件事之一：

| 硬性前提 | 不满足时的典型现象 |
| --- | --- |
| Windows x64 + MSVC 编译器 | `No CMAKE_CXX_COMPILER could be found` |
| 用 **Ninja**（或 Makefile）作为 CMake 生成器 | 反射生成步骤报找不到 `compile_commands.json` |
| 已安装 Vulkan SDK，且 `glslc.exe` 在 PATH 上 | `Could NOT find Vulkan` 或着色器编译失败 |
| 显卡驱动支持 Vulkan 1.2 + `VK_EXT_descriptor_indexing` | 程序启动后卡住、验证层报扩展不支持、画面全黑 |

**明确不需要的东西**（新手最容易多装）：

- ❌ LLVM / Clang 编译器（原因见第 3 节）
- ❌ vcpkg / Conan 等包管理器（第三方库已经放在 `thirdparty/` 里）
- ❌ Visual Studio 的 `.sln` 工程文件（用不到，也不要用 VS 的 CMake 生成器）
- ❌ Python、Node.js、.NET SDK

---

## 2. 依赖清单

| 组件 | 建议版本 | 作用 | 缺失后果 | 验证命令 |
| --- | --- | --- | --- | --- |
| Windows | 10 21H2+ / 11，x64 | — | — | `winver` |
| Visual Studio | **2026**（官网默认，推荐）或 2022；Community 或 Build Tools | 提供 `cl.exe`、Windows SDK、Ninja | 无法配置 CMake | `cl` 应打印版本 |
| MSVC 工具集 | v145（14.50 / 14.51，VS 2026）或 v143（14.38 / 14.44，VS 2022） | C++20 编译器 | 编译失败 | 见第 5.1 节 |
| CMake | **≥ 3.21**（建议 3.27+） | 构建系统 | 预设文件无法解析 | `cmake --version` |
| Ninja | VS 自带即可 | 实际执行编译 | `ninja: command not found` | `ninja --version` |
| Vulkan SDK | 1.3.x（实测 1.3.296.0） | `vulkan-1.lib`、头文件、`glslc.exe` | `Could NOT find Vulkan` | `glslc --version` |
| 显卡驱动 | 支持 Vulkan 1.2 | 运行时渲染 | 启动失败 / 黑屏 | `vulkaninfo --summary` |
| Git | 任意 | 拉取源码（用 ZIP 也行） | — | `git --version` |
| 磁盘 / 内存 | 1 GB / 8 GB | 构建产物约 140 MB | — | — |

需要 CMake **3.21 及以上**。VS 2026 自带的是 **CMake 4.1**，对项目里写的 `3.8` 只会给一条 deprecation 警告（CMake 4.x 只对 < 3.5 报错），可以忽略。

---

## 3. 关于 LLVM / Clang：你可能被误导了

README 里写着"反射工具依赖外部 LLVM/Clang 库"，这句话说的是 **反射生成器这个独立项目**（[suzza-reflect-tool](https://github.com/KomdSpecific/suzza-reflect-tool)）的源码依赖，不是本仓库的使用前提。

本仓库把生成器**已经编译好**放在 `tool/ReflectGenerated/GENERATED_REFLECT_CODE_PROGRAM.exe`（39 MB，x64）。实测它的 DLL 导入表只有：

```
KERNEL32.dll  MSVCP140.dll  VCRUNTIME140_1.dll  api-ms-win-crt-*.dll ...
```

也就是说 **LLVM 是静态链接进去的，运行时不加载任何 LLVM DLL**。你不需要安装 LLVM，也不需要 clang 编译器。

什么时候才真的需要 LLVM 18.x：

| 场景 | 需要 LLVM 吗 |
| --- | --- |
| 只想把项目编译、运行起来 | ❌ 不需要 |
| 改引擎代码、加组件、加字段 | ❌ 不需要（反射代码会自动重新生成） |
| 想改反射生成器本身 / 自己从源码构建它 | ✅ 需要 LLVM + Clang 开发包 |

> 你手上如果有 LLVM 18.1.8，留着即可，本项目用不到；它也不会影响构建。

---

## 4. 工具链支持矩阵

| 工具链 / 预设 | 状态 | 说明 |
| --- | --- | --- |
| MSVC v143 + Ninja（VS 2022，`x64-debug`） | ✅ 已验证 | 作者本机走过的路径 |
| MSVC v143 + Ninja（VS 2022，`x64-release`） | ✅ 可用 | 同上，只是优化等级不同 |
| MSVC v145 + Ninja（VS 2026，`x64-debug`） | ⚠️ 未实测 | 只用标准 C++20 + Win32，理论上无差异；2026 的 STL 会让生成器多打一条 STL1000（见 10.2） |
| Clang / clang-cl | ⚠️ 未验证 | README 明确写"仅验证 MSVC"；反射宏在 clang 下会走 `annotate` 分支，行为未测试 |
| GCC / MinGW | ⚠️ 未验证 | 未测试，遇到问题请自行排查 |
| `x86-debug` / `x86-release` | ⚠️ 未验证 | 生成器是 x64 程序，暂不支持 32 位宿主 |
| Linux / macOS 预设 | ❌ 实际不可用 | 预设存在，但反射生成器是 Windows PE 文件，Linux 上无法执行 |

**结论**：老老实实用 **MSVC + Ninja + x64**，这是唯一被走过一遍的路径。

---

## 5. 分步安装

### 5.1 安装 Visual Studio（2026 或 2022 都行）

> ℹ️ 从 2025 年 11 月起，微软官网默认只提供 **Visual Studio 2026**（v18.x，MSVC 14.50 / 14.51）。本项目只用标准 C++20 与 Win32 API，**2026 和 2022 都能编译**；2022 仍可安装，只是入口被挪到了"旧版本"区域，新手不一定找得到。**已经装了哪个就用哪个，不必为了这份教程回退版本。**

先确认自己装的是哪一代——在开发者命令行里执行 `cl`，看版本号：

| 你装的 VS | `cl` 版本 | 工具集 | 默认安装目录 | 开始菜单里的命令提示符 |
| --- | --- | --- | --- | --- |
| Visual Studio 2026 | 19.50 / 19.51 | v145（14.50 / 14.51） | `C:\Program Files\Microsoft Visual Studio\18\Community` | **Visual Studio** → *x64 Native Tools Command Prompt for VS* |
| Visual Studio 2022 | 19.3x / 19.4x | v143（14.38 / 14.44） | `C:\Program Files\Microsoft Visual Studio\2022\Community` | **Visual Studio 2022** → *x64 Native Tools Command Prompt for VS 2022* |

#### 路径 A：Visual Studio 2026（官网默认，新机器推荐）

1. 打开 [Visual Studio 下载页](https://visualstudio.microsoft.com/downloads/)，下载 **Community 2026**（个人免费）。只要编译器、不要 IDE 的话，在页面下方 **Tools for Visual Studio** 区域下载 **Build Tools for Visual Studio 2026**。
2. 安装器里勾选工作负载 **"使用 C++ 的桌面开发"**（Desktop development with C++）。
3. 右侧"安装详细信息"确认这三项被勾上：
   - **MSVC v145 - VS 2026 C++ x64/x86 生成工具**（2026 起安装器按 MSVC 版本号命名，看到 v145 / 14.50 就是它）
   - **Windows 11 SDK**（或 Windows 10 SDK，二选一即可）
   - **C++ CMake tools for Windows**（可选，会顺带装一份 CMake 4.1 和 Ninja）
4. 装完后从开始菜单打开 **Visual Studio → x64 Native Tools Command Prompt for VS**（2026 起不再带年份后缀；注意选 **x64 Native Tools**，不是普通 cmd 或 PowerShell）。

#### 路径 B：Visual Studio 2022（已经装了就直接用）

1. 官网首页已主推 2026；2022 的安装包在下载页的**旧版本（Previous versions）**区域，或登录 [my.visualstudio.com](https://my.visualstudio.com/downloads) 获取。
2. 工作负载与上面相同，只是组件名换成 **MSVC v143 - VS 2022 C++ x64/x86 生成工具**。
3. 开始菜单里的入口叫 **x64 Native Tools Command Prompt for VS 2022**。

两条路径的验证方式一样：

```bat
cl
:: VS 2026 期望：Microsoft (R) C/C++ Optimizing Compiler Version 19.50.xxxxx for x64（或 19.51）
:: VS 2022 期望：... Version 19.38 / 19.44 ... for x64
where ninja
:: 期望：...\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe
```

> ⚠️ **不要混用两代 VS**：机器上同时装了 2026 和 2022 时，必须用对应版本的开发者命令提示符。想手动指定工具集，用 `vcvarsall.bat x64 -vcvars_ver=14.51`（VS 2026）或 `-vcvars_ver=14.44`（VS 2022）。
>
> 如果 `cl` 提示"不是内部或外部命令"，说明你开的不是 VS 开发者命令行，或者 C++ 工作负载没装成功。

### 5.2 安装 CMake

两种方式，任选其一：

**方式 A（推荐，最省事）**：直接用 VS 自带的 CMake。在"x64 Native Tools Command Prompt"里 `cmake --version` 能打印版本即可（VS 2022 自带 3.2x，VS 2026 自带 **4.1**，都满足要求）。

**方式 B**：从 [cmake.org/download](https://cmake.org/download/) 下载 Windows x64 Installer，安装时选择 **Add CMake to the system PATH**。

验证：

```bat
cmake --version
:: 必须 >= 3.21，否则 CMakePresets.json 会报 unsupported
```

### 5.3 安装 Vulkan SDK

1. 打开 [LunarG Vulkan SDK 下载页](https://vulkan.lunarg.com/sdk/home#windows)，下载 **Windows** 版本（1.3.x，实测 1.3.296.0 可用）。
2. 安装时保持默认选项。安装器会自动：
   - 设置系统环境变量 `VULKAN_SDK`
   - 把 `%VULKAN_SDK%\Bin` 加入 PATH（`glslc.exe`、`vulkaninfo.exe` 都在这里）
   - 安装 `vulkan-1.lib` 与头文件，供 CMake 的 `find_package(Vulkan)` 定位
3. **关掉所有已经打开的终端，重新开一个**（Windows 10 上环境变量不会自动刷新，这是最常见的"我明明装了啊"）。

验证：

```bat
echo %VULKAN_SDK%
:: 期望：C:\VulkanSDK\1.3.296.0 之类的路径

glslc --version
:: 期望：shaderc v2023.x ... target: spirv1.0

vulkaninfo --summary
:: 期望：能看到 GPU 名字、apiVersion 1.3.x，并且没有报错
```

`vulkaninfo` 报错时，先更新显卡驱动（NVIDIA / AMD / Intel 官网的正式版驱动，别用 Windows Update 里的旧版）。Vulkan 运行时随驱动提供，不需要单独装。

### 5.4 获取源码

```bat
git clone https://github.com/KomdSpecific/Suzza.git
cd Suzza
```

或者直接在 GitHub 页面点 **Code → Download ZIP** 解压（没有 `.git` 也能正常构建）。

> ⚠️ 仓库的 `.gitignore` 忽略了 `CMakePresets.json`、`tool/ReflectGenerated/generated/`、`assets/model/pony_cartoon/` 等路径。所以从 GitHub 克隆下来的干净副本里，**这几个东西可能是缺失的**——第 7 节会讲怎么处理，不属于错误。

---

## 6. 一次成功的构建

以下命令全部在 **"x64 Native Tools Command Prompt"** 里执行（VS 2026 的入口叫 *... for VS*，VS 2022 叫 *... for VS 2022*）：

```bat
:: 1) 进入仓库根目录（有 CMakeLists.txt 的那一层）
cd /d D:\path\to\SuzzaEngine

:: 2) 配置：生成器必须是 Ninja
cmake -S . -B out\build\x64-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug

:: 3) 构建（首次大概几分钟，之后增量通常十几秒）
cmake --build out\build\x64-debug

:: 4) 运行
out\build\x64-debug\SuzzaEngine.exe
```

> 如果你手上的副本**有** `CMakePresets.json`（作者本机就有），第 2、3 步可以简写成：
>
> ```bat
> cmake --preset x64-debug
> cmake --build --preset x64-debug
> ```
>
> 两种写法**完全等价**，preset 展开后就是上面那两条命令。没有 preset 文件时用显式命令即可。

构建产物都在 `out\build\x64-debug\` 下：

| 路径 | 内容 |
| --- | --- |
| `SuzzaEngine.exe` | 主程序（约 4 MB，静态链接 CRT） |
| `assets\` | 构建结束时自动从仓库根拷贝过来的资源（约 22 MB） |
| `shaders\*.spv` | 由 `glslc` 编译出来的着色器字节码 |
| `compile_commands.json` | 供反射生成器读取的编译数据库 |
| `build.ninja`、`.ninja_log` | Ninja 的构建脚本与日志 |
| 整个目录 | 约 140 MB（Debug，含 PDB） |

`Release` 版只需把命令里的 `x64-debug` / `Debug` 换成 `x64-release` / `Release`，产物各自独立，互不覆盖。

---

## 7. 三个必知的构建细节

### 7.1 `CMakePresets.json` 可能根本不在仓库里

`.gitignore` 里有一行 `CMakePresets.json`，所以它**没有被 Git 跟踪**。你在新机器上克隆下来的副本里，`cmake --preset x64-debug` 很可能报：

```
CMake Error: No such preset in ...: x64-debug
```

两种解决办法：

- **推荐**：用第 6 节的显式命令，不依赖 preset 文件。
- 或者自己把 preset 文件补回来：内容见 [附录 A](#附录-acmakepresetsjson-原文)。

### 7.2 生成器必须是 Ninja（或 Makefile），不能用 Visual Studio 生成器

反射生成器的输入之一是 `compile_commands.json`（`CMakeLists.txt:115`），而这个文件**只有 Ninja / Makefile 这类生成器才会产出**。项目通过 `CMAKE_EXPORT_COMPILE_COMMANDS ON`（`CMakeLists.txt:15`）打开它。

如果你用 `cmake -G "Visual Studio 17 2022"`（VS 2022）或 `-G "Visual Studio 18 2026"`（VS 2026）：

- 不会生成 `compile_commands.json` → 反射步骤的依赖缺失；
- 要么直接报错，要么跳过生成、继续用 `tool/ReflectGenerated/generated/` 里的**旧文件**（结果就是改了字段但编辑器里看不到，非常难查）。

### 7.3 构建会自动做三件"看不见的事"

| 自动步骤 | 位置 | 产物 |
| --- | --- | --- |
| 编译着色器 | `CMakeLists.txt:67-109`，调用 `Vulkan::glslc` | `out/build/<preset>/shaders/*.spv` |
| 生成反射代码 | `CMakeLists.txt:112-135`，运行仓库内的生成器 exe | `tool/ReflectGenerated/generated/*.generated.{h,cpp}` |
| 拷贝资源目录 | `CMakeLists.txt:207-211`，`POST_BUILD` | `<exe 目录>/assets/` |

两个容易踩的点：

1. **生成器会把本机的绝对路径写进生成文件**（比如 `"D:/LjhUser/.../src/scene/Actor.h"`）。这是正常现象，只影响本机，不用改。
2. **生成目录在干净克隆里可能不存在**。实测生成器会自己创建目录，不需要手动 `mkdir`；万一你的版本报找不到目录，手动建一个空目录即可：

   ```bat
   mkdir tool\ReflectGenerated\generated
   ```

---

## 8. 跑起来之后应该看到什么

| 现象 | 说明 |
| --- | --- |
| 一个 1600×1200 的窗口，标题 `First Window` | 窗口创建成功 |
| 画面里有三个彩色立方体，呈父子层级摆放 | `SuzzaApplication::Init` 里的示例场景 |
| ImGui 面板：`Hierarchy` 与 `Inspector` | 编辑器已启动（非生产构建默认开启 `EDITOR` 宏） |
| 鼠标左键拖动可以转视角 | `MovementComponent` 的鼠标视角 |
| `W` `A` `S` `D` 可以移动 | `MovementComponent` 的位移 |
| 关闭窗口即退出，控制台无报错 | 正常结束 |

> 默认场景里的两个材质都是 `new Material()`，**没有传入贴图路径**，因此不会上传任何纹理，立方体呈纯色是正常现象，不是资源没拷过去。

---

## 9. 环境自检脚本

在 PowerShell 里整段粘贴运行，逐项打印 OK / FAIL：

```powershell
$checks = [ordered]@{
  'CMake >= 3.21' = { (cmake --version | Select-Object -First 1) -match '3\.(2[1-9]|[3-9]\d)' }
  'Ninja'         = { $null -ne (Get-Command ninja -ErrorAction SilentlyContinue) }
  'MSVC cl.exe'   = { $null -ne (Get-Command cl -ErrorAction SilentlyContinue) }
  'VULKAN_SDK'    = { $env:VULKAN_SDK -and (Test-Path "$env:VULKAN_SDK\Bin\glslc.exe") }
  'glslc'         = { $null -ne (Get-Command glslc -ErrorAction SilentlyContinue) }
  'vulkaninfo'    = { $null -ne (Get-Command vulkaninfo -ErrorAction SilentlyContinue) }
  'Git'           = { $null -ne (Get-Command git -ErrorAction SilentlyContinue) }
}
foreach ($k in $checks.Keys) {
  try { $ok = & $checks[$k] } catch { $ok = $false }
  '{0,-16} {1}' -f $k, $(if ($ok) { 'OK' } else { 'FAIL' })
}
```

（开始菜单：VS 2026 是 *Developer PowerShell for VS*，VS 2022 是 *Developer PowerShell for VS 2022*）

---

## 10. 常见问题速查

### 10.1 配置阶段

| 你看到的报错 | 原因 | 处理 |
| --- | --- | --- |
| `Could NOT find Vulkan (missing: Vulkan_LIBRARY Vulkan_INCLUDE_DIR)` | Vulkan SDK 没装 / `VULKAN_SDK` 未生效 | 重开终端；确认 `%VULKAN_SDK%\Lib\vulkan-1.lib` 存在 |
| `No CMAKE_CXX_COMPILER could be found` | 不在 VS 开发者命令行里 | 用 "x64 Native Tools Command Prompt" 重开 |
| `No such preset in ...: x64-debug` | `CMakePresets.json` 被 gitignore | 改用显式命令，或见附录 A |
| `CMake 3.x is too old` / `preset version 3 unsupported` | CMake < 3.21 | 升级 CMake |
| `ninja: command not found` | Ninja 不在 PATH | 用 VS 开发者命令行，或单独安装 Ninja 并加 PATH |

### 10.2 构建阶段

| 你看到的报错 | 原因 | 处理 |
| --- | --- | --- |
| `ninja: error: ... compile_commands.json` / 反射步骤报文件不存在 | 用了 VS 生成器，或配置阶段失败 | 删掉 `out\build\x64-debug` 用 Ninja 重新配置 |
| `STL1000: static assertion failed: expected Clang 19.0.0 or newer` | 生成器内置 clang 18；从 VS 2022 17.10 起（含 VS 2026 的 14.50 / 14.51）STL 要求 clang 19+ | 无害：生成器退出码仍为 0，反射产物正确；想要干净日志见下方说明 |
| `无法打开源文件 "xxx.generated.h"` | 反射生成步骤没跑成功 | 确认 `tool\ReflectGenerated\generated\*.generated.h` 是否存在 |
| 链接错误 `unresolved external symbol`（新增了 .cpp 之后） | `GLOB_RECURSE` 未刷新 | 重新执行 `cmake -S . -B out\build\x64-debug -G Ninja` |
| 中文注释乱码 | 源文件是 **GBK** 编码，编辑器按 UTF-8 打开 | 把编辑器设为 GBK / GB2312 打开；**不要**随手转成 UTF-8 保存，否则注释会变乱码（虽然仍能编译） |
| `MSVCP140.dll` / `VCRUNTIME140_1.dll` 缺失 | 没装 VC++ 运行库（反射生成器需要） | 安装 [VC++ 2015-2022 运行库 x64](https://aka.ms/vs/17/release/vc_redist.x64.exe)（装过 VS 的话本来就有） |

关于 `STL1000` 的补充（**在 VS 2026 上大概率会见到**）：2026 默认的 14.50 / 14.51 STL 对编译器版本检查更严，这条断言会稳定出现，但**不影响构建结果**——实测生成器退出码为 0，产物与 14.38 工具集逐字节相同。只有当你确实想要一条干净日志时，才需要考虑换旧工具集：

**VS 2022**：Installer → 单个组件 → 勾选 *MSVC v143 - VS 2022 C++ x64/x86 生成工具 (v14.38-17.8)*，然后在开发者命令行里执行：

```bat
call "<VS2022 安装目录>\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.38
cl
:: 版本号回到 19.38 才算生效
```

**VS 2026**：安装器的"单个组件"里通常还能并存旧工具集（名称以实际列表为准），同样用 `-vcvars_ver=14.38` 指定；如果你的安装器里找不到该组件，直接忽略这条断言即可。

### 10.3 运行阶段

| 现象 | 可能原因 | 处理 |
| --- | --- | --- |
| 双击 exe 一闪而过 | 非生产构建带控制台，异常时窗口会关闭 | 在终端里运行 exe 看输出；日志宏 `SUZZA_LOG_ERROR` 会打印中文错误 |
| 窗口出来但画面全黑 | 验证层报描述符 / 管线错误；相机矩阵异常 | 看控制台；确认显卡驱动支持 `VK_EXT_descriptor_indexing` |
| 启动即崩溃、验证层报扩展不支持 | 显卡/驱动太旧 | 更新驱动；或换一台支持 Vulkan 1.2 的机器 |
| 提示找不到 assets | 直接拷贝了 exe 而没带 `assets` 目录 | 整个 `out\build\x64-debug` 目录一起用，或把 `assets` 放到 exe 同级 |
| 想跑"加载 pony_cartoon"的练习却失败 | `assets/model/*` 被 `.gitignore` 忽略，克隆里没有 | 需要模型文件请向作者索取，或自行下载 glTF 放到 `assets/model/pony_cartoon/` |

---

## 11. Windows 10 与 Windows 11 的差异

| 方面 | Windows 10 | Windows 11 |
| --- | --- | --- |
| 选 2022 还是 2026 | 很老的 Win10（1909 及更早）建议装 VS 2022 | 两者都能装，新机器按官网默认选 2026 |
| Windows SDK | 装 10.0.19041 及以上 | 同上 |
| 环境变量刷新 | 装完 Vulkan SDK **必须重开终端**，否则 `VULKAN_SDK` 读不到 | 同上，同样建议重开 |
| 终端 | 默认为 cmd / PowerShell，建议手动用 "x64 Native Tools Command Prompt" | 默认 Windows Terminal，**仍然要选 VS 开发者命令行**，不要用普通 PowerShell |
| 长路径 | 需手动开启（组策略 / 注册表 `LongPathsEnabled`） | 默认开启 |
| 首次构建速度 | 受 Windows Defender 实时扫描影响明显 | 同上，可把仓库目录加入排除项 |
| 显卡驱动 | 注意别用 Windows Update 推的旧驱动 | 同上 |
| WSL2 / Hyper-V | 与 Vulkan 无冲突 | 无冲突 |

**通用建议**：把仓库放到**非系统盘、路径不含中文和空格**的位置（例如 `D:\Code\SuzzaEngine`）。虽然 CMake 一般能处理空格，但反射生成器会把路径写进生成代码，路径越简单越省事。

---

## 12. 清理与重来

构建产物全部集中在 `out\`，删掉即可完全重来：

```bat
rmdir /s /q out
cmake -S . -B out\build\x64-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build out\build\x64-debug
```

如果想连反射生成物一起清掉（让生成器重新生成）：

```bat
rmdir /s /q tool\ReflectGenerated\generated
```

> 这两个目录都在 `.gitignore` 里，删除不会影响 `git status`。

---

## 13. 附录

### 附录 A：`CMakePresets.json` 原文

如果克隆里缺这个文件，可以原样保存到仓库根目录（与 `CMakeLists.txt` 同级）。内容与作者本机的 Windows 部分一致：

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "windows-base",
            "hidden": true,
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/out/build/${presetName}",
            "installDir": "${sourceDir}/out/install/${presetName}",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "cl.exe",
                "CMAKE_CXX_COMPILER": "cl.exe"
            },
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Windows"
            }
        },
        {
            "name": "x64-debug",
            "displayName": "x64 Debug",
            "inherits": "windows-base",
            "architecture": { "value": "x64", "strategy": "external" },
            "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug" }
        },
        {
            "name": "x64-release",
            "displayName": "x64 Release",
            "inherits": "x64-debug",
            "cacheVariables": { "CMAKE_BUILD_TYPE": "Release" }
        }
    ],
    "buildPresets": [
        { "name": "x64-debug", "configurePreset": "x64-debug" },
        { "name": "x64-release", "configurePreset": "x64-release" }
    ]
}
```

> 注意：原作者仓库里的 `CMakePresets.json` 还包含 `x86-*`、`linux-*`、`macos-*` 预设，这里省略了，因为本项目的反射生成器只有 Windows x64 版本。

### 附录 B：preset 命令与显式命令对照

| 用 preset | 等价的显式命令 |
| --- | --- |
| `cmake --preset x64-debug` | `cmake -S . -B out/build/x64-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug` |
| `cmake --build --preset x64-debug` | `cmake --build out/build/x64-debug` |
| `cmake --preset x64-release` | `cmake -S . -B out/build/x64-release -G Ninja -DCMAKE_BUILD_TYPE=Release` |

### 附录 C：与构建有关的目录

```
SuzzaEngine/
├─ CMakeLists.txt                  ← 顶层构建脚本（着色器 / 反射 / 资源拷贝都在这里）
├─ CMakePresets.json               ← 可能被 gitignore，缺失属正常
├─ config.h.in                     ← 生成 shared/config.h（注入资源路径宏）
├─ shaders/*.vert|.frag            ← 着色器源码，构建时编译成 .spv
├─ assets/                         ← 运行时资源，构建后整体拷到 exe 旁
├─ thirdparty/                     ← 已内置的第三方库，无需包管理器
├─ shared/  src/  tool/            ← 引擎与工具源码
├─ tool/ReflectGenerated/
│  ├─ GENERATED_REFLECT_CODE_PROGRAM.exe   ← 预编译的反射生成器（39 MB）
│  └─ generated/                           ← 它生成的代码，可能不存在
└─ out/build/<preset>/             ← 构建产物（可随时删除）
```

### 附录 D：术语表

| 术语 | 一句话解释 |
| --- | --- |
| CMake 生成器 | 决定 CMake 产出哪种构建脚本；本项目必须用 Ninja |
| `compile_commands.json` | 每个 .cpp 的完整编译命令清单，反射生成器的输入 |
| 着色器（shader）/ SPIR-V | GPU 上跑的程序；`glslc` 把 `.vert/.frag` 编译成 `.spv` |
| 反射（reflection） | 让程序在运行时知道"某个类有哪些字段"，Inspector 面板靠它工作 |
| 验证层（validation layer） | Vulkan 的调试检查，出错时会打印详细原因 |
| bindless | 把很多贴图塞进一个描述符数组，用下标访问，本项目用来省描述符绑定 |

---

*文档如与代码不一致，以仓库中的 `CMakeLists.txt` 为准。*
