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
