#pragma once

#include <string>
#include <vector>

namespace liz {

/// Metadata associated with a Project.
///
/// Contains organizational and descriptive information.
/// All fields are in-memory only — no persistence.
class ProjectMetadata {
public:
    ProjectMetadata() = default;
    ~ProjectMetadata() = default;

    // ── Accessors ────────────────────────────────────────────────────

    const std::string& company() const { return company_; }
    const std::string& website() const { return website_; }
    const std::string& description() const { return description_; }
    const std::vector<std::string>& tags() const { return tags_; }
    const std::string& author() const { return author_; }
    const std::string& version() const { return version_; }

    // ── Mutators ────────────────────────────────────────────────────

    void set_company(const std::string& value) { company_ = value; }
    void set_website(const std::string& value) { website_ = value; }
    void set_description(const std::string& value) { description_ = value; }
    void set_author(const std::string& value) { author_ = value; }
    void set_version(const std::string& value) { version_ = value; }

    void add_tag(const std::string& tag) { tags_.push_back(tag); }
    void clear_tags() { tags_.clear(); }

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    std::string              company_;
    std::string              website_;
    std::string              description_;
    std::vector<std::string> tags_;
    std::string              author_;
    std::string              version_;
};

} // namespace liz
