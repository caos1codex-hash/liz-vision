# LIZ Vision

**AI Video Enhancement Engine**

---

## Overview

LIZ Vision is a modular AI-powered video enhancement engine designed for high-performance real-time and offline video processing. The system is built with a plugin-based architecture that enables extensible video processing pipelines, supporting upscaling, frame interpolation, restoration, and object detection workflows.

## Objective

Build a scalable, production-ready engine that leverages GPU acceleration (CUDA, TensorRT, ONNX Runtime) and modern C++ to deliver state-of-the-art AI video enhancement capabilities through a clean, modular, and extensible architecture.

## Architecture

```
liz-vision/
├── engine/          # Core engine modules (video, AI, GPU, memory, scheduler)
├── apps/            # Standalone applications (desktop CLI, GUI)
├── plugins/         # Pluggable processing modules (upscalers, interpolation, etc.)
├── models/          # AI model weights and metadata
├── tests/           # Unit and integration tests
├── scripts/         # Build, deploy, and utility scripts
├── tools/           # Development and diagnostic tools
├── third_party/     # Vendored dependencies
├── assets/          # Static assets (shaders, configs, sample media)
├── cmake/           # CMake modules and toolchains
├── docs/            # Architecture docs, roadmaps, API references
└── .github/         # CI/CD workflows
```

## Tech Stack

| Component       | Technology             |
|-----------------|------------------------|
| Language        | C++20                  |
| Build System    | CMake 3.20+            |
| Target OS       | Linux (Ubuntu)         |
| GPU Backend     | CUDA / TensorRT / ONNX |
| Video I/O       | FFmpeg (future sprint) |

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Status

> **This is the base architecture scaffold.** No AI models, video processing, GPU code, or functional features are implemented yet. The project compiles as an empty shell, ready for incremental development in future sprints.

## License

TBD