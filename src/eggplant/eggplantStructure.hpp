/*
 eggplantStructure.hpp
 Katsuki Ohto
 */

#ifndef MAHJONG_EGGPLANT_EGGPLANTSTRUCTURE_HPP_
#define MAHJONG_EGGPLANT_EGGPLANTSTRUCTURE_HPP_

// 思考用の構造体
//#include "montecarlo/playout.h"

//#include "../structure/field/clientField.hpp"
//#include "estimation/galaxy.hpp"
//#include "model/playerModel.hpp"

#include "../mahjong.hpp"

#include "value.hpp"
#include "turnActionPolicy.hpp"

namespace Mahjong{
    namespace Eggplant{

        // Field以外のデータ構造
        // Fieldは基本盤面情報+盤面を進めたり戻したりするときに値が変化するもの
        // それ以外の重目のデータ構造は SharedData
        // スレッドごとのデータは ThreadTools
        
        struct ThreadTools{
            // 各スレッドの持ち物
            using dice64_t = XorShift64;
            //using move_t = MoveInfo;
            
#ifndef POLICY_ONLY
            // MCしないなら世界生成なし
            //using galaxy_t = Galaxy<ImaginaryWorld>;
            
            // 世界生成プール
            //galaxy_t gal;
#endif
            // サイコロ
            dice64_t dice;
            
            // 着手生成バッファ
            //static constexpr int BUFFER_LENGTH = 8192;
            
            // スレッド番号
            int threadIndex;
            
            //move_t buf[BUFFER_LENGTH];
            
            void init(int index){
                //memset(buf, 0, sizeof(buf));
                threadIndex = index;
#ifndef POLICY_ONLY
                //gal.clear();
#endif
            }
            void close(){}
        };
        
        struct SharedData{
            // 全体で共通のデータ
            
            // 計算量解析
            int modeling_time;
            int estimating_by_time;
            
            volatile double exp_wr; // 今ターンの期待報酬(0~1)
            
            //uint32_t game_reward[N_PLAYERS];
            
            // 基本方策
            TurnActionPolicy baseTurnActionPolicy;
            //PlayPolicy basePlayPolicy;
            
            //MinMatchLog<MinClientGameLog<MinClientPlayLog<N_PLAYERS>>> matchLog;
            //MinClientGameLog<MinClientPlayLog<N_PLAYERS>> gameLog;
            
            MatchRecord matchRecord;
            GameRecord gameRecord;
            
#ifndef POLICY_ONLY
            //using galaxy_t = ThreadTools::galaxy_t;
            //GalaxyAnalyzer<galaxy_t, N_THREADS> ga;
            //MyTimeAnalyzer timeAnalyzer;
            
            // 推定用方策
            //ChangePolicy estimationChangePolicy;
            //PlayPolicy estimationPlayPolicy;
            
            // 相手方策モデリング
            //PlayerModelSpace playerModelSpace;
#endif
            void initMatch(){
                
                // 計算量解析初期化
                //modeling_time = 0;
                //estimating_by_time = 0;
                
#if defined(RL_POLICY)
               // changeLearner.setPolicy(&baseChangePolicy);
               // playLearner.setPolicy(&basePlayPolicy);
                
               // baseChangePolicy.setLearner(&changeLearner);
               // basePlayPolicy.setLearner(&playLearner);
#endif
            }
            void setMyPlayerNum(int myPlayerNum){
#ifndef POLICY_ONLY
                //playerModelSpace.init(myPlayerNum);
#endif
            }
            void initGame(){
                
            }
            template<class field_t>
            void closeGame(const field_t& field){}
            void closeMatch(){
#ifndef POLICY_ONLY
                //playerModelSpace.closeMatch();
#endif
            }
        };
        
        /**************************手番での行動+差分情報**************************/
        
        struct TurnActionInfo : public TurnAction{
            NextHandInfo nextHandInfo;
            DiffHandInfo diffhandInfo;
        };
        
        struct GameStatistics{
            // 多数試合のスタッツ
            // モンテカルロの結果記録にもそのまま使える
            uint32_t games;
            uint32_t drawWins, responseWins; // 上がり回数
            uint32_t presents; // 振り込み回数
            uint32_t fishingDraws, farDraws; // 流局の際の聴牌回数
            std::array<uint32_t, 64> bonus; // ボーナスの付いた回数
        };
        
        template<class action_t>
        struct RootActionInfo{
            // モンテカルロの結果保存等
            action_t action;
            
            // モンテカルロ統計
            uint32_t simulations;
            
            BetaDistribution utility;
            uint64_t scoreSum;
            DirichletDistribution<N_PLAYERS> distribution; // 最終順位の予測分布
            uint32_t drawWins, responseWins; // 上がり回数
            uint32_t presents; // 振り込み回数
            
            // 即時評価
            double policyScore; // 方策の評価点
            int minimumSteps; // シャンテン数
            BitSet64 acceptable;
            int acceptableNum;
            
            double mean()const{ return utility.mean(); }
            double mean_var()const{ return utility.var(); }
            
            void initSatistics()noexcept{
                // モンテカルロでの統計初期化
                simulations = 0;
                utility.set(0.5, 0.5); // 報酬事前分布
                distribution.fill(0.5); // 最終順位事前分布
                drawWins = responseWins = presents = 0;
                scoreSum = 0;
            }
            
            template<class field_t>
            void init(const action_t&, field_t&, Player);
            
