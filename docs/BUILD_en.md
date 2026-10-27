# SuzzaEngine Build & Run Guide (Windows 10 / 11)

> **Who this is for**: someone who has never touched this project, and has never used CMake + Vulkan before.
> **Goal**: go from a clean Windows machine to three coloured cubes in a window.
> **Time needed**: 40–60 minutes, most of it downloading installers.
> **Companion document**: [source reading guide](codeReader_en.docx) (`docs/codeReader_en.docx`, also available as [PDF](codeReader_en.pdf); Chinese edition: [docx](codeReader.docx) / [PDF](codeReader.pdf)).

---

## Contents

1. [Read this page first](#1-read-this-page-first)
2. [Dependency checklist](#2-dependency-checklist)
3. [About LLVM / Clang: you may have been misled](#3-about-llvm--clang-you-may-have-been-misled)
4. [Toolchain support matrix](#4-toolchain-support-matrix)
5. [Step-by-step installation](#5-step-by-step-installation)
6. [A build that works](#6-a-build-that-works)
7. [Three build details you must know](#7-three-build-details-you-must-know)
8. [What you should see after launching](#8-what-you-should-see-after-launching)
9. [Environment self-check script](#9-environment-self-check-script)
10. [Troubleshooting](#10-troubleshooting)
11. [Windows 10 vs Windows 11](#11-windows-10-vs-windows-11)
12. [Clean up and start over](#12-clean-up-and-start-over)
13. [Appendices](#13-appendices)

---

## 1. Read this page first

This is **not** a project you can build by double-clicking an `.sln`. It is driven by CMake, and the build automatically runs three extra steps (compile shaders, generate reflection code, copy assets). When a first build fails, it is almost always one of these:

| Hard requirement | Typical symptom when missing |
| --- | --- |
| Windows x64 + MSVC compiler | `No CMAKE_CXX_COMPILER could be found` |
| **Ninja** (or Makefile) as the CMake generator | the reflection step cannot find `compile_commands.json` |
| Vulkan SDK installed, `glslc.exe` on PATH | `Could NOT find Vulkan`, or shader compilation fails |
| GPU driver with Vulkan 1.2 + `VK_EXT_descriptor_indexing` | the app hangs, validation layers complain, screen stays black |

**Things you explicitly do NOT need** (beginners over-install these):

- ❌ LLVM / Clang compilers (see section 3)
- ❌ vcpkg / Conan or any package manager (third-party libraries are vendored in `thirdparty/`)
- ❌ A Visual Studio `.sln` project file (not used; also do not pick the VS CMake generator)
- ❌ Python, Node.js, .NET SDK

---

## 2. Dependency checklist

| Component | Version | Why it is needed | If missing | Check with |
| --- | --- | --- | --- | --- |
| Windows | 10 21H2+ / 11, x64 | — | — | `winver` |
| Visual Studio | **2026** (the current default, recommended) or 2022; Community or Build Tools | provides `cl.exe`, Windows SDK, Ninja | CMake cannot configure | `cl` prints a version |
| MSVC toolset | v145 (14.50 / 14.51, VS 2026) or v143 (14.38 / 14.44, VS 2022) | C++20 compiler | compilation fails | see 5.1 |
| CMake | **>= 3.21** (3.27+ recommended) | build system | presets cannot be parsed | `cmake --version` |
| Ninja | whatever ships with VS | actually runs the build | `ninja: command not found` | `ninja --version` |
| Vulkan SDK | 1.3.x (1.3.296.0 verified) | `vulkan-1.lib`, headers, `glslc.exe` | `Could NOT find Vulkan` | `glslc --version` |
| GPU driver | supports Vulkan 1.2 | runtime rendering | launch failure / black screen | `vulkaninfo --summary` |
| Git | any | clone the repo (a ZIP works too) | — | `git --version` |
| Disk / RAM | 1 GB / 8 GB | build tree is about 140 MB | — | — |

which needs CMake **3.21 or newer**. VS 2026 bundles **CMake 4.1**, which only prints a deprecation warning for the `3.8` in the project (CMake 4.x errors below 3.5) - ignore it.

---

## 3. About LLVM / Clang: you may have been misled

The README says "the reflection tool depends on external LLVM/Clang libraries". That sentence describes the **standalone generator project** ([suzza-reflect-tool](https://github.com/KomdSpecific/suzza-reflect-tool)), not a requirement for using this repository.

This repository ships the generator **already compiled** at `tool/ReflectGenerated/GENERATED_REFLECT_CODE_PROGRAM.exe` (39 MB, x64). Its DLL import table contains only:

```
KERNEL32.dll  MSVCP140.dll  VCRUNTIME140_1.dll  api-ms-win-crt-*.dll ...
```

In other words **LLVM is statically linked in; no LLVM DLL is loaded at runtime**. You do not need LLVM, and you do not need a clang compiler.

When you *would* need LLVM 18.x:

| Scenario | LLVM needed? |
| --- | --- |
| Just build and run the project | ❌ no |
| Edit engine code, add components or fields | ❌ no (reflection code is regenerated automatically) |
| Modify the reflection generator itself / build it from source | ✅ yes, LLVM + Clang development packages |

> If you already have LLVM 18.1.8 installed, keep it; this project will not use it and it will not break anything.

---

## 4. Toolchain support matrix

| Toolchain / preset | Status | Notes |
| --- | --- | --- |
| MSVC v143 + Ninja (VS 2022, `x64-debug`) | ✅ verified | walked through by the author |
| MSVC v143 + Ninja (VS 2022, `x64-release`) | ✅ usable | same, different optimisation level |
| MSVC v145 + Ninja (VS 2026, `x64-debug`) | ⚠️ untested | standard C++20 + Win32 only, so it should behave the same; the 2026 STL adds one extra STL1000 message (see 10.2) |
| Clang / clang-cl | ⚠️ untested | the README states only MSVC is verified; reflection macros take the `annotate` branch under clang, behaviour untested |
| GCC / MinGW | ⚠️ untested | not tested, debug it yourself if you go this way |
| `x86-debug` / `x86-release` | ⚠️ untested | the generator is an x64 binary, 32-bit hosts are not supported |
| Linux / macOS presets | ❌ unusable in practice | the presets exist, but the reflection generator is a Windows PE file |

**Bottom line**: use **MSVC + Ninja + x64**. That is the only path that has actually been walked.

---

## 5. Step-by-step installation

### 5.1 Install Visual Studio (2026 or 2022, either works)

> ℹ️ From November 2025 the Microsoft download page ships **Visual Studio 2026** (v18.x, MSVC 14.50 / 14.51) by default. This project only uses standard C++20 and Win32 APIs, so **both 2026 and 2022 build it**; 2022 is still installable but has moved to the "previous versions" area, where beginners may not find it. **Use whichever you already have - do not downgrade because of this guide.**

First find out which generation you have by running `cl` in a developer prompt:

| Your VS | `cl` version | Toolset | Default install dir | Start-menu prompt |
| --- | --- | --- | --- | --- |
| Visual Studio 2026 | 19.50 / 19.51 | v145 (14.50 / 14.51) | `C:\Program Files\Microsoft Visual Studio\18\Community` | **Visual Studio** -> *x64 Native Tools Command Prompt for VS* |
| Visual Studio 2022 | 19.3x / 19.4x | v143 (14.38 / 14.44) | `C:\Program Files\Microsoft Visual Studio\2022\Community` | **Visual Studio 2022** -> *x64 Native Tools Command Prompt for VS 2022* |

#### Path A: Visual Studio 2026 (the current default, recommended for a new machine)

1. Open the [Visual Studio downloads page](https://visualstudio.microsoft.com/downloads/) and get **Community 2026** (free for individuals). If you only want a compiler, take **Build Tools for Visual Studio 2026** from the **Tools for Visual Studio** section further down the page.
2. In the installer select the workload **"Desktop development with C++"**.
3. On the right, make sure these are ticked:
   - **MSVC v145 - VS 2026 C++ x64/x86 build tools** (since 2026 the installer names packages by MSVC version; v145 / 14.50 is the one)
   - **Windows 11 SDK** (or the Windows 10 SDK, either is fine)
   - **C++ CMake tools for Windows** (optional; brings CMake 4.1 and Ninja along)
4. When it finishes, open **Visual Studio -> x64 Native Tools Command Prompt for VS** from the Start menu (no year suffix since 2026). It must be the **x64 Native Tools** prompt, not a plain `cmd` or PowerShell.

#### Path B: Visual Studio 2022 (already installed - just use it)

1. The download page now promotes 2026; the 2022 installer lives under **Previous versions**, or grab it from [my.visualstudio.com](https://my.visualstudio.com/downloads).
2. Same workload and components, except the component is named **MSVC v143 - VS 2022 C++ x64/x86 build tools**.
3. The Start-menu entry is **x64 Native Tools Command Prompt for VS 2022**.

Verify either way:

```bat
cl
:: VS 2026: Microsoft (R) C/C++ Optimizing Compiler Version 19.50.xxxxx for x64 (or 19.51)
:: VS 2022: ... Version 19.38 / 19.44 ... for x64
where ninja
:: expect: ...\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe
```

> ⚠️ **Never mix the two generations**: if 2026 and 2022 are both installed, use the prompt that belongs to the version you build with. To pin a toolset manually use `vcvarsall.bat x64 -vcvars_ver=14.51` (VS 2026) or `-vcvars_ver=14.44` (VS 2022).
>
> If `cl` is "not recognized", you are not in a VS developer prompt, or the C++ workload did not install.

### 5.2 Install CMake

Two options, pick one:

**Option A (recommended)**: use the CMake that comes with Visual Studio. Inside the "x64 Native Tools Command Prompt", `cmake --version` should already print a version (VS 2022 ships 3.2x, VS 2026 ships **4.1** - both are fine).

**Option B**: download the Windows x64 installer from [cmake.org/download](https://cmake.org/download/) and tick **Add CMake to the system PATH**.

Verify:

```bat
cmake --version
:: must be >= 3.21, otherwise CMakePresets.json reports "unsupported"
```

### 5.3 Install the Vulkan SDK

1. Open the [LunarG Vulkan SDK download page](https://vulkan.lunarg.com/sdk/home#windows) and download the **Windows** package (1.3.x; 1.3.296.0 verified).
2. Keep the default installer options. It will:
   - set the `VULKAN_SDK` environment variable,
   - add `%VULKAN_SDK%\Bin` to PATH (`glslc.exe` and `vulkaninfo.exe` live there),
   - install `vulkan-1.lib` and the headers that `find_package(Vulkan)` needs.
3. **Close every terminal that was already open and start a new one.** On Windows 10 environment variables are not refreshed automatically; this is the most common "but I installed it!" moment.

Verify:

```bat
echo %VULKAN_SDK%
:: expect something like C:\VulkanSDK\1.3.296.0

glslc --version
:: expect: shaderc v2023.x ... target: spirv1.0

vulkaninfo --summary
:: expect: GPU name, apiVersion 1.3.x, no errors
```

If `vulkaninfo` fails, update the GPU driver first (official release from NVIDIA / AMD / Intel, not the old one Windows Update offers). The Vulkan runtime ships with the driver; you never install it separately.

### 5.4 Get the source

```bat
git clone https://github.com/KomdSpecific/Suzza.git
cd Suzza
```

Or use **Code → Download ZIP** on the GitHub page and unzip it (no `.git` needed to build).

> ⚠️ The repository `.gitignore` ignores `CMakePresets.json`, `tool/ReflectGenerated/generated/`, `assets/model/pony_cartoon/` and a few others. A clean clone from GitHub may therefore **lack those files** — section 7 explains what to do, and it is not an error.

---

## 6. A build that works

Run everything below inside **"x64 Native Tools Command Prompt"** (the 2026 entry is *... for VS*, the 2022 one is *... for VS 2022*):

```bat
:: 1) go to the repository root (the folder with CMakeLists.txt)
cd /d D:\path\to\SuzzaEngine

:: 2) configure - the generator MUST be Ninja
cmake -S . -B out\build\x64-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug

:: 3) build (a few minutes the first time, usually seconds afterwards)
cmake --build out\build\x64-debug

:: 4) run
out\build\x64-debug\SuzzaEngine.exe
```

> If your copy **does** contain `CMakePresets.json` (the author's machine has it), steps 2 and 3 can be shortened to:
>
> ```bat
> cmake --preset x64-debug
> cmake --build --preset x64-debug
> ```
>
> The two forms are **exactly equivalent**; the preset just expands to the commands above. If the preset file is missing, use the explicit commands.

Everything lands in `out\build\x64-debug\`:

| Path | Content |
| --- | --- |
| `SuzzaEngine.exe` | the application (about 4 MB, statically linked CRT) |
| `assets\` | resources copied from the repository root at the end of the build (about 22 MB) |
| `shaders\*.spv` | shader bytecode produced by `glslc` |
| `compile_commands.json` | compile database consumed by the reflection generator |
| `build.ninja`, `.ninja_log` | Ninja build script and log |
| whole folder | about 140 MB (Debug, PDB included) |

For `Release`, replace `x64-debug` / `Debug` with `x64-release` / `Release`; the two trees are independent.

---

## 7. Three build details you must know

### 7.1 `CMakePresets.json` may not exist in the repository at all

`.gitignore` contains the line `CMakePresets.json`, so the file is **not tracked by Git**. On a new machine, `cmake --preset x64-debug` will likely fail with:

```
CMake Error: No such preset in ...: x64-debug
```

Two ways out:

- **Recommended**: use the explicit commands from section 6 and never depend on the preset file.
- Or recreate the file yourself; its content is in [Appendix A](#appendix-a-cmakepresetsjson).

### 7.2 The generator must be Ninja (or Makefile), never the Visual Studio generator

One of the reflection generator's inputs is `compile_commands.json` (`CMakeLists.txt:115`), and **only Ninja / Makefile style generators produce that file**. The project enables it via `CMAKE_EXPORT_COMPILE_COMMANDS ON` (`CMakeLists.txt:15`).

If you run `cmake -G "Visual Studio 17 2022"` (VS 2022) or `-G "Visual Studio 18 2026"` (VS 2026):

- no `compile_commands.json` is produced, so the reflection step loses its dependency;
- the build either fails outright or silently reuses the **stale** files in `tool/ReflectGenerated/generated/` — you then add a field and the inspector never shows it, which is very hard to debug.

### 7.3 The build automatically does three invisible things

| Automatic step | Where | Output |
| --- | --- | --- |
| Compile shaders | `CMakeLists.txt:67-109`, calls `Vulkan::glslc` | `out/build/<preset>/shaders/*.spv` |
| Generate reflection code | `CMakeLists.txt:112-135`, runs the bundled generator exe | `tool/ReflectGenerated/generated/*.generated.{h,cpp}` |
| Copy assets | `CMakeLists.txt:207-211`, `POST_BUILD` | `<exe folder>/assets/` |

Two easy traps:

1. **The generator writes your machine's absolute paths into the generated files** (e.g. `"D:/LjhUser/.../src/scene/Actor.h"`). That is expected; it only matters on your own machine, do not "fix" it.
2. **The generated folder may not exist in a clean clone.** In practice the generator creates it by itself, no manual `mkdir` needed. If your copy complains about a missing directory, create it:

   ```bat
   mkdir tool\ReflectGenerated\generated
   ```

---

## 8. What you should see after launching

| What happens | Meaning |
| --- | --- |
| a 1600×1200 window titled `First Window` | window creation succeeded |
| three coloured cubes arranged in a parent/child hierarchy | the sample scene in `SuzzaApplication::Init` |
| ImGui panels `Hierarchy` and `Inspector` | the editor is running (`EDITOR` macro is on for non-production builds) |
| dragging with the left mouse button rotates the view | `MovementComponent` mouse look |
| `W` `A` `S` `D` move around | `MovementComponent` translation |
| closing the window exits, console shows no errors | clean shutdown |

> Both materials in the default scene are `new Material()` with **no texture path**, so no texture is uploaded and the cubes render as flat colours. That is expected, not a missing asset.

---

## 9. Environment self-check script

Paste the whole block into PowerShell; it prints OK / FAIL per item:

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

(Start menu: *Developer PowerShell for VS* for 2026, *Developer PowerShell for VS 2022* for 2022)

---

## 10. Troubleshooting

### 10.1 Configure stage

| Message you see | Cause | Fix |
| --- | --- | --- |
| `Could NOT find Vulkan (missing: Vulkan_LIBRARY Vulkan_INCLUDE_DIR)` | SDK not installed, or `VULKAN_SDK` not visible | reopen the terminal; check `%VULKAN_SDK%\Lib\vulkan-1.lib` exists |
| `No CMAKE_CXX_COMPILER could be found` | not in a VS developer prompt | reopen with "x64 Native Tools Command Prompt" |
| `No such preset in ...: x64-debug` | `CMakePresets.json` is gitignored | use the explicit commands, or Appendix A |
| `CMake 3.x is too old` / `preset version 3 unsupported` | CMake older than 3.21 | upgrade CMake |
| `ninja: command not found` | Ninja not on PATH | use the VS developer prompt, or install Ninja and add it to PATH |

### 10.2 Build stage

| Message you see | Cause | Fix |
| --- | --- | --- |
| `ninja: error: ... compile_commands.json` / reflection step says the file is missing | VS generator used, or configure failed | delete `out\build\x64-debug` and reconfigure with Ninja |
| `STL1000: static assertion failed: expected Clang 19.0.0 or newer` | the bundled generator embeds clang 18, while the STL from VS 2022 17.10+ (including VS 2026's 14.50 / 14.51) demands clang 19+ | harmless: the generator still exits with 0 and produces correct reflection data; see below if you insist on a clean log |
| `cannot open source file "xxx.generated.h"` | the reflection step did not run | check that `tool\ReflectGenerated\generated\*.generated.h` exists |
| linker error `unresolved external symbol` after adding a `.cpp` | `GLOB_RECURSE` was not refreshed | re-run `cmake -S . -B out\build\x64-debug -G Ninja` |
| Chinese comments look like garbage | the sources are **GBK** encoded but the editor opened them as UTF-8 | open the files as GBK / GB2312; do **not** casually re-save as UTF-8, or the comments are destroyed (it still compiles) |
| `MSVCP140.dll` / `VCRUNTIME140_1.dll` missing | VC++ runtime absent (the generator needs it) | install the [VC++ 2015-2022 x64 runtime](https://aka.ms/vs/17/release/vc_redist.x64.exe) (already there if VS is installed) |

More on `STL1000` (**expect to see it with VS 2026**): the 14.50 / 14.51 STL shipped with 2026 checks the compiler version more strictly, so the assertion shows up reliably - but it **does not change the build result**: the generator exits with 0 and its output is byte-identical to a 14.38 run. Only switch toolsets if you really want a clean log:

**VS 2022**: Installer -> Individual components -> *MSVC v143 - VS 2022 C++ x64/x86 build tools (v14.38-17.8)*, then in a developer prompt:

```bat
call "<VS2022 install dir>\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.38
cl
:: the version must read 19.38 for this to take effect
```

**VS 2026**: the installer usually still offers older toolsets under Individual components (names vary), select one with `-vcvars_ver=14.38` the same way. If your installer does not offer it, simply ignore the assertion - it does not affect the build.

### 10.3 Run stage

| Symptom | Likely cause | Fix |
| --- | --- | --- |
| double-clicking the exe flashes and closes | non-production build keeps a console; on an exception the window dies | run it from a terminal and read the output; `SUZZA_LOG_ERROR` prints the error |
| window opens but the screen is black | validation layers report descriptor / pipeline errors; camera matrices wrong | read the console; make sure the driver supports `VK_EXT_descriptor_indexing` |
| crashes on start, validation says an extension is unsupported | GPU or driver too old | update the driver, or use a machine with Vulkan 1.2 |
| "assets not found" | the exe was copied away from its `assets` folder | keep the whole `out\build\x64-debug` tree together, or put `assets` next to the exe |
| the "load pony_cartoon" exercise fails | `assets/model/*` is gitignored and absent in a clone | ask the author for the models, or drop your own glTF into `assets/model/pony_cartoon/` |

---

## 11. Windows 10 vs Windows 11

| Aspect | Windows 10 | Windows 11 |
| --- | --- | --- |
| VS 2022 or 2026 | on a very old Windows 10 (1909 or earlier) prefer VS 2022 | either works; take 2026 as the current default |
| Windows SDK | install 10.0.19041 or newer | same |
| Environment refresh | after installing the Vulkan SDK you **must reopen the terminal**, otherwise `VULKAN_SDK` is empty | same, reopen the terminal |
| Terminal | defaults to cmd / PowerShell; open "x64 Native Tools Command Prompt" manually | Windows Terminal is the default, but **still pick the VS developer prompt**, not a plain PowerShell |
| Long paths | must be enabled (policy / `LongPathsEnabled`) | enabled by default |
| First build speed | noticeably affected by Windows Defender real-time scanning | same; add the repo folder to the exclusions |
| GPU driver | avoid the old driver Windows Update pushes | same |
| WSL2 / Hyper-V | no conflict with Vulkan | no conflict |

**General advice**: put the repository on a **non-system drive, in a path without spaces or non-ASCII characters** (e.g. `D:\Code\SuzzaEngine`). CMake usually copes with spaces, but the reflection generator embeds the path into generated code, so the simpler the path the better.

---

## 12. Clean up and start over

All build output lives under `out\`; delete it to start from scratch:

```bat
rmdir /s /q out
cmake -S . -B out\build\x64-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build out\build\x64-debug
```

To force the reflection generator to run again:

```bat
rmdir /s /q tool\ReflectGenerated\generated
```

> Both folders are gitignored, so deleting them does not affect `git status`.

---

## 13. Appendices

### Appendix A: `CMakePresets.json`

If your clone lacks the file, save this next to `CMakeLists.txt` at the repository root. It matches the author's Windows presets:

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

> The author's file also contains `x86-*`, `linux-*` and `macos-*` presets; they are omitted here because the reflection generator only exists as a Windows x64 binary.

### Appendix B: preset commands vs explicit commands

| With presets | Equivalent explicit command |
| --- | --- |
| `cmake --preset x64-debug` | `cmake -S . -B out/build/x64-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug` |
| `cmake --build --preset x64-debug` | `cmake --build out/build/x64-debug` |
| `cmake --preset x64-release` | `cmake -S . -B out/build/x64-release -G Ninja -DCMAKE_BUILD_TYPE=Release` |

### Appendix C: build-related layout

```
SuzzaEngine/
├─ CMakeLists.txt                  <- top-level build script (shaders / reflection / asset copy)
├─ CMakePresets.json               <- may be gitignored; missing is normal
├─ config.h.in                     <- generates shared/config.h (resource path macros)
├─ shaders/*.vert|.frag            <- shader sources, compiled to .spv during the build
├─ assets/                         <- runtime resources, copied next to the exe
├─ thirdparty/                     <- vendored libraries, no package manager needed
├─ shared/  src/  tool/            <- engine and tool sources
├─ tool/ReflectGenerated/
│  ├─ GENERATED_REFLECT_CODE_PROGRAM.exe   <- prebuilt reflection generator (39 MB)
│  └─ generated/                           <- its output, may not exist
└─ out/build/<preset>/             <- build output (safe to delete)
```

### Appendix D: glossary

| Term | One-line explanation |
| --- | --- |
| CMake generator | decides which build files CMake emits; this project requires Ninja |
| `compile_commands.json` | the full compile command for every `.cpp`, consumed by the reflection generator |
| shader / SPIR-V | the program that runs on the GPU; `glslc` turns `.vert/.frag` into `.spv` |
| reflection | lets the program know at runtime which fields a class has; the inspector depends on it |
| validation layer | Vulkan's debug checker; prints detailed reasons when something is wrong |
| bindless | many textures in one descriptor array indexed at runtime; used here to cut descriptor binds |

---

*If this document and the code disagree, `CMakeLists.txt` in the repository wins.*
