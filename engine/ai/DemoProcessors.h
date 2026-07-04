#pragma once

#include "engine/ai/AIProcessor.h"
#include "engine/core/Logger.h"

#include <sstream>
#include <string>

namespace liz {

/// Demo upscaler processor — simulates 1080p → 4K upscaling.
/// Implements both AIProcessor and PluginInterface to demonstrate
/// the bridge between the AI and Plugin subsystems.
class DemoUpscalerProcessor : public AIProcessor {
public:
    DemoUpscalerProcessor()
        : name_("Demo Upscaler"), type_(AIProcessorType::Upscaler) {}

    // ── AIProcessor interface ─────────────────────────────────────────────────
    std::string_view name() const override { return name_; }
    AIProcessorType  type() const override { return type_; }

    VideoFrame process_frame(const VideoFrame& input) override {
        // Simulated: double resolution, copy data, tag metadata.
        auto out_w = input.width() * 2;
        auto out_h = input.height() * 2;
        std::size_t out_size = static_cast<std::size_t>(out_w) * out_h * 3;

        VideoFrame output(input.frame_id(), out_w, out_h,
                          input.timestamp_ms(),
                          std::vector<std::uint8_t>(out_size, 0));

        output.set_meta("upscaled_from", std::to_string(input.width()) + "x" + std::to_string(input.height()));
        output.set_meta("processor", std::string(name_));
        return output;
    }

    void on_init() override {
        LIZ_INFO("DemoUpscalerProcessor: initialized (simulated)");
    }

    void on_cleanup() override {
        LIZ_INFO("DemoUpscalerProcessor: cleaned up");
    }

private:
    std::string    name_;
    AIProcessorType type_;
};

/// Demo interpolator processor — simulates 24fps → 120fps.
class DemoInterpolatorProcessor : public AIProcessor {
public:
    DemoInterpolatorProcessor()
        : name_("Demo Interpolator"), type_(AIProcessorType::Interpolator) {}

    std::string_view name() const override { return name_; }
    AIProcessorType  type() const override { return type_; }

    VideoFrame process_frame(const VideoFrame& input) override {
        // Simulated: same frame, marked as interpolated.
        VideoFrame output(input);
        output.set_meta("interpolated", "true");
        output.set_meta("processor", std::string(name_));
        return output;
    }

    void on_init() override {
        LIZ_INFO("DemoInterpolatorProcessor: initialized (simulated)");
    }

    void on_cleanup() override {
        LIZ_INFO("DemoInterpolatorProcessor: cleaned up");
    }

private:
    std::string    name_;
    AIProcessorType type_;
};

} // namespace liz