#pragma once

#include <string>

namespace liz {

/// Types of jobs that can be submitted to the Job System.
///
/// Each job type represents a category of long-running operations
/// that the engine can execute through the Scheduler.
enum class JobType {
    Render,
    Inference,
    Pipeline,
    Import,
    Export,
    Plugin,
    Cloud,
    Asset,
    Project,
    Workspace,
    Custom
};

/// Convert a JobType to a human-readable string.
inline const char* job_type_to_string(JobType type) {
    switch (type) {
        case JobType::Render:     return "Render";
        case JobType::Inference: return "Inference";
        case JobType::Pipeline:   return "Pipeline";
        case JobType::Import:    return "Import";
        case JobType::Export:    return "Export";
        case JobType::Plugin:    return "Plugin";
        case JobType::Cloud:      return "Cloud";
        case JobType::Asset:      return "Asset";
        case JobType::Project:   return "Project";
        case JobType::Workspace: return "Workspace";
        case JobType::Custom:    return "Custom";
    }
    return "Unknown";
}

/// Priority levels for jobs.
///
/// Currently stored as metadata only — the JobQueue processes
/// jobs in FIFO order regardless of priority.
/// Future sprints: priority-based scheduling.
enum class JobPriority {
    Low,
    Normal,
    High,
    Critical
};

/// Convert a JobPriority to a human-readable string.
inline const char* job_priority_to_string(JobPriority priority) {
    switch (priority) {
        case JobPriority::Low:      return "Low";
        case JobPriority::Normal:   return "Normal";
        case JobPriority::High:     return "High";
        case JobPriority::Critical: return "Critical";
    }
    return "Unknown";
}

/// States a job transitions through during its lifecycle.
enum class JobState {
    Created,
    Queued,
    Running,
    Completed,
    Failed,
    Cancelled
};

/// Convert a JobState to a human-readable string.
inline const char* job_state_to_string(JobState state) {
    switch (state) {
        case JobState::Created:   return "Created";
        case JobState::Queued:    return "Queued";
        case JobState::Running:   return "Running";
        case JobState::Completed: return "Completed";
        case JobState::Failed:    return "Failed";
        case JobState::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

} // namespace liz
