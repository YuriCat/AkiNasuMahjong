/*
 mahjong.hpp
 Katsuki Ohto
 */

#ifndef MAHJONG_MAHJONG_HPP_
#define MAHJONG_MAHJONG_HPP_

#include "structure/base.hpp"
#include "structure/piece.hpp"
#include "structure/meld.hpp"
#include "structure/hand.hpp"
#include "structure/score.hpp"
#include "structure/record.hpp"
#include "structure/world.hpp"
#include "structure/field.hpp"
#include "structure/score.hpp"
#include "structure/ai.hpp"

namespace Mahjong{

    /**************************初期化**************************/
    
    struct MahjongInitializer{
        MahjongInitializer(){
            // ハッシュ値計算用のテーブル初期化
            initHash();
            
            // シャンテン数、受け入れ数計算用テーブル読み込み
            tick();
            initShantenTable();
            initAcceptableTable();
            tock();
            tick();
            initMinimumStepsInfoTable();
            initAcceptableInfoTable();
            tock();
        }
    };
    
    MahjongInitializer mahjongInitializer_;
}

#endif // MAHJONG_MAHJONG_HPP_