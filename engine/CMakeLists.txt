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
    video/ffmpeg/FFmpegDecoder.cpp
    ai/AIProcessor.cpp
    ai/AIContext.cpp
    ai/AIPipeline.cpp
    ai/tensor/TensorShape.cpp
    ai/tensor/Tensor.cpp
    gpu/GPUDevice.cpp
    gpu/GPUContext.cpp
    gpu/memory/GPUBuffer.cpp
    gpu/memory/GPUMemoryPool.cpp
    gpu/compute/GPUCommand.cpp
    gpu/compute/GPUQueue.cpp
    gpu/compute/GPUComputeEngine.cpp
    gpu/batch/Batch.cpp
    gpu/batch/BatchProcessorGPU.cpp
    ai/inference/Model.cpp
    ai/inference/ModelLoader.cpp
    ai/inference/InferenceEngine.cpp
    ai/models/ModelRegistry.cpp
    performance/ThreadPool.cpp
    performance/FrameQueue.cpp
    performance/TaskExecutor.cpp
    performance/BatchProcessor.cpp
    performance/PerformanceManager.cpp
    resources/Resource.cpp
    resources/ResourceHandle.cpp
    resources/ResourceCache.cpp
    resources/ResourceManager.cpp
    runtime/LifecycleManager.cpp
    runtime/Runtime.cpp
    events/Event.cpp
    events/EventDispatcher.cpp
    events/EventBus.cpp
)
target_include_directories(liz-engine PUBLIC ${CMAKE_SOURCE_DIR})
target_compile_features(liz-engine PUBLIC cxx_std_20)
target_link_libraries(liz-engine PUBLIC Threads::Threads)

# Future sprints:
#   - memory/ subdirectory
#   - #ifdef WITH_FFMPEG: link ffmpeg libraries