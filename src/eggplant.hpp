/*
 eggplant.hpp
 Katsuki Ohto
 */

// 秋茄子思考部メイン

#ifndef MAHJONG_EGGPLANT_HPP_
#define MAHJONG_EGGPLANT_HPP_

#include "mahjong.hpp"

#include "eggplant/eggplant.h"
#include "eggplant/eggplantStructure.hpp"

#include "eggplant/monteCarlo.hpp"

namespace Mahjong{
    namespace Eggplant{
        
        class EggplantAI : public AI{
        public:
            
            TurnAction turn(ExtPiece drawn){
                // ツモ手番での行動決定
                cerr << field().toString();
                cerr << "drawn " << drawn << endl;
                
                std::array<TurnAction, N_MAX_TURN_ACTIONS> action;
                const int actions = genTurnActions(action.data(), playerNum(), field(), drawn);
                cerr << actions << " turn actions were generated." << endl;
                
                std::shuffle(action.begin(), action.begin() + actions, dice_); // 偏りをなくす
                
                RootInfo<TurnAction> info;
                info.init();
                info.setActions(action.data(), actions, field(), playerNum());
                
                if(actions > 1){
                    //doMonteCarloSearch(&info, field(), &shared_, tools_);
                }
                
                TurnAction bestAction = action[info.searchBestIndex()];
                
                cerr << info;
                
                cerr << "\033[1m" << "\033[" << 34 << "m";
                cerr << "Best Turn Action : " << bestAction << endl;
                cerr << "\033[" << 39 << "m" << "\033[0m";
                
                if(bestAction.discarded() == drawn){
                    // ツモ切りかどうかは現時点で検討していない
                    bestAction.setParrot();
                }
                lastTurnAction_ = bestAction; // 一時保存
                return bestAction;
            }
            
            ResponseAction response(ExtPiece discarded){
                // 相手の打牌の後の行動決定
                cerr << field().toString();
                cerr << "discarded " << discarded << endl;
                
                std::array<ResponseAction, N_MAX_RESPONSE_ACTIONS> action;
                const int actions = genResponseActions(action.data(), field(), discarded, playerNum());
                cerr << actions << " response actions were generated." << endl;
                
                std::shuffle(action.begin(), action.begin() + actions, dice_); // 偏りをなくす
                
                RootInfo<ResponseAction> info;
                info.init();
                info.setActions(action.data(), actions, field(), playerNum());
                
                //doMonteCarloSearch(&info, field(), &shared_, tools_);
                ResponseAction bestAction = action[info.searchBestIndex()];
                
                cerr << info;
                
                cerr << "\033[1m" << "\033[" << 31 << "m";
                cerr << "Best Response Action : " << bestAction << endl;
                cerr << "\033[" << 39 << "m" << "\033[0m";
                
                lastResponseAction_ = bestAction; // 一時保存
                return bestAction;
            }
            
            int initMatch(){
                for(int i = 0; i < N_MAX_THREADS; ++i){
                    tools_[i].init(i);
                }
                shared_.initMatch();
                return 0;
            }
            int initGame(){
                shared_.initGame();
                return 0;
            }
            int closeGame(){
                shared_.closeGame(field());
                cerr << field().toString();
                return 0;
            }
            int closeMatch(){
                shared_.closeMatch();
                for(int i = 0; i < N_MAX_THREADS; ++i){
                    tools_[i].close();
                }
                return 0;
            }
            
            int setRandomSeed(uint64_t seed){
                AI::setRandomSeed(seed);
                for(int i = 0; i < N_MAX_THREADS; ++i){
                    tools_[i].dice.srand(dice_());
                }
                return 0;
            }
            
            EggplantAI(): AI(){
                // 乱数シードは時刻で初期化
                // (あとで変更できる)
                setRandomSeed((unsigned int)time(NULL));
            }
            
        private:
            SharedData shared_;
            ThreadTools tools_[N_MAX_THREADS];
        };
        
    }
}

#endif // MAHJONG_EGGPLANT_HPP_