add_library(
        constants
            Constants.h
            EvaluationConstants.h
            MoveGenerationConstants.cpp
            MoveGenerationConstants.h
        PieceSquareTables.h
)

target_include_directories(constants PUBLIC .)