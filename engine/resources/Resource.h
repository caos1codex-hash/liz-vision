#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace liz {

// -- ResourceType -------------------------------------------------------------

/// Semantic category of a managed resource.
enum class ResourceType {
    Unknown,
    Video,
    Frame,
    Tensor,
    AIModel,
    GPUBuffer,
    Plugin,
    Configuration,
    Project,
    Temporary
};

/// Convert ResourceType to a readable string.
const char* resource_type_to_string(ResourceType type);

// -- ResourceState ------------------------------------------------------------

/// Lifecycle state of a resource.
enum class ResourceState {
    Loaded,
    Unloaded,
    Error
};

/// Convert ResourceState to a readable string.
const char* resource_state_to_string(ResourceState state);

// -- Resource -----------------------------------------------------------------

/// A managed resource in the engine.
///
/// Represents any entity the engine tracks: video files, frames, tensors,
/// AI models, GPU buffers, plugins, configurations, etc.
///
/// No file I/O — purely in-memory bookkeeping.
class Resource {
public:
    /// Construct a resource with a name and type.
    /// UUID, timestamps, and counters are auto-generated.
    Resource(std::string name, ResourceType type);

    /// Construct with explicit size in bytes.
    Resource(std::string name, ResourceType type, std::size_t size_bytes);

    // -- Identity --------------------------------------------------------------
    const std::string& uuid() const;
    const std::string& name() const;
    void               set_name(const std::string& name);
    ResourceType       type() const;

    // -- Size ------------------------------------------------------------------
    std::size_t size_bytes() const;
    void        set_size(std::size_t bytes);

    // -- State -----------------------------------------------------------------
    ResourceState state() const;
    void          set_state(ResourceState state);

    // -- Reference counting ----------------------------------------------------
    std::uint32_t ref_count() const;
    void          add_ref();
    void          release();
    bool          is_orphan() const;

    // -- Timestamps (simulated) ------------------------------------------------
    double created_at() const;
    double last_accessed() const;
    void   touch();

    // -- Info ------------------------------------------------------------------
    /// One-line summary for logging.
    std::string info() const;

private:
    static std::string generate_uuid();
    static double       now();

    std::string    uuid_;
    std::string    name_;
    ResourceType   type_;
    std::size_t    size_bytes_    = 0;
    ResourceState  state_         = ResourceState::Loaded;
    std::uint32_t  ref_count_     = 0;
    double         created_at_    = 0.0;
    double         last_accessed_ = 0.0;

    static std::uint64_t uuid_counter_;
};

} // namespace liz