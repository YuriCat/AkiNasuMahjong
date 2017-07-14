/*
 world.hpp
 Katsuki Ohto
 */

#ifndef MAHJONG_STRUCTURE_WORLD_HPP_
#define MAHJONG_STRUCTURE_WORLD_HPP_

#include "../mahjong.hpp"

namespace Mahjong{
    
    /**************************仮想世界**************************/
    
    struct World{
        std::array<ExtPieceSet, N_PLAYERS> piece; // 持ち牌
        Wall wall; // 山
    };

}

#endif // MAHJONG_STRUCTURE_WORLD_HPP_