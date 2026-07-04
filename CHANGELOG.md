# LIZ Vision — Changelog

All notable changes to the LIZ Vision project are documented in this file.

---

## Sprint 19 — Cloud Sync Foundation (2026-07-05)

Implemented the Cloud Sync Foundation for the LIZ Vision engine. An
offline-first simulated cloud synchronization system, fully decoupled
from the engine core. Items are enqueued into a FIFO queue and processed
with a simulated 70% Synced / 30% Conflict probability distribution.

**New module:** `engine/cloud/`

- `CloudTypes` — enum `CloudSyncState` (5 states: Pending, Syncing, Synced, Conflict, Failed) and `CloudSyncType` (5 types: Project, Workspace, Asset, Pipeline, Settings) with inline string converters
- `CloudSyncItem` — sync item with UUID, name, type, state, timestamps, error message; non-copyable, movable
- `CloudSyncQueue` — FIFO queue with enqueue/dequeue/peek/size/clear operations
- `CloudSyncManager` — central sync manager: enqueue, process_all, process_item, 70/30 simulation, EventBus integration, statistics

**Integration:**

- EventBus: 4 new events (CloudSyncStarted, CloudSyncCompleted, CloudSyncFailed, CloudConflictDetected)
- Service Registry: CloudSyncManager registered as official service (ServiceType::CloudSyncManager)
- Diagnostics: cloud_pending, cloud_synced, cloud_conflicts, cloud_sync_time_ms shown in EngineStatistics and DiagnosticsManager reports
- Public API: `cloud_statistics()`, `list_cloud_items()`
- ApiTypes: new `CloudSyncInfo`, `ApiCloudStatistics`, `CloudSyncInfoList` types

**Architecture:**

```
Application
    |
    v
Public API (EngineAPI)
    |
    v
CloudSyncManager (lifecycle)
    |
    v
CloudSyncQueue (FIFO)
    |
    v
CloudSyncItem (state machine: Pending → Syncing → Synced/Conflict)
    |
    v
EventBus (CloudSyncStarted / CloudSyncCompleted / CloudConflictDetected)
```

**Backward compatibility:**
- All sprints 1-18 fully functional
- No modifications to GPU / Tensor / Pipeline / Asset / Project / Workspace internals

---

## Sprint 18 — Workspace Foundation (2026-07-05)

Implemented the Workspace system for the LIZ Vision engine. A Workspace
represents the complete user working environment, capable of containing
multiple projects, layouts, and preferences. Everything is in-memory
only — no persistence.

**New module:** `engine/workspace/`

- `WorkspaceTypes` — enum `WorkspaceState` (4 states: Created, Opened, Active, Closed) with inline string converter
- `WorkspaceLayout` — logical layout: panels, active panel, position, size, visibility (no GUI)
- `WorkspacePreferences` — user settings: theme, language, autosave, GPU, diagnostics, auto-validate, threads, batch size, recent projects limit
- `WorkspaceStatistics` — cumulative stats: workspaces created/open, active workspace, projects/assets/pipelines loaded, runtime
- `Workspace` — core workspace with UUID, name, state, timestamps, project list, layout, preferences
- `WorkspaceManager` — central lifecycle manager: create, open, close, destroy, set_active, add/remove project, list, statistics, EventBus integration

**Integration:**

- EventBus: 7 new events (WorkspaceCreated, WorkspaceOpened, WorkspaceActivated, WorkspaceClosed, WorkspaceDestroyed, ProjectAddedToWorkspace, ProjectRemovedFromWorkspace)
- Service Registry: WorkspaceManager registered as official service (ServiceType::WorkspaceManager)
- Diagnostics: active_workspace, loaded_workspaces, workspace_projects shown in EngineStatistics and DiagnosticsManager reports
- Public API: `create_workspace()`, `open_workspace()`, `close_workspace()`, `destroy_workspace()`, `set_active_workspace()`, `active_workspace()`, `workspace_statistics()`, `list_workspaces()`, `add_project_to_workspace()`, `remove_project_from_workspace()`
- ApiTypes: new `WorkspaceInfo`, `ApiWorkspaceStatistics`, `WorkspaceInfoList` types

