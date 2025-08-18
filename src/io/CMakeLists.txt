add_library(io
        Output.cpp
)

target_include_directories(io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(io constants game_logic)