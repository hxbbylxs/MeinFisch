add_library (
        game_logic
            GameBoard.cpp
            MoveGeneration.cpp
            Move.cpp
            Conversions.cpp
)

target_include_directories(game_logic PUBLIC .)
target_link_libraries(game_logic constants)