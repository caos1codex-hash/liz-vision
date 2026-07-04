#pragma once

#include "engine/pipeline/PipelineTypes.h"

#include <string>

namespace liz {

/// A directed connection between two nodes in a pipeline graph.
class PipelineEdge {
public:
    PipelineEdge(std::string source_uuid, std::string dest_uuid,
                 ConnectionType type = ConnectionType::Data);

    ~PipelineEdge() = default;

    // Non-copyable.
    PipelineEdge(const PipelineEdge&) = delete;
    PipelineEdge& operator=(const PipelineEdge&) = delete;

    // Movable.
    PipelineEdge(PipelineEdge&&) noexcept = default;
    PipelineEdge& operator=(PipelineEdge&&) noexcept = default;

    // ── Identity ──────────────────────────────────────────────────────

    const std::string& uuid() const;
    const std::string& source_uuid() const;
    const std::string& dest_uuid() const;

    // ── Type ─────────────────────────────────────────────────────────

    ConnectionType type() const;
    void           set_type(ConnectionType type);

    // ── State ─────────────────────────────────────────────────────────

    bool is_active() const;
    void activate();
    void deactivate();

    // ── Info ───────────────────────────────────────────────────────────

    /// One-line summary for logging.
    std::string info() const;

private:
    static std::string generate_uuid();

    std::string    uuid_;
    std::string    source_uuid_;
    std::string    dest_uuid_;
    ConnectionType type_     = ConnectionType::Data;
    bool           active_   = true;
};

} // namespace liz
