#pragma once

#include "engine/cloud/CloudTypes.h"

#include <cstdint>
#include <string>

namespace liz {

/// A single item in the cloud sync queue.
///
/// Tracks sync state, type, timestamps, and an optional error message
/// for conflict/failed states.
class CloudSyncItem {
public:
    CloudSyncItem();

    /// Construct a sync item with a name and type.
    CloudSyncItem(const std::string& name, CloudSyncType type);

    // Non-copyable, movable.
    CloudSyncItem(const CloudSyncItem&) = delete;
    CloudSyncItem& operator=(const CloudSyncItem&) = delete;
    CloudSyncItem(CloudSyncItem&& other) noexcept;
    CloudSyncItem& operator=(CloudSyncItem&& other) noexcept;

    // ── Accessors ────────────────────────────────────────────────────────

    const std::string& uuid() const { return uuid_; }
    const std::string& name() const { return name_; }
    CloudSyncType type() const { return type_; }
    CloudSyncState state() const { return state_; }

    std::uint64_t created_at_ms() const { return created_at_ms_; }
    std::uint64_t synced_at_ms() const { return synced_at_ms_; }

    const std::string& error_message() const { return error_message_; }

    // ── Mutators ─────────────────────────────────────────────────────────

    void set_state(CloudSyncState state);
    void set_synced_at(std::uint64_t ms);
    void set_error_message(const std::string& msg);

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    std::string    uuid_;
    std::string    name_;
    CloudSyncType  type_;
    CloudSyncState state_;
    std::uint64_t  created_at_ms_;
    std::uint64_t  synced_at_ms_;
    std::string    error_message_;
};

} // namespace liz
