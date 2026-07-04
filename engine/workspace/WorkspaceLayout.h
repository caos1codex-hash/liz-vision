#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace liz {

/// Logical layout descriptor for a Workspace.
///
/// Represents the visual arrangement of panels — purely in-memory.
/// No GUI, docking, or window system implementation.
class WorkspaceLayout {
public:
    WorkspaceLayout() = default;
    ~WorkspaceLayout() = default;

    // ── Panel management ────────────────────────────────────────────

    void add_panel(const std::string& name) { panels_.push_back(name); }
    void set_active_panel(const std::string& name) { active_panel_ = name; }
    void set_position(std::size_t x, std::size_t y) { position_x_ = x; position_y_ = y; }
    void set_size(std::size_t w, std::size_t h) { width_ = w; height_ = h; }
    void set_visible(bool visible) { visible_ = visible; }

    // ── Accessors ────────────────────────────────────────────────────

    const std::vector<std::string>& panels() const { return panels_; }
    const std::string& active_panel() const { return active_panel_; }
    std::size_t position_x() const { return position_x_; }
    std::size_t position_y() const { return position_y_; }
    std::size_t width() const { return width_; }
    std::size_t height() const { return height_; }
    bool is_visible() const { return visible_; }
    std::size_t visible_panel_count() const { return panels_.size(); }

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    std::vector<std::string> panels_;
    std::string              active_panel_;
    std::size_t              position_x_ = 0;
    std::size_t              position_y_ = 0;
    std::size_t              width_      = 1920;
    std::size_t              height_     = 1080;
    bool                     visible_    = true;
};

} // namespace liz
