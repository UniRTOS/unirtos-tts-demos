# unirtos-tts-demos

[中文](README.zh.md) | English

This repository is recommended to be used via the unirtos-cli demo workflow to ensure a consistent process for project creation, environment setup, and compilation.

## Feature Description

**Note: This demo requires an external ES8311 codec and SPI NOR flash.**

This demo demonstrates Text-to-Speech (TTS) playback on UniRTOS, including codec initialization, TTS session management, and optional external resource loading.

- Demonstrates ES8311 external codec initialization via I2C register configuration
- Demonstrates TTS language and callback configuration with `qosa_tts_open`
- Demonstrates UTF-8 text playback with `qosa_tts_play` and semaphore-based completion waiting
- Demonstrates optional external resource path support for TTS engine library stored on SPI NOR flash
- Demonstrates audio output routing to external codec through I2S interface

## Quick Start

### 1. Install the UniRTOS Toolchain

- [Development Preparation](https://www.quectel.com.cn/unirtos/docs?docs_page=快速上手/开发准备/开发准备.html)
- [Install the Cross-Compilation Toolchain](https://www.quectel.com.cn/unirtos/docs?docs_page=快速上手/环境搭建/环境搭建.html)
- [Install Python3](https://www.python.org/downloads/)
- [Install git](https://git-scm.com)
- Install unirtos-cli: `pip install unirtos-cli`

Once all the above tools are installed, verify the following commands are available:

```bash
python --version    # Python3
git --version
unirtos --version   # version 1.0.5 or above
unirtos-cli version # version 1.0.11 or above
```

### 2. Pull the Demo Using unirtos-cli

List available demos and versions:

```bash
unirtos-cli ls-demos
```

Create this demo project:

```bash
unirtos-cli new -r unirtos-tts-demos
```

To specify a version:

```bash
unirtos-cli new -r unirtos-tts-demos -v 1.0.0
```

### 3. Enter the Project and Build

```bash
cd unirtos-tts-demos-1.0.0
unirtos-cli env-setup
unirtos-cli build
```

## Common Commands

```bash
# Open the SDK menu configuration
unirtos-cli menuconfig

# Clean build artifacts
unirtos-cli clean
```

## Technical Community

Technical Community: https://forumschinese.quectel.com/c/66-category/66

## Contribution Guidelines

Contributions are welcome. Please follow these guidelines when submitting:
- Run a basic validation before submitting: env-setup, build, clean.
- Use clear commit messages describing the purpose of the change, its scope of impact, and verification results.
- When adding new features or changing behavior, update the README and related documentation accordingly.
- Submit bug fixes and feature improvements via Issues or Pull Requests.
