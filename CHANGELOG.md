# LIZ Vision — Changelog

All notable changes to the LIZ Vision project are documented in this file.

---

## Sprint 14 — Asset System Foundation (2026-07-05)

Implemented the official asset management system for the engine.
Manages reusable resources: videos, AI models, configurations, shaders,
plugins, textures, serialized tensors, projects. All simulated — no
filesystem access.

**New module:** `engine/assets/`

- `AssetType` — enum with 11 types (Unknown, Video, Image, Audio, Model, Tensor, Shader, Plugin, Configuration, Project, Temporary)
- `AssetState` — 5 lifecycle states (Unloaded, Loading, Loaded, Unloading, Error)
- `Asset` — base class with UUID, name, type, version, size, state transitions, timestamps, reference counting
- `AssetHandle` — lightweight handle with UUID, name, type, valid(), acquire(), release()
- `AssetDatabase` — in-memory database with insert, remove, find, find_by_name, exists, list, count, clear, statistics
- `AssetManager` — central manager with create_asset, destroy_asset, load, unload, reload, find, statistics, clear, EventBus integration

**Integration:**
- Logger: all operations logged
- EventBus: AssetCreated, AssetLoaded, AssetUnloaded, AssetDestroyed events
- Service Registry: AssetManager registered as official service
- Diagnostics: assets_loaded and assets_active shown in reports
- EventType: 4 new event types added (backward compatible)

**Refactoring:** Introduced DemoRunner pattern. All demo logic moved to
`apps/liz-vision-desktop/demo/`. main.cpp now only initializes Engine
and delegates to DemoRunner implementations.

**Commit:** `Implement Asset System Foundation (Sprint 14)`

---

## Sprint 13 — Diagnostics & Profiler Foundation (2026-07-05)

Implemented the official diagnostics and profiling system for the engine.
This module provides runtime monitoring, performance snapshots, and
cumulative statistics without modifying existing subsystem behavior.

**New module:** `engine/diagnostics/`

- `Profiler` — lightweight timer with start/stop/begin_section/end_section
- `ProfilerSession` — completed session record (UUID, name, duration, samples)
- `PerformanceSnapshot` — point-in-time metrics (FPS, frame time, CPU/GPU time, RAM, VRAM, events, services, resources, tasks)
- `EngineStatistics` — cumulative engine statistics (frames, events, services, resources, plugins, batches, inferences, GPU commands)
- `DiagnosticsManager` — central diagnostics hub with capture_snapshot, statistics, print_report
- `DiagnosticsDataProvider` — data source interface to avoid circular dependencies

**Integration:** Reuses data from existing subsystems (PerformanceManager, GPUMemoryPool, Scheduler, PluginManager, ResourceManager) via provider pattern.

**Commit:** `Implement Diagnostics and Profiler Foundation (Sprint 13)`

---

## Sprint 12 — Service Registry Foundation (2026-07-05)

Implemented the official Service Registry for the engine.

**New module:** `engine/services/`

- `ServiceType` — enum with 12 service types + `ServiceState` with 5 lifecycle states
- `Service` — base class with UUID, name, type, version, state transitions
- `ServiceRegistry` — central registry with register/unregister/find/exists/count/list/statistics
- `ServiceLocator` — static convenience interface for service lookup by name or type

**Commit:** `Implement Service Registry Foundation (Sprint 12)`

---

## Sprint 11 — Event Bus Foundation (2026-07-05)

Implemented the central event bus for module-to-module communication.

**New module:** `engine/events/`

- `Event`, `EventType`, `EventPriority`, `EventListener`, `EventDispatcher`, `EventBus`

**Commit:** `Implement Event Bus Foundation (Sprint 11)`

---

## Sprint 10 — Runtime Architecture Foundation (2026-07-05)

Implemented the central Runtime lifecycle controller.

**New module:** `engine/runtime/`

- `Runtime`, `RuntimeState`, `RuntimeStage`, `LifecycleManager`

**Commit:** `Implement Runtime Architecture Foundation (Sprint 10)`

---

## Sprint 9 — Resource Management Foundation (2026-07-05)

Implemented centralized resource management.

**New module:** `engine/resources/`

- `Resource`, `ResourceType`, `ResourceState`, `ResourceCache`, `ResourceHandle`, `ResourceManager`

**Commit:** `Implement Resource Management Foundation (Sprint 9)`

---

## Sprint 8 — Tensor Layer + Batch Processing (2026-07-05)

Added tensor conversion and GPU batch processing.

**New files:** `engine/ai/tensor/`, `engine/gpu/batch/`

**Commit:** `Implement Tensor + GPU Batch Processing Layer (Sprint 8)`

---

## Sprint 7 — GPU Execution Layer (Stable) (2026-07-05)

Stabilized GPU execution with FIFO command pipeline and memory pool.

**New files:** `engine/gpu/compute/`, `engine/gpu/memory/`

**Commit:** `Fix GPU Execution Layer - Sprint 7 stable version`

---

## Sprint 6 — Performance Layer (2026-07-05)

Added thread pool, frame queue, batch processor, and performance manager.

**New module:** `engine/performance/`

**Commit:** `Implement Performance + GPU Execution Layer (Sprint 6)`

---

## Sprint 5 — Inference Layer (2026-07-05)

Implemented AI inference engine with model loading.

**New files:** `engine/ai/inference/`, `engine/ai/models/`

**Commit:** `Implement AI Inference Layer base (Sprint 5)`

---

## Sprint 4 — GPU Context + FFmpeg Base (2026-07-05)

Added GPU abstraction and FFmpeg decoder with CPU fallback.

**New files:** `engine/gpu/`, `engine/video/ffmpeg/`

**Commit:** `Implement GPU abstraction and FFmpeg base (Sprint 4)`

---

## Sprint 3 — AI Processing Layer (2026-07-05)

Implemented AI processing pipeline with demo processors.

**New module:** `engine/ai/`

**Commit:** `Implement AI Processing Core base (Sprint 3)`

---

## Sprint 2 — Video Pipeline (2026-07-05)

Implemented video decoding pipeline with streaming support.

**New files:** `engine/video/VideoFrame`, `VideoDecoder`, `VideoPipeline`

**Commit:** `Implement Video Pipeline base (Sprint 2)`

---

## Sprint 1 — Engine Core (2026-07-05)

Initial engine core with Logger, Config, EngineContext, Scheduler, and Plugin System.

**New modules:** `engine/core/`, `engine/scheduler/`, `engine/plugins/`

**Commit:** `Implement LIZ Engine Core base system (Sprint 1)`
