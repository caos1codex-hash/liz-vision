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

## Job System

The Job System provides a high-level abstraction for executing long-running
operations (render, inference, import, export, cloud, plugins, pipelines,
resources, projects, workspaces) through the existing Scheduler.

Does NOT replace the Scheduler or ThreadPool — works on top of them.
Each submitted Job is converted into a Scheduler Task for execution.

### Job System Architecture

```
Application
    |
    v
EngineAPI (submit_job, list_jobs, job_statistics)
    |
    v
JobManager (submit -> queue -> convert to Scheduler Task)
    |
    v
JobQueue (FIFO) -> Job (metadata + state)
    |
    v
Scheduler (existing, unchanged) -> Task (execution)
    |
    v
EventBus (JobCreated / JobQueued / JobStarted / JobCompleted / JobFailed / JobCancelled)
```

### Job System API (EngineAPI)

| Method | Description |
|--------|-------------|
| `submit_job(name, type, info)` | Submit a job by name and type string; returns JobInfo |
| `cancel_job(uuid)` | Cancel a queued job by UUID |
| `list_jobs()` | List all processed jobs with their state and duration |
| `job_statistics()` | Get job system statistics (created, running, completed, failed) |

### Job System Example

```cpp
// Submit a job via the public API.
liz::JobInfo info;
api.submit_job("AI Upscaling", "Inference", info);
// info.uuid, info.name, info.state -> "Queued"

// Get job statistics.
auto stats = api.job_statistics();
// stats.jobs_created    -> total created
// stats.jobs_running    -> currently running
// stats.jobs_completed  -> completed
// stats.jobs_failed     -> failed
// stats.total_execution_time_ms -> total time

// List all processed jobs.
auto jobs = api.list_jobs();
for (const auto& j : jobs) {
    // j.uuid, j.name, j.type, j.priority, j.state
    // j.progress (0-100), j.duration_ms
}
```

### Job Types

| Type | Description |
|------|-------------|
| `Render` | Frame rendering operations |
| `Inference` | AI inference (upscaling, interpolation, etc.) |
| `Pipeline` | Pipeline graph execution |
| `Import` | File/resource import |
| `Export` | File/resource export |
| `Plugin` | Plugin loading/reloading |
| `Cloud` | Cloud sync operations |
| `Asset` | Asset management operations |
| `Project` | Project operations |
| `Workspace` | Workspace operations |
| `Custom` | User-defined operations |

### Job Priority Levels

| Priority | Description |
|----------|-------------|
| `Low` | Background tasks |
| `Normal` | Default priority |
| `High` | Important tasks |
| `Critical` | Time-sensitive tasks |

### Job States

| State | Description |
|-------|-------------|
| `Created` | Job created but not yet queued |
| `Queued` | Job waiting in the FIFO queue |
| `Running` | Job currently executing |
| `Completed` | Job finished successfully (progress=100) |
| `Failed` | Job finished with error |
| `Cancelled` | Job cancelled before execution |

### Job Lifecycle Events

| Event | Description |
|-------|-------------|
| `JobCreated` | Job created |
| `JobQueued` | Job added to queue |
| `JobStarted` | Job started executing |
| `JobCompleted` | Job finished successfully |
| `JobFailed` | Job finished with error |
| `JobCancelled` | Job cancelled |

## Configuration System

The Configuration System provides a fully in-memory configuration management system.
No JSON, YAML, XML, or INI files are read or written — all configuration
lives entirely in memory. This serves as the base for future persistence layers.

### Configuration System Architecture

```
Application
    |
    v
EngineAPI (create / destroy / set_value / get_value)
    |
    v
ConfigurationManager (create / destroy / set_active)
    |
    v
Configuration (UUID + sections + timestamps)
    |
    v
ConfigSection (named group)
    |
    v
ConfigValue (key + type + value + default + modified flag)
    |
    v
EventBus (ConfigurationCreated / Destroyed / Activated / ValueChanged / Reset)
```

### Configuration System API (EngineAPI)

| Method | Description |
|--------|-------------|
| `create_configuration(name, info)` | Create a new configuration profile |
| `destroy_configuration(uuid)` | Destroy a configuration by UUID |
| `active_configuration(info)` | Get the active configuration info |
| `list_configurations()` | List all configurations |
| `configuration_statistics()` | Get configuration system statistics |
| `set_value(section, key, value)` | Set a config value (string representation) |
| `get_value(section, key)` | Get a config value as string |

### Configuration System Example

