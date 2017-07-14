/*
 record.hpp
 Katsuki Ohto
 */

// 牌譜

#ifndef MAHJONG_STRUCTURE_RECORD_HPP_
#define MAHJONG_STRUCTURE_RECORD_HPP_

#include "base.hpp"
#include "piece.hpp"
#include "action.hpp"
#include "world.hpp"

#include "../protocol/tenho.hpp"

namespace Mahjong{
    
    /**************************牌譜**************************/
    
    struct TurnRecord{
        Player turnPlayer; // ツモプレーヤー
        Piece drawn; // ツモ牌
        TurnAction turnAction;
        uint32_t turnActionTime;
        std::array<ResponseAction, N_PLAYERS> resonseAction;
        std::array<uint32_t, N_PLAYERS> responseActionTime;
        
        //constexpr bool anyChow()const noexcept{ return chow != NONE_PLAYER; }
        //constexpr bool anyPung()const noexcept{ return pung != NONE_PLAYER; }
        
        bool anyResponse(){
            return false;
        }
        
        constexpr TurnRecord()
        :turnPlayer(NONE_PLAYER), drawn(PIECE_NONE),
        turnAction(), turnActionTime(), resonseAction(), responseActionTime(){}
    };
    
    struct GameRecord{
        World world_; // 初期牌譜と山(観測可能な範囲のみ)
        std::array<TurnRecord, N_TURNS> turn_; // 各ターンでの記録
        std::array<int, N_PLAYERS> beforeScore_; // 局開始時点での得点
        std::array<int, N_PLAYERS> afterScore_; // 局終了時点での得点
        Wind fieldWind_;
        Player owner_; // 親
        int turns_; // ターン数
        
        int owner()const noexcept{ return owner_; }
        int turns()const noexcept{ return turns_; }
        Wind fieldWind()const noexcept{ return fieldWind_; }
        
        Wind playerWind(Player pn)const noexcept{ // プレーヤーpの自風 親なら東(WIND_E)
            return static_cast<Wind>((unsigned int)(pn - owner()) % N_PLAYERS) + WIND_E;
        }
        
        TurnRecord& turn(int t){ return turn_[t]; }
        const TurnRecord& turn(int t)const{ return turn_[t]; }
        
        int finTenho(const std::string& fileName){
            std::ifstream ifs(fileName);
            if(!ifs){
                cerr << "GameRecord::finTenho : failed to open " << fileName << endl;
                return -1;
            }
            
            return 0;
        }
        int fout(const std::string& fileName){
            return 0;
        }
    };
    
    template<class field_t,
    class gameRecord_t,
    class firstCallback_t,
    class drawCallback_t,
    class lastCallback_t>
    int iterateGame
    (const gameRecord_t& gameRecord,
     const firstCallback_t& firstCallback,
     const drawCallback_t& drawCallback,
     const lastCallback_t& lastCallback){
        
        field_t field;
        
        firstCallback(field);
        
        for(int t = 0; t < gameRecord.turns(); ++t){
            const auto turnRecord = gameRecord.turn(t);
            
            turnCallback(field, gameRecord.drawn(t), turnRecord.turnAction, turnRecord.turnActionTime);
            
            field.discard(turnRecord.turnPlayer, turnRecord.turnAction.discarded());
            
            if(turnRecord.anyResponse()){
                for(Player p = 0; p < N_PLAYERS; ++p){
                    responseCallback(field, turnRecord.turnAction.discarded(),
                                     turnRecord.resonseAction[p], turnRecord.resonseActionTime[p]);
                }
            }
        }
        
        return 0;
    }
    
    struct MatchRecord : public std::vector<GameRecord>{
        GameRecord& game(int t){ return (*this)[t]; }
        const GameRecord& game(int t)const{ return (*this)[t]; }
        
        int finTenho(std::ifstream& ifs){
            //for()
            return 0;
        }
        
        int finTenho(const std::string& fileName){
            // 天鳳の棋譜ファイル読み込み
            std::ifstream ifs(fileName);
            if(!ifs){
                cerr << "GameRecord::finTenho : failed to open " << fileName << endl;
                return -1;
            }
            return finTenho(ifs);
            return 0;
        }
        int fout(const std::string& fileName){
            return 0;
        }
    };
}

#endif // MAHJONG_STRUCTURE_RECORD_HPP_