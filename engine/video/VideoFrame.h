#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace liz {

/// Represents a single decoded video frame in memory.
///
/// Stores raw pixel data (RGB24 by convention) along with
/// dimensions, timing, and optional metadata.
class VideoFrame {
public:
    /// Construct an empty frame (default).
    VideoFrame() = default;

    /// Construct a frame with full metadata.
    VideoFrame(std::uint32_t frame_id,
               std::uint32_t width,
               std::uint32_t height,
               double timestamp_ms,
               std::vector<std::uint8_t> raw_data);

    // ── Accessors ────────────────────────────────────────────────────────────
    std::uint32_t                     frame_id() const;
    std::uint32_t                     width() const;
    std::uint32_t                     height() const;
    double                            timestamp_ms() const;
    const std::vector<std::uint8_t>&  data() const;
    std::vector<std::uint8_t>&        data();
    std::size_t                       data_size_bytes() const;

    /// True when the frame has no pixel data allocated.
    bool is_empty() const;

    // ── Metadata ─────────────────────────────────────────────────────────────
    /// Get a metadata value.  Returns std::nullopt if key doesn't exist.
    std::string get_meta(std::string_view key) const;

    /// Set a metadata key-value pair.
    void set_meta(std::string key, std::string value);

    /// Check whether a metadata key exists.
    bool has_meta(std::string_view key) const;

private:
    std::uint32_t frame_id_      = 0;
    std::uint32_t width_         = 0;
    std::uint32_t height_        = 0;
    double        timestamp_ms_  = 0.0;
    std::vector<std::uint8_t> raw_data_;
    std::map<std::string, std::string> metadata_;
};

} // namespace liz