#include "engine/performance/FrameQueue.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// -- Constructor / Destructor --------------------------------------------------
FrameQueue::FrameQueue(std::size_t capacity)
    : capacity_(capacity) {
    std::ostringstream oss;
    oss << "FrameQueue: created with capacity=" << capacity;
    LIZ_DEBUG(oss.str());
}

FrameQueue::~FrameQueue() {
    close();
}

// -- Producer API --------------------------------------------------------------
bool FrameQueue::push(VideoFrame frame) {
    std::unique_lock<std::mutex> lock(mutex_);

    if (closed_) return false;

    // Backpressure: wait until there is room.
    cv_not_full_.wait(lock, [this] {
        return closed_ || queue_.size() < capacity_;
    });

    if (closed_) return false;

    bool was_full = queue_.size() >= capacity_;
    queue_.push(std::move(frame));
    ++total_pushed_;

    if (was_full) {
        ++bp_events_;
    }

    cv_not_empty_.notify_one();
    return true;
}

bool FrameQueue::try_push(VideoFrame frame) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (closed_ || queue_.size() >= capacity_) {
        return false;
    }

    queue_.push(std::move(frame));
    ++total_pushed_;

    cv_not_empty_.notify_one();
    return true;
}

// -- Consumer API --------------------------------------------------------------
std::optional<VideoFrame> FrameQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    cv_not_empty_.wait(lock, [this] {
        return closed_ || !queue_.empty();
    });

    if (queue_.empty()) {
        return std::nullopt;
    }

    VideoFrame frame = std::move(queue_.front());
    queue_.pop();
    ++total_popped_;

    cv_not_full_.notify_one();
    return frame;
}

std::optional<VideoFrame> FrameQueue::try_pop() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (queue_.empty()) {
        return std::nullopt;
    }

    VideoFrame frame = std::move(queue_.front());
    queue_.pop();
    ++total_popped_;

    cv_not_full_.notify_one();
    return frame;
}

// -- Control -------------------------------------------------------------------
void FrameQueue::close() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) return;
        closed_ = true;
    }
    cv_not_full_.notify_all();
    cv_not_empty_.notify_all();

    std::ostringstream oss;
    oss << "FrameQueue: closed (pushed=" << total_pushed_
        << " popped=" << total_popped_
        << " bp_events=" << bp_events_ << ")";
    LIZ_INFO(oss.str());
}

bool FrameQueue::is_closed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return closed_;
}

// -- Query ---------------------------------------------------------------------
std::size_t FrameQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

std::size_t FrameQueue::capacity() const {
    return capacity_;
}

bool FrameQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

bool FrameQueue::full() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size() >= capacity_;
}

std::size_t FrameQueue::total_pushed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return total_pushed_;
}

std::size_t FrameQueue::total_popped() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return total_popped_;
}

std::size_t FrameQueue::backpressure_events() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return bp_events_;
}

} // namespace liz