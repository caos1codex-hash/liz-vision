#pragma once

#include "engine/video/VideoFrame.h"

#include <cstdint>
#include <string>
#include <vector>

namespace liz {

/// Parameters that describe how to decode a video source.
struct DecodeRequest {
    std::string source_path;     ///< Input file path (simulated — not read from disk)
    std::uint32_t target_width   = 1920;
    std::uint32_t target_height  = 1080;
    double        fps            = 30.0;
    std::uint32_t max_frames     = 10;   ///< Number of fake frames to generate
};

/// Result of a decode operation.
struct DecodeResult {
    bool                     success    = false;
    std::string              error_msg;
    std::vector<VideoFrame>  frames;
    std::uint32_t            total_frames_decoded = 0;
    double                   duration_ms = 0.0;
};

/// Simulated video decoder.
///
/// Generates fake VideoFrame objects without reading any real file.
/// Each frame contains dummy RGB data of the requested resolution.
///
/// This is a STUB — no FFmpeg, no GPU, no real decoding.
/// Future sprints will replace this with an actual decoder backend.
class VideoDecoder {
public:
    VideoDecoder() = default;

    /// Decode (simulate) frames according to the given request.
    DecodeResult decode(const DecodeRequest& request);

private:
    /// Generate a single fake frame filled with a gradient pattern.
    VideoFrame generate_frame(std::uint32_t frame_id,
                              std::uint32_t width,
                              std::uint32_t height,
                              double timestamp_ms);
};

} // namespace liz