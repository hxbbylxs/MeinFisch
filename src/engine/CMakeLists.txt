add_library(
        engine
            Engine.cpp
            EvaluationFunction.cpp
            Memory.cpp
        Movepicking.cpp
        Movepicking.h
)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine constants game_logic io nnue)