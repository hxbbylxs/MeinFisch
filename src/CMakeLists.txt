
add_subdirectory(constants)
add_subdirectory(game_logic)
add_subdirectory(game_loop)
add_subdirectory(engine)
add_subdirectory(io)
add_subdirectory(engineUCI)


add_executable(MeinFischV4 main.cpp
)

target_link_libraries(MeinFischV4
        constants
        game_logic
        game_loop
        io
        engine
        engineUCI
        test
)