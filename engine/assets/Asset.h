#pragma once

#include "engine/assets/AssetType.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace liz {

/// A managed asset in the LIZ Engine.
///
/// Represents any reusable entity: videos, AI models, configurations,
/// shaders, plugins, textures, serialized tensors, projects, etc.
///
/// No filesystem access — purely in-memory bookkeeping.
/// All loading/unloading is simulated in this sprint.
class Asset {
public:
    /// Construct an asset with a name, type, and optional version.
    Asset(std::string name, AssetType type, std::string version = "1.0.0");

    /// Construct with explicit size.
    Asset(std::string name, AssetType type, std::size_t size_bytes,
          std::string version = "1.0.0");

    ~Asset() = default;

    // Non-copyable.
    Asset(const Asset&) = delete;
    Asset& operator=(const Asset&) = delete;

    // Movable.
    Asset(Asset&&) noexcept = default;
    Asset& operator=(Asset&&) noexcept = default;

    // ── Identity ──────────────────────────────────────────────────────────

    const std::string& uuid() const;
    const std::string& name() const;
    void               set_name(const std::string& name);
    AssetType          type() const;
    const std::string& version() const;
    void               set_version(const std::string& version);

    // ── Size ──────────────────────────────────────────────────────────────

    std::size_t size_bytes() const;
    void        set_size(std::size_t bytes);

    // ── State ─────────────────────────────────────────────────────────────

    AssetState state() const;
    void       set_state(AssetState state);

    // ── Reference counting ──────────────────────────────────────────────

    std::uint32_t ref_count() const;
    void          add_ref();
    bool          release();
    bool          is_orphan() const;

    // ── Timestamps (simulated) ───────────────────────────────────────────

    double created_at() const;
    double modified_at() const;
    void   touch();

    // ── Info ─────────────────────────────────────────────────────────────

    /// One-line summary for logging.
    std::string info() const;

private:
    static std::string generate_uuid();
    static double       now();

    std::string    uuid_;
    std::string    name_;
    AssetType      type_        = AssetType::Unknown;
    std::string    version_;
    std::size_t    size_bytes_  = 0;
    AssetState     state_       = AssetState::Unloaded;
    std::uint32_t  ref_count_   = 0;
    double         created_at_  = 0.0;
    double         modified_at_ = 0.0;
};

} // namespace liz