**Architecture:**

```
Application
    |
    v
Public API (EngineBuilder / EngineAPI / EngineSession)
    |
    v
WorkspaceManager (lifecycle)
    |
    v
Workspace (projects + layout + preferences)
    |
    v
ProjectManager, PipelineGraph, AssetManager (existing subsystems)
```

**Backward compatibility:**
- All sprints 1-17 fully functional
- LegacyDemo, Sprint14Demo, Sprint15Demo, Sprint16Demo, Sprint17Demo unmodified

**Commit:** `Implement Workspace Foundation (Sprint 18)`

---

## Sprint 17 — Project System Foundation (2026-07-05)

Implemented the official Project system for the LIZ Vision engine.
A Project represents a complete LIZ Vision project with identity,
metadata, settings, and lifecycle management. Everything is
in-memory only — no persistence.

**New module:** `engine/project/`

- `ProjectTypes` — enum `ProjectState` (5 states: Created, Opened, Modified, Saved, Closed) with inline string converter
- `ProjectMetadata` — organizational data: company, website, description, tags, author, version
- `ProjectSettings` — project configuration: GPU, diagnostics, pipeline auto-validation, autosave, thread count, language, theme, batch size
- `ProjectStatistics` — cumulative stats: projects created/open/saved/closed, active project, assets, pipelines, services, runtime
- `Project` — core project class with UUID, name, description, author, version, state, timestamps, metadata, settings
- `ProjectManager` — central lifecycle manager: create, open, save, close, destroy, current_project, find, list, statistics, EventBus integration

**Integration:**

- EventBus: 5 new events (ProjectCreated, ProjectOpened, ProjectSaved, ProjectClosed, ProjectDestroyed)
- Service Registry: ProjectManager registered as official service (ServiceType::ProjectManager)
- Diagnostics: active_project, open_projects, saved_projects shown in EngineStatistics and DiagnosticsManager reports
- Public API: `create_project()`, `open_project()`, `save_project()`, `close_project()`, `destroy_project()`, `current_project()`, `project_statistics()`, `list_projects()`
- ApiTypes: new `ProjectInfo`, `ApiProjectStatistics`, `ProjectInfoList` types

**Architecture:**

```
Application
    |
    v
Public API (EngineBuilder / EngineAPI / EngineSession)
    |
    v
ProjectManager (lifecycle)
    |
    v
Project (identity + metadata + settings)
    |
    v
PipelineGraph, AssetManager, Diagnostics (existing subsystems)
```

**Backward compatibility:**
- All sprints 1-16 fully functional
- LegacyDemo, Sprint14Demo, Sprint15Demo, Sprint16Demo unmodified

**Commit:** `Implement Project System Foundation (Sprint 17)`

---

## Sprint 16 — Pipeline Graph Foundation (2026-07-05)

Created the Pipeline Graph architecture — the foundation for all future
LIZ Engine processing workflows. Processing pipelines are now represented
as Directed Acyclic Graphs (DAGs) where nodes represent operations and
edges represent data flow.

No real AI, GPU, or video processing is performed — this sprint creates
only the infrastructure.

**New module:** `engine/pipeline/`

