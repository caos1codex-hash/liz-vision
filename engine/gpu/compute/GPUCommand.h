#pragma once

#include <cstdint>
#include <string>

namespace liz {

// -- Enumerations -------------------------------------------------------------

enum class GPUCommandType {
    UploadFrame,
    RunInference,
    CopyBuffer
};

enum class GPUCommandStatus {
    Pending,
    Running,
    Completed,
    Failed
};

// -- Base command -------------------------------------------------------------

/// A single GPU command.  execute() simulates the operation with a cout log.
class GPUCommand {
public:
    GPUCommand(GPUCommandType type, std::string name);
    virtual ~GPUCommand() = default;

    /// Run the command.  Sets status to Completed or Failed.
    void execute();

    GPUCommandType   type()   const;
    GPUCommandStatus status() const;
    const std::string& name() const;

protected:
    /// Subclass implements the actual simulated work.
    /// Return true on success, false on failure.
    virtual bool on_execute() = 0;

private:
    GPUCommandType   type_;
    GPUCommandStatus status_ = GPUCommandStatus::Pending;
    std::string      name_;
};

// -- Concrete commands --------------------------------------------------------

/// Simulates uploading a video frame to GPU memory.
class UploadFrameCommand final : public GPUCommand {
public:
    explicit UploadFrameCommand(std::string name, std::size_t frame_size = 0);

protected:
    bool on_execute() override;

private:
    std::size_t frame_size_;
};

/// Simulates running an AI inference kernel on the GPU.
class RunInferenceCommand final : public GPUCommand {
public:
    RunInferenceCommand(std::string name,
                        std::string model_name,
                        std::string operation);

protected:
    bool on_execute() override;

private:
    std::string model_name_;
    std::string operation_;
};

/// Simulates copying a GPU buffer.
class CopyBufferCommand final : public GPUCommand {
public:
    CopyBufferCommand(std::string name,
                      std::uint64_t source_id,
                      std::size_t size);

protected:
    bool on_execute() override;

private:
    std::uint64_t source_id_;
    std::size_t   size_;
};

} // namespace liz