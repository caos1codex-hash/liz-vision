# LIZ Engine SDK

Official SDK for integrating LIZ Engine into third-party applications.

## Quick Start — Hello LIZ Engine

The LIZ Engine SDK exposes a clean, minimal API. Applications should only
interact with three classes:

```
EngineBuilder  ->  EngineAPI  ->  EngineSession
```

### Step 1: Configure and Build

```cpp
#include "engine/api/EngineBuilder.h"
#include "engine/api/EngineAPI.h"

// Create a builder and configure the engine.
liz::EngineBuilder builder;
builder.set_application_name("MyApp")
       .set_application_version("1.0.0")
       .enable_gpu(true)
       .enable_diagnostics(true)
       .enable_assets(true)
       .enable_events(true)
       .build();
```

### Step 2: Initialize the API

```cpp
liz::EngineAPI api;
auto result = api.initialize(builder);

if (result != liz::ApiResult::Success) {
    // Handle initialization error.
    return;
}
```

### Step 3: Create a Session

```cpp
liz::SessionInfo session;
api.create_session(session);

// The session ID can be used to track multiple sessions.
// Sessions are independent and can be created/destroyed at will.
```

### Step 4: Query Engine Information

```cpp
// Version
auto ver = api.version();
// ver.version_string() -> "1.0.0"

// Engine info
auto info = api.engine_info();
// info.session_id, info.state, info.engine_version, etc.

// Statistics
auto stats = api.statistics();
// stats.services_registered, stats.assets_active, etc.

// Services
auto services = api.service_info();
for (const auto& svc : services) {
    // svc.name, svc.type, svc.state, svc.version
}

// Assets
auto assets = api.asset_info();
for (const auto& asset : assets) {
    // asset.name, asset.type, asset.state, asset.size_bytes
}
```

### Step 5: Shutdown

```cpp
// Destroy sessions
api.destroy_session(session.session_id);

// Shut down the engine
api.shutdown();
```

## Architecture

```
Applications
    |
    v
Public API (EngineBuilder / EngineAPI / EngineSession)
    |
    v
Runtime
    |
    v
Services / Assets / Resources / Diagnostics / Performance
    |
    v
Scheduler / GPU / AI / Video
```

## API Reference

### ApiResult

Result codes for all API operations:

| Code | Description |
|------|-------------|
| `Success` | Operation completed successfully |
| `Warning` | Succeeded with caveats |
| `Failed` | General failure |
| `InvalidArgument` | Invalid input parameter |
| `NotFound` | Resource not found |
| `AlreadyExists` | Duplicate resource |
| `Busy` | Engine is busy |
| `Unsupported` | Not supported in current config |
| `InternalError` | Internal engine error |
| `Timeout` | Operation timed out |

### ApiVersion

Semantic versioning: `major.minor.patch+build`

```cpp
auto ver = api.version();
ver.version_string();       // "1.0.0"
ver.full_version_string();  // "1.0.0+1"
ver.compatible_with(other); // Same major = compatible
```

### EngineBuilder

Builder pattern for engine configuration:

| Method | Description |
|--------|-------------|
| `set_application_name(name)` | Set app name |
| `set_application_version(ver)` | Set app version |
| `enable_gpu(true/false)` | GPU acceleration |
| `enable_plugins(true/false)` | Plugin system |
| `enable_diagnostics(true/false)` | Profiling & diagnostics |
| `enable_assets(true/false)` | Asset management |
| `enable_events(true/false)` | Event bus |
| `build()` | Finalize configuration |

### EngineAPI

Public facade for the engine:

| Method | Description |
|--------|-------------|
| `initialize(config)` | Initialize engine from builder config |
| `shutdown()` | Gracefully shut down |
| `is_initialized()` | Check if ready |
| `create_session(info)` | Create a new session |
| `destroy_session(id)` | Destroy a session |
| `list_sessions()` | List active sessions |
| `version()` | Get API version |
| `engine_info()` | Get engine information |
| `statistics()` | Get engine statistics |
| `service_info()` | List registered services |
| `asset_info()` | List managed assets |
| `diagnostics_info()` | Get diagnostics data |

