#include "engine/video/ffmpeg/FFmpegDecoder.h"
#include "engine/core/Logger.h"

#include <memory>
#include <sstream>

namespace liz {

// ── Open ─────────────────────────────────────────────────────────────────────
FFmpegOpenResult FFmpegDecoder::open(const std::string& file_path) {
    close();
    open_result_ = FFmpegOpenResult{};

    LIZ_INFO("FFmpegDecoder: attempting to open '" + file_path + "'");

    // ── Try real FFmpeg ──────────────────────────────────────────────────────
    // #ifdef WITH_FFMPEG
    //   ... real AVFormatContext / AVCodecContext open ...
    //   if (success) { open_ = true; using_stub_ = false; return open_result_; }
    // #endif

    // ── Fallback: stub decoder ───────────────────────────────────────────────
    LIZ_INFO("FFmpegDecoder: FFmpeg not available — falling back to simulated decoder");

    using_stub_ = true;

    DecodeRequest req;
    req.source_path    = file_path;
    req.target_width   = 320;   // default stub resolution
    req.target_height  = 240;
    req.fps            = 24.0;
    req.max_frames     = 10;

    stub_decoder_ = std::make_unique<VideoDecoder>();
    auto result = stub_decoder_->decode(req);

    if (!result.success) {
        open_result_.success   = false;
        open_result_.error_msg = result.error_msg;
        LIZ_ERROR("FFmpegDecoder: stub decode failed — " + result.error_msg);
        return open_result_;
    }

    stub_frames_ = std::move(result.frames);
    stub_index_  = 0;
    current_frame_ = 0;
    total_frames_  = static_cast<std::uint32_t>(stub_frames_.size());

    // Populate metadata from first frame.
    if (!stub_frames_.empty()) {
        open_result_.width  = stub_frames_[0].width();
        open_result_.height = stub_frames_[0].height();
    }
    open_result_.fps         = req.fps;
    open_result_.frame_count = total_frames_;
    open_result_.duration_ms = static_cast<double>(total_frames_) * 1000.0 / req.fps;
    open_result_.codec_name  = "stub_rgb24";
    open_result_.using_stub  = true;
    open_result_.success     = true;

    open_ = true;

    {
        std::ostringstream oss;
        oss << "FFmpegDecoder: opened (stub) — "
            << open_result_.width << "x" << open_result_.height
            << " @ " << open_result_.fps << " fps, "
            << open_result_.frame_count << " frames, "
            << open_result_.duration_ms << " ms";
        LIZ_INFO(oss.str());
    }

    return open_result_;
}

// ── Streaming decode ─────────────────────────────────────────────────────────
std::unique_ptr<VideoFrame> FFmpegDecoder::decode_next_frame() {
    if (!open_ || stub_index_ >= stub_frames_.size()) {
        return nullptr;
    }

    auto frame = std::make_unique<VideoFrame>(std::move(stub_frames_[stub_index_]));
    ++stub_index_;
    ++current_frame_;

    std::ostringstream oss;
    oss << "FFmpegDecoder: streamed frame #" << frame->frame_id()
        << " [" << frame->width() << "x" << frame->height() << "]"
        << " ts=" << frame->timestamp_ms() << "ms"
        << " (remaining: " << (stub_frames_.size() - stub_index_) << ")";
    LIZ_TRACE(oss.str());

    return frame;
}

std::vector<VideoFrame> FFmpegDecoder::decode_all() {
    std::vector<VideoFrame> all;
    while (auto frame = decode_next_frame()) {
        all.push_back(std::move(*frame));
    }

    std::ostringstream oss;
    oss << "FFmpegDecoder: streamed all " << all.size() << " frames";
    LIZ_INFO(oss.str());

    return all;
}

// ── Seek ──────────────────────────────────────────────────────────────────────
bool FFmpegDecoder::seek_to_frame(std::uint32_t frame_index) {
    if (!open_) return false;
    if (frame_index >= stub_frames_.size()) {
        LIZ_WARN("FFmpegDecoder: seek out of range");
        return false;
    }

    stub_index_ = frame_index;
    current_frame_ = frame_index;

    std::ostringstream oss;
    oss << "FFmpegDecoder: seeked to frame #" << frame_index;
    LIZ_DEBUG(oss.str());
    return true;
}

// ── Close ─────────────────────────────────────────────────────────────────────
void FFmpegDecoder::close() {
    if (open_) {
        LIZ_INFO("FFmpegDecoder: closed");
    }
    open_ = false;
    using_stub_ = false;
    stub_decoder_.reset();
    stub_frames_.clear();
    stub_index_ = 0;
    current_frame_ = 0;
    total_frames_ = 0;
    open_result_ = FFmpegOpenResult{};
}

bool FFmpegDecoder::is_open() const { return open_; }

const FFmpegOpenResult& FFmpegDecoder::open_result() const { return open_result_; }

} // namespace liz