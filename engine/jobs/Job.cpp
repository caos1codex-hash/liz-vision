#include "engine/jobs/Job.h"

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace liz {

// ── Constructor ──────────────────────────────────────────────────────────

Job::Job(std::string name, JobType type, JobPriority priority)
    : uuid_(generate_uuid())
    , name_(std::move(name))
    , type_(type)
    , priority_(priority)
{
    using namespace std::chrono;
    creation_time_ = static_cast<std::uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

// ── Accessors ────────────────────────────────────────────────────────────

const std::string& Job::uuid() const            { return uuid_; }
const std::string& Job::name() const           { return name_; }
JobType Job::type() const                       { return type_; }
JobPriority Job::priority() const              { return priority_; }
JobState Job::state() const                     { return state_; }
std::uint8_t Job::progress() const              { return progress_; }
std::uint64_t Job::creation_time() const        { return creation_time_; }
std::uint64_t Job::start_time() const           { return start_time_; }
std::uint64_t Job::end_time() const             { return end_time_; }
double Job::duration_ms() const                 { return duration_ms_; }
const std::string& Job::error_message() const   { return error_message_; }
bool Job::is_cancelable() const                 { return cancelable_; }
void* Job::user_data() const                    { return user_data_; }

// ── Mutators ────────────────────────────────────────────────────────────

void Job::set_state(JobState state)              { state_ = state; }
void Job::set_progress(std::uint8_t progress)    { progress_ = progress; }
void Job::set_start_time(std::uint64_t time)     { start_time_ = time; }
void Job::set_end_time(std::uint64_t time)        { end_time_ = time; }
void Job::set_duration_ms(double duration)        { duration_ms_ = duration; }
void Job::set_error_message(const std::string& msg) { error_message_ = msg; }
void Job::set_cancelable(bool cancelable)          { cancelable_ = cancelable; }
void Job::set_user_data(void* data)                { user_data_ = data; }

// ── Utility ──────────────────────────────────────────────────────────────

std::string Job::to_string() const {
    std::ostringstream oss;
    oss << "Job{"
        << "uuid=" << uuid_
        << ", name=\"" << name_ << "\""
        << ", type=" << job_type_to_string(type_)
        << ", priority=" << job_priority_to_string(priority_)
        << ", state=" << job_state_to_string(state_)
        << ", progress=" << static_cast<int>(progress_)
        << ", duration=" << std::fixed << std::setprecision(1) << duration_ms_ << "ms"
        << "}";
    return oss.str();
}

// ── UUID generation ─────────────────────────────────────────────────────

std::string Job::generate_uuid() {
    static std::mt19937_64 rng(std::random_device{}());
    std::uint64_t value = rng();
    std::ostringstream oss;
    oss << std::hex << std::setw(8) << std::setfill('0') << (value & 0xFFFFFFFFu);
    return oss.str();
}

} // namespace liz
