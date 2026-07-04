#include "engine/video/VideoFrame.h"

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────────────
VideoFrame::VideoFrame(std::uint32_t frame_id,
                       std::uint32_t width,
                       std::uint32_t height,
                       double timestamp_ms,
                       std::vector<std::uint8_t> raw_data)
    : frame_id_(frame_id)
    , width_(width)
    , height_(height)
    , timestamp_ms_(timestamp_ms)
    , raw_data_(std::move(raw_data))
{}

// ── Accessors ─────────────────────────────────────────────────────────────────
std::uint32_t VideoFrame::frame_id() const { return frame_id_; }
std::uint32_t VideoFrame::width() const    { return width_; }
std::uint32_t VideoFrame::height() const   { return height_; }
double        VideoFrame::timestamp_ms() const { return timestamp_ms_; }

const std::vector<std::uint8_t>& VideoFrame::data() const { return raw_data_; }
std::vector<std::uint8_t>&       VideoFrame::data()       { return raw_data_; }

std::size_t VideoFrame::data_size_bytes() const {
    return raw_data_.size();
}

bool VideoFrame::is_empty() const {
    return raw_data_.empty();
}

// ── Metadata ──────────────────────────────────────────────────────────────────
std::string VideoFrame::get_meta(std::string_view key) const {
    auto it = metadata_.find(std::string(key));
    if (it != metadata_.end()) {
        return it->second;
    }
    return {};
}

void VideoFrame::set_meta(std::string key, std::string value) {
    metadata_[std::move(key)] = std::move(value);
}

bool VideoFrame::has_meta(std::string_view key) const {
    return metadata_.find(std::string(key)) != metadata_.end();
}

} // namespace liz