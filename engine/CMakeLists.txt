# engine/CMakeLists.txt
# Core engine library — all processing modules live here.
# This module must NEVER depend on apps/ (architecture rule).

add_library(liz-engine
    core/Logger.cpp
    core/Config.cpp
    core/EngineContext.cpp
    core/Engine.cpp
    scheduler/Task.cpp
    scheduler/TaskQueue.cpp
    scheduler/Scheduler.cpp
    plugins/PluginManager.cpp
    video/VideoFrame.cpp
    video/VideoDecoder.cpp
    video/VideoPipeline.cpp
)
target_include_directories(liz-engine PUBLIC ${CMAKE_SOURCE_DIR})
target_compile_features(liz-engine PUBLIC cxx_std_20)

# Future sprints will add:
#   - ai/, gpu/, memory/ subdirectories