#include "engine/cloud/CloudSyncItem.h"

#include <chrono>
#include <random>
#include <sstream>

namespace liz {

// ── UUID generation (same pattern as Project/Workspace) ────────────────────

static std::string generate_cloud_uuid() {
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937_64 rng(static_cast<std::mt19937_64::result_type>(now));
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFFFFFFFFFFFFFF);
    auto val = dist(rng);
    std::ostringstream oss;
    oss << std::hex << (val & 0xFFFFFFFF);
    std::string hex = oss.str();
    if (hex.length() < 8) {
        hex.insert(0, 8 - hex.length(), '0');
    }
    return hex.substr(0, 8);
}

static std::uint64_t now_ms() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count());
}

// ── Constructors ─────────────────────────────────────────────────────────

CloudSyncItem::CloudSyncItem()
    : uuid_(generate_cloud_uuid())
    , type_(CloudSyncType::Settings)
    , state_(CloudSyncState::Pending)
    , created_at_ms_(now_ms())
    , synced_at_ms_(0)
{}

CloudSyncItem::CloudSyncItem(const std::string& name, CloudSyncType type)
    : uuid_(generate_cloud_uuid())
    , name_(name)
    , type_(type)
    , state_(CloudSyncState::Pending)
    , created_at_ms_(now_ms())
    , synced_at_ms_(0)
{}

// ── Move semantics ────────────────────────────────────────────────────────

CloudSyncItem::CloudSyncItem(CloudSyncItem&& other) noexcept
    : uuid_(std::move(other.uuid_))
    , name_(std::move(other.name_))
    , type_(other.type_)
    , state_(other.state_)
    , created_at_ms_(other.created_at_ms_)
    , synced_at_ms_(other.synced_at_ms_)
    , error_message_(std::move(other.error_message_))
{
    other.state_ = CloudSyncState::Pending;
    other.created_at_ms_ = 0;
    other.synced_at_ms_ = 0;
}

CloudSyncItem& CloudSyncItem::operator=(CloudSyncItem&& other) noexcept {
    if (this != &other) {
        uuid_ = std::move(other.uuid_);
        name_ = std::move(other.name_);
        type_ = other.type_;
        state_ = other.state_;
        created_at_ms_ = other.created_at_ms_;
        synced_at_ms_ = other.synced_at_ms_;
        error_message_ = std::move(other.error_message_);
        other.state_ = CloudSyncState::Pending;
        other.created_at_ms_ = 0;
        other.synced_at_ms_ = 0;
    }
    return *this;
}

// ── Mutators ──────────────────────────────────────────────────────────────

void CloudSyncItem::set_state(CloudSyncState state) {
    state_ = state;
    if (state == CloudSyncState::Synced ||
        state == CloudSyncState::Conflict ||
        state == CloudSyncState::Failed) {
        synced_at_ms_ = now_ms();
    }
}

void CloudSyncItem::set_synced_at(std::uint64_t ms) {
    synced_at_ms_ = ms;
}

void CloudSyncItem::set_error_message(const std::string& msg) {
    error_message_ = msg;
}

// ── to_string ────────────────────────────────────────────────────────────

std::string CloudSyncItem::to_string() const {
    std::ostringstream oss;
    oss << "CloudSyncItem["
        << "uuid=" << uuid_
        << ", name=\"" << name_ << "\""
        << ", type=" << cloud_sync_type_to_string(type_)
        << ", state=" << cloud_sync_state_to_string(state_)
        << "]";
    if (!error_message_.empty()) {
        oss << " error=\"" << error_message_ << "\"";
    }
    return oss.str();
}

} // namespace liz
