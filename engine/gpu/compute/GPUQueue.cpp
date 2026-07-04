#include "engine/gpu/compute/GPUQueue.h"

namespace liz {

void GPUQueue::push(std::shared_ptr<GPUCommand> cmd) {
    queue_.push(std::move(cmd));
}

std::shared_ptr<GPUCommand> GPUQueue::pop() {
    if (queue_.empty()) return nullptr;
    auto cmd = std::move(queue_.front());
    queue_.pop();
    return cmd;
}

bool GPUQueue::empty() const {
    return queue_.empty();
}

std::size_t GPUQueue::size() const {
    return queue_.size();
}

} // namespace liz