//
// Created by salom on 16.07.2025.
//

#ifndef EVALUATIONCONSTANTS_H
#define EVALUATIONCONSTANTS_H
#include <array>
#include <cstdint>
#include <functional>
#include "Constants.h"
#include "MoveGenerationConstants.h"


inline constexpr int CHECKMATE_VALUE = 100000;
inline constexpr int LAZY_EVAL_SAFETY_MARGIN = 200;
inline constexpr int FUTILITY_BASE_SAFETY_MARGIN = 280;
inline constexpr int FUTILITY_DEPTH_SAFETY_MARGIN = 100;

inline constexpr std::array<int,13> STATIC_MG_PIECE_VALUES = {0,100,-100,320,-320,330,-330,500,-500,900,-900,0,0};
inline constexpr std::array<int,13> STATIC_EG_PIECE_VALUES = {0,120,-120,330,-330,340,-340,520,-520,940,-940,0,0};



#endif //EVALUATIONCONSTANTS_H
