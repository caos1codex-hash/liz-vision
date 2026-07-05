# apps/liz-vision-desktop/CMakeLists.txt

set(DEMO_SOURCES
    main.cpp
    demo/DemoRunner.cpp
    demo/LegacyDemo.cpp
    demo/Sprint14Demo.cpp
    demo/Sprint15Demo.cpp
    demo/Sprint16Demo.cpp
    demo/Sprint17Demo.cpp
    demo/Sprint18Demo.cpp
    demo/Sprint19Demo.cpp
    demo/Sprint20Demo.cpp
    demo/Sprint21Demo.cpp
)

add_executable(liz-vision-desktop ${DEMO_SOURCES})
target_include_directories(liz-vision-desktop PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(liz-vision-desktop PRIVATE liz-engine)
