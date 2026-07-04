#include "engine/ai/inference/ModelLoader.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

namespace liz {

// -- Simulated concrete models (internal to ModelLoader) ------------------------

namespace {

/// Simulated upscaler model — doubles resolution, marks metadata.
class SimUpscalerModel final : public liz::Model {
public:
    SimUpscalerModel(const std::string& name, const std::string& version)
        : name_(name), version_(version) {}

    std::string_view name()    const override { return name_; }
    std::string_view version() const override { return version_; }
    ModelType        type()    const override { return ModelType::Upscaler; }

    TensorFormat input_format()  const override {
        return {"RGB24", 3, "uint8"};
    }
    TensorFormat output_format() const override {
        return {"RGB24", 3, "uint8"};
    }

    VideoFrame run(const VideoFrame& input) override {
        auto out_w = input.width() * 2;
        auto out_h = input.height() * 2;
        std::size_t out_size = static_cast<std::size_t>(out_w) * out_h * 3;

        VideoFrame output(input.frame_id(), out_w, out_h,
                          input.timestamp_ms(),
                          std::vector<std::uint8_t>(out_size, 0));

        output.set_meta("model", std::string(name_));
        output.set_meta("model_version", std::string(version_));
        output.set_meta("enhancement", "upscaled_2x");
        output.set_meta("upscaled_from",
                        std::to_string(input.width()) + "x" +
                        std::to_string(input.height()));

        std::ostringstream oss;
        oss << "running model " << name_
            << " on Frame #" << input.frame_id()
            << " [" << input.width() << "x" << input.height()
            << " -> " << out_w << "x" << out_h << "]";
        LIZ_INFO(oss.str());

        return output;
    }

    void on_load() override {
        std::ostringstream oss;
        oss << "loading model: " << name_ << " v" << version_;
        LIZ_INFO(oss.str());
    }

    void on_unload() override {
        std::ostringstream oss;
        oss << "unloading model: " << name_;
        LIZ_INFO(oss.str());
    }

private:
    std::string name_;
    std::string version_;
};

/// Simulated interpolator model — marks frame as interpolated.
class SimInterpolatorModel final : public liz::Model {
public:
    SimInterpolatorModel(const std::string& name, const std::string& version)
        : name_(name), version_(version) {}

    std::string_view name()    const override { return name_; }
    std::string_view version() const override { return version_; }
    ModelType        type()    const override { return ModelType::Interpolator; }

    TensorFormat input_format()  const override {
        return {"RGB24", 3, "uint8"};
    }
    TensorFormat output_format() const override {
        return {"RGB24", 3, "uint8"};
    }

    VideoFrame run(const VideoFrame& input) override {
        VideoFrame output(input);
        output.set_meta("model", std::string(name_));
        output.set_meta("model_version", std::string(version_));
        output.set_meta("enhancement", "interpolated");
        output.set_meta("interpolated", "true");

        std::ostringstream oss;
        oss << "running model " << name_
            << " on Frame #" << input.frame_id()
            << " [interpolation simulated]";
        LIZ_INFO(oss.str());

        return output;
    }

    void on_load() override {
        std::ostringstream oss;
        oss << "loading model: " << name_ << " v" << version_;
        LIZ_INFO(oss.str());
    }

    void on_unload() override {
        std::ostringstream oss;
        oss << "unloading model: " << name_;
        LIZ_INFO(oss.str());
    }

private:
    std::string name_;
    std::string version_;
};

/// Simulated denoiser model — marks frame as denoised.
class SimDenoiserModel final : public liz::Model {
public:
    SimDenoiserModel(const std::string& name, const std::string& version)
        : name_(name), version_(version) {}

    std::string_view name()    const override { return name_; }
    std::string_view version() const override { return version_; }
    ModelType        type()    const override { return ModelType::Denoiser; }

    TensorFormat input_format()  const override {
        return {"RGB24", 3, "uint8"};
    }
    TensorFormat output_format() const override {
        return {"RGB24", 3, "uint8"};
    }

    VideoFrame run(const VideoFrame& input) override {
        VideoFrame output(input);
        output.set_meta("model", std::string(name_));
        output.set_meta("model_version", std::string(version_));
        output.set_meta("enhancement", "denoised");
        output.set_meta("noise_reduction", "simulated");

        std::ostringstream oss;
        oss << "running model " << name_
            << " on Frame #" << input.frame_id()
            << " [denoising simulated]";
        LIZ_INFO(oss.str());

        return output;
    }

    void on_load() override {
        std::ostringstream oss;
        oss << "loading model: " << name_ << " v" << version_;
        LIZ_INFO(oss.str());
    }

    void on_unload() override {
        std::ostringstream oss;
        oss << "unloading model: " << name_;
        LIZ_INFO(oss.str());
    }

private:
    std::string name_;
    std::string version_;
};

} // anonymous namespace

// -- ModelLoader implementation -------------------------------------------------

void ModelLoader::populate_builtin_models() {
    catalog_.push_back({"liz_upscaler_v1",     ModelType::Upscaler,
                        "1.0.0", {"RGB24", 3, "uint8"}, {"RGB24", 3, "uint8"}});
    catalog_.push_back({"liz_interpolator_v1", ModelType::Interpolator,
                        "1.0.0", {"RGB24", 3, "uint8"}, {"RGB24", 3, "uint8"}});
    catalog_.push_back({"liz_denoiser_v1",     ModelType::Denoiser,
                        "1.0.0", {"RGB24", 3, "uint8"}, {"RGB24", 3, "uint8"}});
}

void ModelLoader::discover() {
    if (catalog_.empty()) {
        populate_builtin_models();
    }

    LIZ_INFO("ModelLoader: discovering available models...");

    for (const auto& desc : catalog_) {
        std::ostringstream oss;
        oss << "  found model: '" << desc.name
            << "' [" << model_type_to_string(desc.type)
            << "] v" << desc.version
            << " in=" << desc.input_fmt.name
            << " out=" << desc.output_fmt.name;
        LIZ_INFO(oss.str());
    }

    {
        std::ostringstream oss;
        oss << "ModelLoader: " << catalog_.size() << " model(s) discovered";
        LIZ_INFO(oss.str());
    }
}

bool ModelLoader::has_model(std::string_view name) const {
    for (const auto& desc : catalog_) {
        if (desc.name == name) return true;
    }
    return false;
}

std::vector<std::string> ModelLoader::available_models() const {
    std::vector<std::string> names;
    names.reserve(catalog_.size());
    for (const auto& desc : catalog_) {
        names.push_back(desc.name);
    }
    return names;
}

std::unique_ptr<Model> ModelLoader::load(std::string_view name) {
    for (const auto& desc : catalog_) {
        if (desc.name == name) {
            std::unique_ptr<Model> model;

            switch (desc.type) {
                case ModelType::Upscaler:
                    model = std::make_unique<SimUpscalerModel>(
                        desc.name, desc.version);
                    break;
                case ModelType::Interpolator:
                    model = std::make_unique<SimInterpolatorModel>(
                        desc.name, desc.version);
                    break;
                case ModelType::Denoiser:
                    model = std::make_unique<SimDenoiserModel>(
                        desc.name, desc.version);
                    break;
            }

            if (model) {
                model->on_load();
            }
            return model;
        }
    }

    std::ostringstream oss;
    oss << "ModelLoader: model '" << name << "' not found in catalog";
    LIZ_ERROR(oss.str());
    return nullptr;
}

std::size_t ModelLoader::model_count() const {
    return catalog_.size();
}

} // namespace liz