```cpp
// Create a configuration via the public API.
liz::ConfigurationInfo info;
api.create_configuration("Production", info);

// Set values.
api.set_value("Graphics", "width", "1920");
api.set_value("Graphics", "height", "1080");
api.set_value("Audio", "volume", "80");
api.set_value("GPU", "backend", "CUDA");

// Get values.
std::string backend = api.get_value("GPU", "backend");
// backend -> "CUDA"

// Get statistics.
auto stats = api.configuration_statistics();
// stats.configurations -> total configs
// stats.sections -> total sections
// stats.values -> total values
// stats.modified -> modified values

// List all configurations.
auto configs = api.list_configurations();
for (const auto& c : configs) {
    // c.uuid, c.name, c.active, c.sections, c.values, c.modified
}
```

### Configuration Value Types

| Type | Description |
|------|-------------|
| `Bool` | Boolean (true/false) |
| `Int` | Integer |
| `Double` | Floating point |
| `String` | String value |

### Configuration Lifecycle Events

| Event | Description |
|-------|-------------|
| `ConfigurationCreated` | Configuration created |
| `ConfigurationDestroyed` | Configuration destroyed |
| `ConfigurationActivated` | Configuration set as active |
| `ConfigurationValueChanged` | A value was modified |
| `ConfigurationReset` | Values reset to defaults |

## Advanced Configuration System (Sprint 23)

The Advanced Configuration System is a layer built **on top of the Sprint 22
Configuration Foundation**. It does not replace the Foundation — it wraps the
existing `ConfigurationManager` and adds schema validation, execution profiles,
environment overrides, runtime override priority, in-memory persistence, and
dynamic reload. All Foundation APIs above continue to work unchanged.

### Advanced Configuration Architecture

```
Application
    |
    v
EngineAPI (validate / override / profile / reload / advanced stats)
    |
    v
AdvancedConfigurationManager  ── wraps (composition) ──>  ConfigurationManager (Sprint 22)
    |
    ├── ConfigSchema            (rules + validate)
    ├── ConfigOverrideStack     (priority resolution: SchemaDefault < Profile < Environment < Runtime)
    ├── ConfigEnvironment       (LIZ_<SECTION>_<KEY> env vars)
    ├── ConfigPersistence        (INI-like text serialize/deserialize)
    └── ConfigReloader          (reload from bound file path)
```

### Advanced Configuration API (EngineAPI)

| Method | Description |
|--------|-------------|
| `validate_configuration()` | Validate the active configuration against the schema |
| `configuration_schema_summary()` | Get a `ConfigSchemaSummary` (section + rule counts) |
| `apply_config_override(section, key, value)` | Apply a runtime override (highest priority) |
| `create_profiled_configuration(name, profile, path, info)` | Create a config bound to an execution profile |
| `activate_configuration_profile(profile)` | Activate the configuration bound to a profile |
| `reload_configuration()` | Reload the active configuration from its bound file path |
| `serialize_active_configuration()` | Serialize the active configuration to text |
| `advanced_configuration_statistics()` | Get the advanced counters |

### Advanced Configuration Example

```cpp
// Create a configuration tagged with an execution profile and a file path.
liz::ConfigurationInfo info;
api.create_profiled_configuration("Engine", "Production", "engine.cfg", info);

// Apply a runtime override (highest priority; wins over profile/env/default).
api.apply_config_override("GPU", "memory_mb", "8192");

// Validate the active configuration against the configured schema.
if (api.validate_configuration() == liz::ApiResult::Success) {
    // configuration conforms to schema
}

// Switch profiles without restarting the engine.
api.activate_configuration_profile("Benchmark");

// Reload the active configuration from its file path (dynamic reload).
api.reload_configuration();

// Advanced statistics.
auto a = api.advanced_configuration_statistics();
// a.schema_validations, a.reload_count, a.override_count,
// a.profile_switches, a.validation_failures
```

### Execution Profiles

| Profile | Description |
|---------|-------------|
| `Development` | Ergonomic defaults for local development |
| `Production` | Safe, conservative settings for production |
| `Benchmark` | Throughput-oriented settings for benchmarking |
| `Custom` | User-defined profile |

### Override Sources (by priority, highest first)

| Source | Description |
|--------|-------------|
| `Runtime` | Explicit override applied via `apply_config_override` |
| `Environment` | `LIZ_<SECTION>_<KEY>` process variables |
| `Profile` | Values contributed by the activated profile |
| `SchemaDefault` | Fallback default declared in the schema rule |

### Advanced Configuration Events

| Event | Description |
|-------|-------------|
| `ConfigSchemaValidated` | A configuration passed schema validation |
| `ConfigValidationFailed` | A configuration failed schema validation |
| `ConfigReloaded` | A configuration was reloaded from its source |
| `ConfigProfileActivated` | A profile-bound configuration was activated |
| `ConfigOverrideApplied` | A runtime override was applied |