## Design Principles

- **Encapsulation**: Internal engine details are completely hidden.
- **Delegation**: The public API delegates to existing engine subsystems.
- **No duplication**: No logic is copied or moved from internal modules.
- **Low coupling**: The API depends only on public types.
- **Future SDK**: This layer prepares for distribution as a shared library.

## Pipeline Graph

The Pipeline Graph is the foundation for all future LIZ Engine operations.
Processing workflows (Video, AI, GPU, Plugins, Assets) can be represented as
directed acyclic graphs (DAGs).

### Creating a Pipeline via Public API

```cpp
// Create a pipeline.
api.create_pipeline("My Upscale Pipeline");

// The pipeline is managed internally.
// In future sprints, the API will expose node/edge manipulation.
```

### Architecture

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

### Pipeline Node Types

| Type | Description |
|------|-------------|
| `Input` | Data input (file, stream, buffer) |
| `Decoder` | Video/audio decoding |
| `VideoFilter` | Visual processing filter |
| `TensorConverter` | Convert frames to tensors |
| `Inference` | AI model inference |
| `GPUUpload` | Upload data to GPU memory |
| `GPUCompute` | GPU compute operation |
| `GPUDownload` | Download data from GPU memory |
| `Output` | Final output (file, display, buffer) |
| `Custom` | User-defined node type |

### Pipeline API (EngineAPI)

| Method | Description |
|--------|-------------|
| `create_pipeline(name)` | Create a new pipeline |
| `destroy_pipeline(name)` | Destroy a pipeline |
| `pipeline_statistics(name)` | Get pipeline statistics |
| `list_pipelines()` | List all pipelines |

## Project System

The Project System manages complete LIZ Vision projects. Each project
contains identity, metadata, settings, and lifecycle state. Projects
integrate with pipelines, assets, and diagnostics.

### Creating a Project via Public API

```cpp
// Create a project.
liz::ProjectInfo proj_info;
api.create_project("My Video Project", proj_info);
// proj_info.uuid, proj_info.name, proj_info.state are populated.

// Open the project.
api.open_project(proj_info.uuid);

// Modify metadata (through the Project object — internal API).
// Save the project (simulated).
api.save_project(proj_info.uuid);

// Close the project.
api.close_project(proj_info.uuid);

// Destroy the project completely.
api.destroy_project(proj_info.uuid);
```

### Project API (EngineAPI)

| Method | Description |
|--------|-------------|
| `create_project(name, info)` | Create a new project |
| `open_project(uuid)` | Open a project |
| `save_project(uuid)` | Save a project (simulated) |
| `close_project(uuid)` | Close a project |
| `destroy_project(uuid)` | Destroy a project |
| `current_project(info)` | Get the active project |
| `project_statistics()` | Get project system statistics |
| `list_projects()` | List all projects |

### Project States

| State | Description |
|-------|-------------|
| `Created` | Project has been created |
| `Opened` | Project is open for editing |
| `Modified` | Project has unsaved changes |
| `Saved` | Project has been saved |
| `Closed` | Project is closed |

## Workspace System

The Workspace System manages the complete user working environment.
Each workspace can contain multiple projects, has its own layout
configuration and user preferences. Workspaces integrate with
projects, pipelines, assets, and diagnostics.

### Creating a Workspace via Public API

```cpp
// Create a workspace.
liz::WorkspaceInfo ws_info;
api.create_workspace("My Studio", ws_info);

// Open and activate the workspace.
api.open_workspace(ws_info.uuid);
api.set_active_workspace(ws_info.uuid);

// Add a project to the workspace.
api.create_project("Video Project", proj_info);
api.add_project_to_workspace(ws_info.uuid, proj_info.uuid);

// Close the workspace.
api.close_workspace(ws_info.uuid);

// Destroy the workspace.
api.destroy_workspace(ws_info.uuid);
```

### Workspace API (EngineAPI)

