# 构建与运行

## 依赖

- CMake ≥ 3.28
- C++17 编译器（推荐 MSYS2 MinGW64：`g++` + `ninja`）
- Git（FetchContent 拉取 SFML 需要）
- 网络（**首次**配置会下载 SFML 及其依赖，可能要几分钟）

请确保 PATH 中有：

```text
C:\msys64\mingw64\bin
D:\Cmake\bin
D:\Git\cmd
```

## 推荐构建命令（Ninja）

在项目根目录 `D:\Xgame\Xgame` 下：

```powershell
$env:Path = "C:\msys64\mingw64\bin;D:\Cmake\bin;D:\Git\cmd;" + $env:Path
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

可执行文件：`build\bin\StarshipWorkstation.exe`
请在 `build\bin` 目录运行（或确保能访问同目录下的 `assets/`）。

## 常见错误：FetchContent SFML failed

若出现：

```text
CMake Error at .../FetchContent.cmake:1921 (message):
  CMake step for sfml failed: 1
```

**最常见原因**：`build/` 里混用了不同生成器（例如先用了 MinGW Makefiles，后来又用 Ninja），缓存损坏。

### 解决办法（干净重建）

1. 关掉占用 `build` 的程序（CLion / VS / 正在运行的 `main.exe`）。
2. 删除构建目录后重配：

```powershell
$env:Path = "C:\msys64\mingw64\bin;D:\Cmake\bin;D:\Git\cmd;" + $env:Path
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

3. 若删不掉 `build`，换一个新目录：

```powershell
cmake -S . -B build2 -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build2
```

### 其他检查

| 现象 | 处理 |
|------|------|
| 找不到 `ninja` | 安装 MSYS2 包 `mingw-w64-x86_64-ninja`，或把 `C:\msys64\mingw64\bin` 加入 PATH |
| 找不到 `git` | 安装 Git，或把 `D:\Git\cmd` 加入 PATH |
| 生成器不一致提示 | 不要混用 `-G Ninja` 与 `-G "MinGW Makefiles"`；同一 `build` 目录始终用同一种 |
| 网络拉取失败 | 检查 GitHub 访问；可多试几次，或使用代理 |

## 可选：中文字体

将字体放到 `assets/fonts/`（如 `NotoSansSC-Regular.otf`），否则回退系统字体（微软雅黑 / 宋体等）。

## 快速验收

1. 单击桌面图标：仅选中，不打开窗口  
2. 双击：打开窗口；再次双击应复用已有窗口  
3. 有音效：单击 / 打开 / 错误密码 / 通关  
4. 解谜：我的文档 → IE → 我的电脑属性 → 档案室密码 → 加密档案室 → 回收站还原 → Starship.exe  
5. 隐藏结局：地址栏输入 `tesla-prize.local`，再输入完整密钥  
