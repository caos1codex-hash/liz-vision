#pragma once

#include "engine/ai/tensor/Tensor.h"

#include <cstdint>
#include <string>
#include <vector>

namespace liz {

/// A named group of tensors processed together on the GPU.
///
/// Represents one batch unit in the tensor-based execution model.
/// No computation — pure data container.
struct Batch {
    std::uint64_t          batch_id   = 0;
    double                 timestamp  = 0.0;
    std::vector<Tensor>    tensors;

    /// Add a tensor to the batch.
    void add_tensor(Tensor t);

    /// Read-only access to the tensor vector.
    const std::vector<Tensor>& get_tensors() const;

    /// Number of tensors in this batch.
    std::size_t size() const;

    /// Total byte size across all tensors.
    std::size_t total_bytes() const;

    /// One-line summary for logging.
    std::string info() const;
};

} // namespace liz