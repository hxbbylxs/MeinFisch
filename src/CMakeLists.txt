
add_subdirectory(constants)
add_subdirectory(game_logic)
add_subdirectory(engine)
add_subdirectory(io)
add_subdirectory(engineUCI)


add_executable(MeinFischCandidate main.cpp
)

target_link_libraries(MeinFischCandidate
        constants
        game_logic
        io
        engine
        engineUCI
        test
)