| Method | Description |
|--------|-------------|
| `create_workspace(name, info)` | Create a new workspace |
| `open_workspace(uuid)` | Open a workspace |
| `close_workspace(uuid)` | Close a workspace |
| `destroy_workspace(uuid)` | Destroy a workspace |
| `set_active_workspace(uuid)` | Set the active workspace |
| `active_workspace(info)` | Get the active workspace |
| `workspace_statistics()` | Get workspace system statistics |
| `list_workspaces()` | List all workspaces |
| `add_project_to_workspace(ws_uuid, proj_uuid)` | Add a project to a workspace |
| `remove_project_from_workspace(ws_uuid, proj_uuid)` | Remove a project from a workspace |

### Workspace States

| State | Description |
|-------|-------------|
| `Created` | Workspace has been created |
| `Opened` | Workspace is open |
| `Active` | Workspace is the active one |
| `Closed` | Workspace is closed |

## Cloud Sync System

The Cloud Sync System provides offline-first simulated cloud synchronization.
Items are enqueued into a FIFO queue and processed with simulated network
behavior (70% Synced, 30% Conflict). The system is fully decoupled from
the engine core and integrates via EventBus, ServiceRegistry, and Diagnostics.

### Cloud Sync API (EngineAPI)

| Method | Description |
|--------|-------------|
| `cloud_statistics()` | Get cloud sync statistics (pending, synced, conflicts, sync time) |
| `list_cloud_items()` | List all processed cloud sync items |

### Cloud Sync Example

```cpp
// Get cloud statistics.
auto stats = api.cloud_statistics();
// stats.items_synced    -> number of synced items
// stats.items_conflict -> number of conflicts
// stats.items_pending  -> number of pending items

// List processed cloud items.
auto items = api.list_cloud_items();
for (const auto& item : items) {
    // item.uuid, item.name, item.type, item.state, item.error_message
}
```

### Cloud Sync States

| State | Description |
|-------|-------------|
| `Pending` | Item is queued and waiting for sync |
| `Syncing` | Item is currently being synced |
| `Synced` | Item successfully synced to cloud |
| `Conflict` | Version conflict detected |
| `Failed` | Sync operation failed |

### Cloud Sync Types

| Type | Description |
|------|-------------|
| `Project` | Project data sync |
| `Workspace` | Workspace configuration sync |
| `Asset` | Asset file sync |
| `Pipeline` | Pipeline graph sync |
| `Settings` | Application settings sync |

## Plugin Loader System

The Plugin Loader provides a simulated plugin loading and management system.
No real DLLs or shared libraries are loaded — everything is in-memory,
prepared for future dynamic loading without breaking the API.

### Plugin Loader API (EngineAPI)

| Method | Description |
|--------|-------------|
| `plugin_statistics()` | Get plugin loader statistics (registered, loaded, failed, reloaded) |
| `list_plugins()` | List all registered plugins with their state |
| `load_plugin(uuid)` | Load a plugin by UUID |
| `reload_plugin(uuid)` | Reload a plugin by UUID |
| `unload_plugin(uuid)` | Unload a plugin by UUID |

### Plugin Loader Example

```cpp
// Get plugin statistics.
auto stats = api.plugin_statistics();
// stats.plugins_registered -> total registered
// stats.plugins_loaded     -> currently loaded
// stats.plugins_failed     -> load failures
// stats.plugins_reloaded   -> reloads performed

// List all plugins.
auto plugins = api.list_plugins();
for (const auto& p : plugins) {
    // p.uuid, p.name, p.author, p.version, p.category, p.state
}

// Load and unload plugins.
api.load_plugin(plugin_uuid);
api.reload_plugin(plugin_uuid);
api.unload_plugin(plugin_uuid);
```

### Plugin Lifecycle Events

| Event | Description |
|-------|-------------|
| `PluginLoading` | Plugin is being loaded |
| `PluginLoaded` | Plugin successfully loaded |
| `PluginUnloading` | Plugin is being unloaded |
| `PluginUnloaded` | Plugin successfully unloaded |
| `PluginReloaded` | Plugin reloaded (unload + load) |
| `PluginLoadFailed` | Plugin load failed |
