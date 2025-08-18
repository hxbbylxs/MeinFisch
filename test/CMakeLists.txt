add_library(test
        Test.cpp
        PerformanceTest.cpp
)

target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test constants game_logic engine)