#include "engine/assets/Asset.h"

#include <chrono>
#include <cstdint>
#include <sstream>

namespace liz {

// ── Constructors ──────────────────────────────────────────────────────────

Asset::Asset(std::string name, AssetType type, std::string version)
    : uuid_(generate_uuid())
    , name_(std::move(name))
    , type_(type)
    , version_(std::move(version))
    , created_at_(now())
    , modified_at_(created_at_) {}

Asset::Asset(std::string name, AssetType type, std::size_t size_bytes,
             std::string version)
    : uuid_(generate_uuid())
    , name_(std::move(name))
    , type_(type)
    , version_(std::move(version))
    , size_bytes_(size_bytes)
    , created_at_(now())
    , modified_at_(created_at_) {}

// ── Identity ──────────────────────────────────────────────────────────────

const std::string& Asset::uuid() const { return uuid_; }
const std::string& Asset::name() const { return name_; }
AssetType          Asset::type() const { return type_; }
const std::string& Asset::version() const { return version_; }

void Asset::set_name(const std::string& name) {
    name_ = name;
    touch();
}

void Asset::set_version(const std::string& version) {
    version_ = version;
    touch();
}

// ── Size ──────────────────────────────────────────────────────────────────

std::size_t Asset::size_bytes() const { return size_bytes_; }

void Asset::set_size(std::size_t bytes) {
    size_bytes_ = bytes;
    touch();
}

// ── State ──────────────────────────────────────────────────────────────────

AssetState Asset::state() const { return state_; }

void Asset::set_state(AssetState state) {
    state_ = state;
    touch();
}

// ── Reference counting ──────────────────────────────────────────────────

std::uint32_t Asset::ref_count() const { return ref_count_; }

void Asset::add_ref() {
    ++ref_count_;
}

bool Asset::release() {
    if (ref_count_ == 0) {
        return false;
    }
    --ref_count_;
    return ref_count_ > 0;
}

bool Asset::is_orphan() const { return ref_count_ == 0; }

// ── Timestamps ───────────────────────────────────────────────────────────

double Asset::created_at() const { return created_at_; }
double Asset::modified_at() const { return modified_at_; }

void Asset::touch() {
    modified_at_ = now();
}

// ── Info ─────────────────────────────────────────────────────────────────

std::string Asset::info() const {
    std::ostringstream oss;
    oss << "Asset[" << name_
        << "] type=" << asset_type_to_string(type_)
        << " version=" << version_
        << " state=" << asset_state_to_string(state_)
        << " size=" << size_bytes_
        << " refs=" << ref_count_
        << " uuid=" << uuid_;
    return oss.str();
}

// ── Private ───────────────────────────────────────────────────────────────

std::string Asset::generate_uuid() {
    static std::uint64_t counter = 0;
    ++counter;

    char buf[32];
    std::snprintf(buf, sizeof(buf), "ast_%016llx",
                  static_cast<unsigned long long>(counter));
    return std::string(buf);
}

double Asset::now() {
    auto tp = std::chrono::steady_clock::now();
    auto dur = tp.time_since_epoch();
    return std::chrono::duration<double>(dur).count();
}

} // namespace liz
