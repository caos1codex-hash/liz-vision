#include "engine/video/VideoDecoder.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// ── Public API ────────────────────────────────────────────────────────────────
DecodeResult VideoDecoder::decode(const DecodeRequest& request) {
    DecodeResult result;
    result.frames.reserve(request.max_frames);

    std::ostringstream oss;
    oss << "VideoDecoder: starting simulated decode — source='"
        << request.source_path << "' "
        << request.target_width << "x" << request.target_height
        << " @ " << request.fps << " fps, "
        << request.max_frames << " frames";
    LIZ_INFO(oss.str());

    if (request.max_frames == 0) {
        LIZ_WARN("VideoDecoder: max_frames is 0, nothing to decode");
        result.success = true;
        return result;
    }

    if (request.target_width == 0 || request.target_height == 0) {
        result.success = false;
        result.error_msg = "Invalid dimensions (0x0)";
        LIZ_ERROR("VideoDecoder: " + result.error_msg);
        return result;
    }

    double frame_interval_ms = 1000.0 / request.fps;

    for (std::uint32_t i = 0; i < request.max_frames; ++i) {
        double ts = static_cast<double>(i) * frame_interval_ms;
        auto frame = generate_frame(i, request.target_width, request.target_height, ts);
        result.frames.push_back(std::move(frame));
    }

    result.total_frames_decoded = static_cast<std::uint32_t>(result.frames.size());
    result.duration_ms = static_cast<double>(result.total_frames_decoded) * frame_interval_ms;
    result.success = true;

    oss.str("");
    oss << "VideoDecoder: decoded " << result.total_frames_decoded
        << " frames (" << result.duration_ms << " ms total)";
    LIZ_INFO(oss.str());

    return result;
}

// ── Private ───────────────────────────────────────────────────────────────────
VideoFrame VideoDecoder::generate_frame(std::uint32_t frame_id,
                                         std::uint32_t width,
                                         std::uint32_t height,
                                         double timestamp_ms) {
    // RGB24: 3 bytes per pixel
    std::size_t pixel_count = static_cast<std::size_t>(width) * height;
    std::size_t data_size = pixel_count * 3;

    std::vector<std::uint8_t> raw_data(data_size);

    // Fill with a simple horizontal gradient based on x-position
    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 3;
            raw_data[idx + 0] = static_cast<std::uint8_t>((x * 255) / width);            // R
            raw_data[idx + 1] = static_cast<std::uint8_t>((y * 255) / height);           // G
            raw_data[idx + 2] = static_cast<std::uint8_t>(((x + y) * 128) / (width + height)); // B
        }
    }

    VideoFrame frame(frame_id, width, height, timestamp_ms, std::move(raw_data));
    frame.set_meta("format", "RGB24");
    frame.set_meta("decoder", "simulated");
    frame.set_meta("pixel_count", std::to_string(pixel_count));

    return frame;
}

} // namespace liz