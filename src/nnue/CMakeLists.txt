add_library(nnue
        Network.cpp
)

target_include_directories(nnue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nnue constants game_logic)