            template<class result_t>
            void feed(const result_t& result)noexcept{
                const double reward = distributionToReward(result.distribution);
                utility += BetaDistribution(reward, 1 - reward);
                simulations += 1;
                scoreSum += result.nextScore;
                for(int i = 0; i < N_PLAYERS; ++i){
                    distribution.add(i, result.distribution[i]);
                }
                if(result.drawWin){
                    drawWins += 1;
                }else if(result.responseWin){
                    responseWins += 1;
                }else if(result.present){
                    presents += 1;
                }
            }
            
            uint32_t wins()const noexcept{ return drawWins + responseWins; }
            double drawWinRate()const{ return drawWins / (double)simulations; }
            double responseWinRate()const{ return responseWins / (double)simulations; }
            double winRate()const{ return wins() / (double)simulations; }
            double presentRate()const{ return presents / (double)simulations; }
            double meanScore()const{ return (double)scoreSum / simulations; }
        };
        
        template<>template<class field_t>
        void RootActionInfo<TurnAction>::init(const TurnAction& aa, field_t& field, Player pn){
            action = aa;
            
            initSatistics();
            policyScore = 0;
            
            auto& hand = field.hand[pn];
            
            // 基本的な性質をセット
            if(!field.isInReach(field.turnPlayer)){
                if(!aa.finish()){
                    DiffHandInfo diff;
                    NextHandInfo next;
                    doTurnAction(&hand, aa, &diff, &next);
                    ASSERT(hand.exam(), cerr << hand.toDebugString() << endl;);
                    minimumSteps = calcMinimumSteps(hand, &acceptable);
                    undoTurnAction(&hand, aa, diff);
                    ASSERT(hand.exam(), cerr << hand.toDebugString() << endl;);
                    acceptableNum = acceptable.count();
                }
                policyScore = -minimumSteps + acceptableNum / double(N_PIECES);
            }else{
                minimumSteps = 0;
            }
            
            double ps = sigmoid(policyScore, 1);
            
            utility += BetaDistribution(ps * 4, (1 - ps) * 4);
        }

        template<>template<class field_t>
        void RootActionInfo<ResponseAction>::init(const ResponseAction& aa, field_t& field, Player pn){
            action = aa;
            initSatistics();
            policyScore = 0;
            
            auto& hand = field.hand[pn];
            
            // 基本的な性質をセット
            
            double ps = sigmoid(policyScore, 1);
            
            utility += BetaDistribution(ps * 4, (1 - ps) * 4);
        }
        
        template<class action_t>
        struct RootInfo{
            
            static constexpr int N_MAX_ACTIONS = cmax(N_MAX_TURN_ACTIONS, N_MAX_RESPONSE_ACTIONS);
            std::array<RootActionInfo<action_t>, N_MAX_ACTIONS> action;
            int actions;
            Score originalScore;
            
            uint32_t simulations;
            BetaDistribution allUtility;
            
#ifdef MULTI_THREADING
            SpinLock<> lock_;
#else
            NullLock<> lock_;
#endif
            
            template<class field_t>
            void setActions(action_t *const pact, const int aactions, field_t& field, Player pn){
                originalScore = field.score[pn];
                actions = aactions;
                for(int i = 0; i < actions; ++i){
                    action[i].init(pact[i], field, pn);
                    allUtility += action[i].utility;
                }
            }
            
            int searchBestIndex()const{
                int bestIndex = 0;
                double bestScore = -DBL_MAX;
                for(int i = 0; i < actions; ++i){
                    double sc = action[i].mean();
                    if(sc > bestScore){
                        bestIndex = i;
                        bestScore = sc;
                    }
                }
                return bestIndex;
            }
            
            template<class result_t>
            void feed(int index, const result_t& result){
                lock_.lock();
                action[index].feed(result);
                lock_.unlock();
            }
            
            void init()noexcept{
                lock_.unlock();
                actions = 0;
                simulations = 0;
            }
            
            std::string toString()const{
                std::ostringstream oss;
                std::array<RootActionInfo<action_t>, N_MAX_ACTIONS> sortedAction = action;
                std::stable_sort(sortedAction.begin(), sortedAction.begin() + actions, [](const auto& a, const auto& b)->bool{
                    return a.mean() > b.mean();
                });
                for(int i = 0; i < actions; ++i){
                    const double mean = sortedAction[i].mean();
                    const double sem = sqrt(sortedAction[i].mean_var());
                    const double ci[2] = {mean - sem, mean + sem};
                    
                    if(i == 0){
                        oss << "\033[1m";
                    }
                    oss << i << " " << sortedAction[i].action << " : " << mean << " ( " << ci[0] << " ~ " << ci[1] << " ) ";
                    oss << "ms = " << sortedAction[i].minimumSteps;
                    oss << " pol = " << (int(sortedAction[i].policyScore * 10000) / 10000.0) << " ";
                    if(sortedAction[i].simulations > 0){
                        oss << "w:" << int(sortedAction[i].winRate() * 100);
                        oss << "(" << int(sortedAction[i].drawWinRate() * 100) << "," << int(sortedAction[i].responseWinRate() * 100) << ") ";
                        oss << "sc " << int(sortedAction[i].meanScore() - (double)originalScore) << " ";
                        oss << sortedAction[i].simulations << " simulations.";
                    }
                    oss << endl;
                    if(i == 0){
                        oss << "\033[0m";
                    }
                }
                return oss.str();
            }
        };
        
        template<class action_t>
        std::ostream& operator <<(std::ostream& ost, const RootInfo<action_t>& ri){
            ost << ri.toString();
            return ost;
        }
    }
}

#endif // MAHJONG_EGGPLANT_EGGPLANTSTRUCTURE_HPP_
