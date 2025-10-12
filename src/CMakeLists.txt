
add_subdirectory(constants)
add_subdirectory(game_logic)
add_subdirectory(engine)
add_subdirectory(io)
add_subdirectory(engineUCI)
add_subdirectory(utils)


add_executable(MeinFischCandidate main.cpp
        utils/Utils.cpp
        utils/Utils.h
)

target_link_libraries(MeinFischCandidate
        constants
        game_logic
        io
        engine
        engineUCI
        test
        utils
)