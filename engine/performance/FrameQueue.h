#pragma once

#include "engine/video/VideoFrame.h"

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>

namespace liz {

/// A bounded producer-consumer queue for VideoFrame objects.
///
/// Implements backpressure: producers block when the queue is full,
/// consumers block when the queue is empty.
///
/// Usage:
///   FrameQueue fq(8);   // capacity 8
///   fq.push(frame);     // blocks if full
///   auto f = fq.pop();  // blocks if empty, returns nullopt on close
///   fq.close();         // unblocks all waiters
class FrameQueue {
public:
    /// Construct a queue with the given maximum capacity.
    explicit FrameQueue(std::size_t capacity);

    /// Destructor closes the queue and wakes all waiters.
    ~FrameQueue();

    // Non-copyable.
    FrameQueue(const FrameQueue&) = delete;
    FrameQueue& operator=(const FrameQueue&) = delete;

    // -- Producer API -----------------------------------------------------------
    /// Push a frame into the queue.
    /// Blocks if the queue is full (backpressure).
    /// Returns false if the queue has been closed.
    bool push(VideoFrame frame);

    /// Try to push without blocking.  Returns false if full or closed.
    bool try_push(VideoFrame frame);

    // -- Consumer API -----------------------------------------------------------
    /// Pop a frame from the queue.
    /// Blocks if the queue is empty.
    /// Returns std::nullopt if the queue has been closed and drained.
    std::optional<VideoFrame> pop();

    /// Try to pop without blocking.  Returns nullopt if empty.
    std::optional<VideoFrame> try_pop();

    // -- Control ----------------------------------------------------------------
    /// Close the queue.  All current and future pushes return false.
    /// All waiting consumers are woken and will receive nullopt.
    void close();

    /// True after close() has been called.
    bool is_closed() const;

    // -- Query ------------------------------------------------------------------
    /// Current number of frames in the queue.
    std::size_t size() const;

    /// Maximum queue capacity.
    std::size_t capacity() const;

    /// True when no frames are buffered.
    bool empty() const;

    /// True when the queue is at capacity.
    bool full() const;

    /// Total frames that have been pushed (lifetime counter).
    std::size_t total_pushed() const;

    /// Total frames that have been popped (lifetime counter).
    std::size_t total_popped() const;

    /// Number of times a producer was blocked (backpressure events).
    std::size_t backpressure_events() const;

private:
    std::size_t                     capacity_;
    std::queue<VideoFrame>          queue_;

    mutable std::mutex              mutex_;
    std::condition_variable         cv_not_full_;
    std::condition_variable         cv_not_empty_;

    bool                            closed_      = false;
    std::size_t                     total_pushed_ = 0;
    std::size_t                     total_popped_ = 0;
    std::size_t                     bp_events_   = 0;
};

} // namespace liz