# AEMU 主机公共库

[English](README.md) | 简体中文

本仓库提供 gfxstream 和跨平台主机构建使用的 AEMU 公共基础库。BSCP 在 Android 固定基线
上维护少量 CMake 与操作系统兼容修改，使 Linux、macOS 和 Windows 构建使用一致的依赖发现
与主机抽象。

该组件由 BSCP 根构建脚本间接构建。修改公共 API、文件系统或动态库加载行为时，必须验证
三个主机平台，并避免把本机绝对路径或生成产物提交到仓库。
