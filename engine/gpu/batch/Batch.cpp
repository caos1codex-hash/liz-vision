#include "engine/gpu/batch/Batch.h"

#include <sstream>

namespace liz {

void Batch::add_tensor(Tensor t) {
    tensors.push_back(std::move(t));
}

const std::vector<Tensor>& Batch::get_tensors() const { return tensors; }
std::size_t Batch::size() const { return tensors.size(); }

std::size_t Batch::total_bytes() const {
    std::size_t total = 0;
    for (const auto& t : tensors) {
        total += t.byte_size();
    }
    return total;
}

std::string Batch::info() const {
    std::ostringstream oss;
    oss << "Batch{id=" << batch_id
        << " tensors=" << tensors.size()
        << " bytes=" << total_bytes() << "}";
    return oss.str();
}

} // namespace liz