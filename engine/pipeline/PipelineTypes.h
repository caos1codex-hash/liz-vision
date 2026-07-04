#pragma once

#include <string>

namespace liz {

/// Types of nodes in a pipeline graph.
enum class PipelineNodeType {
    Unknown,
    Input,           ///< Data input (file, stream, buffer)
    Decoder,         ///< Video/audio decoding
    VideoFilter,     ///< Visual processing filter
    TensorConverter, ///< Convert frames to tensors
    Inference,       ///< AI model inference
    GPUUpload,       ///< Upload data to GPU memory
    GPUCompute,      ///< GPU compute operation
    GPUDownload,     ///< Download data from GPU memory
    Output,          ///< Final output (file, display, buffer)
    Custom           ///< User-defined node type
};

/// Convert a PipelineNodeType to a human-readable string.
inline const char* pipeline_node_type_to_string(PipelineNodeType type) {
    switch (type) {
        case PipelineNodeType::Unknown:        return "Unknown";
        case PipelineNodeType::Input:          return "Input";
        case PipelineNodeType::Decoder:        return "Decoder";
        case PipelineNodeType::VideoFilter:    return "VideoFilter";
        case PipelineNodeType::TensorConverter: return "TensorConverter";
        case PipelineNodeType::Inference:      return "Inference";
        case PipelineNodeType::GPUUpload:      return "GPUUpload";
        case PipelineNodeType::GPUCompute:     return "GPUCompute";
        case PipelineNodeType::GPUDownload:    return "GPUDownload";
        case PipelineNodeType::Output:         return "Output";
        case PipelineNodeType::Custom:         return "Custom";
    }
    return "Unknown";
}

/// Lifecycle state of a pipeline node.
enum class PipelineNodeState {
    Created,    ///< Node exists but not configured
    Ready,      ///< Node is configured and ready to execute
    Running,    ///< Node is currently executing
    Completed,  ///< Node has finished execution successfully
    Failed,     ///< Node execution failed
    Disabled    ///< Node is disabled and will be skipped
};

/// Convert a PipelineNodeState to a human-readable string.
inline const char* pipeline_node_state_to_string(PipelineNodeState state) {
    switch (state) {
        case PipelineNodeState::Created:   return "Created";
        case PipelineNodeState::Ready:     return "Ready";
        case PipelineNodeState::Running:   return "Running";
        case PipelineNodeState::Completed: return "Completed";
        case PipelineNodeState::Failed:    return "Failed";
        case PipelineNodeState::Disabled:  return "Disabled";
    }
    return "Unknown";
}

/// Types of connections between pipeline nodes.
enum class ConnectionType {
    Data,        ///< Data flows from source to destination
    Control,     ///< Source triggers or controls destination execution
    Dependency   ///< Destination cannot start until source completes
};

/// Convert a ConnectionType to a human-readable string.
inline const char* connection_type_to_string(ConnectionType type) {
    switch (type) {
        case ConnectionType::Data:        return "Data";
        case ConnectionType::Control:     return "Control";
        case ConnectionType::Dependency: return "Dependency";
    }
    return "Unknown";
}

} // namespace liz
