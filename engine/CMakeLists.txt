# engine/CMakeLists.txt
# Core engine library — all processing modules live here.
# This module must NEVER depend on apps/ (architecture rule).

add_library(liz-engine
    core/Logger.cpp
    core/Config.cpp
)
target_include_directories(liz-engine PUBLIC ${CMAKE_SOURCE_DIR})
target_compile_features(liz-engine PUBLIC cxx_std_20)

# Future sprints will add:
#   - core/Engine.cpp
#   - core/EngineContext.cpp
#   - scheduler/TaskQueue.cpp
#   - scheduler/Scheduler.cpp
#   - plugins/PluginManager.cpp
#   - video/, ai/, gpu/, memory/ subdirectories