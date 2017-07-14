/*
 ai.hpp
 Katsuki Ohto
 */

// 思考クラスのベース

#ifndef MAHJONG_AI_HPP_
#define MAHJONG_AI_HPP_

#include <random>

#include "../settings.h"
#include "meld.hpp"
#include "action.hpp"
#include "score.hpp"
#include "field.hpp"

namespace Mahjong{
    class AI{
    public:
        std::string name()const{ return name_; }
        Field& field()noexcept{ return field_; }
        const Field& field()const noexcept{ return field_; }
        Player playerNum()const noexcept{ return field().myPlayerNum; }
        TurnAction lastTurnAction()const noexcept{ return lastTurnAction_; }
        ResponseAction lastResponseAction()const noexcept{ return lastResponseAction_; }
        void setLastMeld(Meld m)noexcept{ lastMeld_ = m; }
        void clearLastMeld()noexcept{ lastMeld_ = 0; }
        Meld lastMeld()const noexcept{ return lastMeld_; }
        
        TurnAction turn(ExtPiece drawn){
            TurnAction action;
            action.clear().setDiscarded(drawn).setParrot();
            lastTurnAction_ = action;
            return action;
        }
        
        ResponseAction response(ExtPiece discarded){
            ResponseAction action;
            action.clear();
            lastResponseAction_ = action;
            return action;
        }
        
        int initMatch(){ return 0; }
        int initGame(){ return 0; }
        int closeGame(){ return 0; }
        int closeMatch(){ return 0; }
        int setName(const std::string& name){
            name_ = name;
            return 0;
        }
        int setRandomSeed(uint64_t seed){
            dice_.seed(seed);
            return 0;
        }
        
        AI():name_(MY_NAME), field_(), dice_(){
            // 乱数シードは時刻で初期化
            // (あとで変更できる)
            setRandomSeed((unsigned int)time(NULL));
        }
        
    protected:
        std::string name_;
        Field field_;
        std::mt19937 dice_;
        // 自分の最後の行動
        TurnAction lastTurnAction_;
        ResponseAction lastResponseAction_;
        // 過去の行動の記録(自他問わず)
        Meld lastMeld_;
    };
}

#endif // MAHJONG_AI_HPP_