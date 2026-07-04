# engine/CMakeLists.txt
# Core engine library — all processing modules live here.
# This module must NEVER depend on apps/ (architecture rule).

add_library(liz-engine INTERFACE)
target_include_directories(liz-engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Future sprints will add:
#   - add_subdirectory(core)
#   - add_subdirectory(video)
#   - add_subdirectory(ai)
#   - add_subdirectory(gpu)
#   - add_subdirectory(plugins)
#   - add_subdirectory(memory)
#   - add_subdirectory(scheduler)