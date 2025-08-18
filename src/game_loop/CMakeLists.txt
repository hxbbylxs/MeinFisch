add_library (
        game_loop
        Settings.h
)

target_include_directories(game_loop PUBLIC .)
target_link_libraries(game_loop engine game_logic io)