add_library (
        engineUCI
            engineUCI.cpp
)

target_include_directories(engineUCI PUBLIC .)
target_link_libraries(engineUCI engine io)