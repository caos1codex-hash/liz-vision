#include "engine/gpu/compute/GPUCommand.h"

#include <iostream>
#include <sstream>

namespace liz {

// -- Base command -------------------------------------------------------------

GPUCommand::GPUCommand(GPUCommandType type, std::string name)
    : type_(type), name_(std::move(name)) {}

GPUCommandType   GPUCommand::type()   const { return type_; }
GPUCommandStatus GPUCommand::status() const { return status_; }
const std::string& GPUCommand::name() const { return name_; }

void GPUCommand::execute() {
    status_ = GPUCommandStatus::Running;

    bool ok = on_execute();

    status_ = ok ? GPUCommandStatus::Completed : GPUCommandStatus::Failed;

    std::cout << "  [GPU] " << name_
              << " -> " << (ok ? "Completed" : "Failed")
              << std::endl;
}

// -- UploadFrameCommand -------------------------------------------------------

UploadFrameCommand::UploadFrameCommand(std::string name, std::size_t frame_size)
    : GPUCommand(GPUCommandType::UploadFrame, std::move(name)),
      frame_size_(frame_size) {}

bool UploadFrameCommand::on_execute() {
    std::cout << "  [GPU] simulating upload: " << name()
              << " (" << frame_size_ << " bytes)" << std::endl;
    return true;
}

// -- RunInferenceCommand -------------------------------------------------------

RunInferenceCommand::RunInferenceCommand(std::string name,
                                           std::string model_name,
                                           std::string operation)
    : GPUCommand(GPUCommandType::RunInference, std::move(name)),
      model_name_(std::move(model_name)),
      operation_(std::move(operation)) {}

bool RunInferenceCommand::on_execute() {
    std::cout << "  [GPU] simulating inference: model='"
              << model_name_ << "' operation='" << operation_
              << "' [" << name() << "]" << std::endl;
    return true;
}

// -- CopyBufferCommand ---------------------------------------------------------

CopyBufferCommand::CopyBufferCommand(std::string name,
                                       std::uint64_t source_id,
                                       std::size_t size)
    : GPUCommand(GPUCommandType::CopyBuffer, std::move(name)),
      source_id_(source_id),
      size_(size) {}

bool CopyBufferCommand::on_execute() {
    std::cout << "  [GPU] simulating copy: source=#" << source_id_
              << " size=" << size_ << " bytes [" << name() << "]"
              << std::endl;
    return true;
}

} // namespace liz