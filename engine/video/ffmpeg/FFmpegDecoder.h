#pragma once

#include "engine/video/VideoFrame.h"
#include "engine/video/VideoDecoder.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace liz {

/// Result of opening a video file.
struct FFmpegOpenResult {
    bool        success      = false;
    std::string error_msg;
    std::uint32_t width       = 0;
    std::uint32_t height      = 0;
    double        fps         = 0.0;
    std::uint64_t frame_count = 0;
    double        duration_ms = 0.0;
    std::string   codec_name;
    bool          using_stub  = false;  ///< True when falling back to simulated decode
};

/// FFmpeg-based video decoder with intelligent stub fallback.
///
/// Behavior:
///   1. Attempt to open the file with FFmpeg (if available at compile time).
///   2. If FFmpeg is not linked or the file doesn't exist, fall back
///      to the simulated decoder from Sprint 2.
///   3. The caller never needs to know which path was taken.
///
/// In this sprint: always uses stub (FFmpeg not linked yet).
/// Future: #ifdef WITH_FFMPEG to enable real decoding.
class FFmpegDecoder {
public:
    FFmpegDecoder() = default;

    /// Attempt to open a video file.
    /// Returns metadata about the video (or error).
    FFmpegOpenResult open(const std::string& file_path);

    /// Decode the next frame (streaming model).
    /// Returns nullptr when no more frames are available.
    std::unique_ptr<VideoFrame> decode_next_frame();

    /// Decode all remaining frames at once.
    std::vector<VideoFrame> decode_all();

    /// Seek to a specific frame index (stub: just adjusts counter).
    bool seek_to_frame(std::uint32_t frame_index);

    /// Close the file and release resources.
    void close();

    /// True if a file is currently open.
    bool is_open() const;

    /// Metadata from the last open() call.
    const FFmpegOpenResult& open_result() const;

private:
    bool              open_       = false;
    bool              using_stub_ = false;
    FFmpegOpenResult  open_result_;
    std::uint32_t     current_frame_ = 0;
    std::uint32_t     total_frames_  = 0;

    /// Internal stub decoder used as fallback.
    std::unique_ptr<VideoDecoder> stub_decoder_;
    std::vector<VideoFrame>       stub_frames_;
    std::size_t                   stub_index_ = 0;
};

} // namespace liz