- `PipelineTypes` — enums: `PipelineNodeType` (11 types), `PipelineNodeState` (6 states), `ConnectionType` (Data, Control, Dependency)
- `PipelineNode` — node with UUID, name, type, state, input/output tracking, logical position (x,y), enable/disable, simulated `execute()`
- `PipelineEdge` — directed connection between nodes with UUID, source/dest, connection type, active state
- `PipelineGraph` — DAG manager: `create_node()`, `remove_node()`, `connect()`, `disconnect()`, `find_node()`, `find_edge()`, `list_nodes()`, `list_edges()`, `statistics()`, `clear()`, `validate()` with cycle detection (DFS-based)
- `PipelineExecutor` — graph traversal engine: `register_pipeline()`, `destroy_pipeline()`, `execute()` with topological sort (Kahn's algorithm), `pipeline_statistics()`, `global_statistics()`
- `PipelineStatistics` — stats: nodes, edges, executed, failed, disabled, execution_time, validation_result

**Architecture:**

```
Application
    |
    v
Public API (EngineBuilder / EngineAPI / EngineSession)
    |
    v
PipelineGraph (DAG)
    |
    v
PipelineNode (operations) <-- PipelineEdge (connections)
    |
    v
Engine (GPU / AI / Video / ...)
```

**Integration:**

- EventBus: 5 new events (PipelineCreated, PipelineDestroyed, NodeCreated, NodeExecuted, PipelineFinished)
- Service Registry: PipelineExecutor registered as official service (ServiceType::PipelineExecutor)
- Diagnostics: pipeline stats in EngineStatistics and DiagnosticsManager reports
- Public API: `create_pipeline()`, `destroy_pipeline()`, `pipeline_statistics()`, `list_pipelines()`
- ApiTypes: new `PipelineInfo` and `PipelineInfoList` types

**Cycle detection:**
- DFS-based cycle prevention on `connect()`
- Validates entire graph on `validate()`
- Proposed edges are tested before insertion

**Backward compatibility:**
- All sprints 1-15 fully functional
- LegacyDemo, Sprint14Demo, Sprint15Demo unmodified

**Commit:** `Implement Pipeline Graph Foundation (Sprint 16)`

---

## Sprint 15 — Public API Foundation (2026-07-05)

Created the first official public API layer for the LIZ Engine. This sprint
does NOT add new processing functionality — it creates a stable interface
for external applications to consume the engine without accessing internals.

Prepares the foundation for a future distributable SDK.

**New module:** `engine/api/`

- `ApiVersion` — semantic versioning (major.minor.patch+build), `version_string()`, `compatible_with()`, comparison operators
- `ApiResult` — 10 result codes (Success, Warning, Failed, InvalidArgument, NotFound, AlreadyExists, Busy, Unsupported, InternalError, Timeout)
- `ApiTypes` — public data structures: `ApiInfo`, `ApiStatistics`, `EngineInfo`, `SessionInfo`, `ServiceInfo`, `AssetInfo`, `DiagnosticsInfo`
- `EngineSession` — public session with UUID, creation timestamp, `is_running()`, `shutdown()`, `active_time_ms()`
- `EngineBuilder` — builder pattern: `set_application_name()`, `set_application_version()`, `enable_gpu()`, `enable_plugins()`, `enable_diagnostics()`, `enable_assets()`, `enable_events()`, `build()`
- `EngineAPI` — public facade: `initialize()`, `shutdown()`, `create_session()`, `destroy_session()`, `version()`, `statistics()`, `runtime_info()`, `service_info()`, `asset_info()`, `diagnostics_info()`

**Architecture:**

```
Applications
    |
    v
Public API (EngineBuilder / EngineAPI / EngineSession)
    |
    v
Runtime / Services / Assets / Diagnostics / ...
```

**SDK structure:**

- `sdk/README.md` — Hello LIZ Engine example, API reference, architecture overview

**DemoRunner improvements:**

- `DemoRunner` now requires `name()` method (polymorphism)
- `DemoRegistry` class: `register_demo()`, `remove_demo()`, `clear()`, `run_all()`, `run_demo(name)`, `list()`, `count()`, `has()`
- No giant if-else chains — all demos registered dynamically via polymorphism
- `main.cpp` uses `DemoRegistry` to run all demos

**Integration:**

- Logger: all API operations logged
- Engine: internal engine created and managed by EngineAPI
- Service Registry: 11 services registered during initialization
- AssetManager: initialized if assets enabled in builder config
- EventBus: created if events enabled in builder config
- DiagnosticsManager: created if diagnostics enabled in builder config

**Backward compatibility:**

- All previous sprints (1-14) remain fully functional
- LegacyDemo and Sprint14Demo continue to work without modifications
- `DemoRunner` interface extended with `name()` — all existing demos updated

**Commit:** `Implement Public API Foundation (Sprint 15)`

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
