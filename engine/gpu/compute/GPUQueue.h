#pragma once

#include "engine/gpu/compute/GPUCommand.h"

#include <cstddef>
#include <memory>
#include <queue>

namespace liz {

/// Simple FIFO command queue.  No mutex, no threads.
/// Push commands in, pop them out in order.
class GPUQueue {
public:
    void push(std::shared_ptr<GPUCommand> cmd);
    std::shared_ptr<GPUCommand> pop();
    bool empty() const;
    std::size_t size() const;

private:
    std::queue<std::shared_ptr<GPUCommand>> queue_;
};

} // namespace liz