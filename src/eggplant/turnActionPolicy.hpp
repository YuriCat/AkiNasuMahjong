/*
 turnActionPolicy.hpp
 Katsuki Ohto
 */

// ツモ手番行動用の方策

#ifndef MAHJONG_EGGPLANT_TURNACTIONPOLICY_HPP_
#define MAHJONG_EGGPLANT_TURNACTIONPOLICY_HPP_

#include "../mahjong.hpp"

#include "eggplant.h"

namespace Mahjong{
    namespace Eggplant{
#define Foo(f, i) {ASSERT(FEA_IDX(p) <= i && i < FEA_IDX((f) + 1),\
        cerr << "Foo() : illegal index " << (i) << " in " << FEA_IDX(f) << "~" << (FEA_IDX((f) + 1) - 1);\
        cerr << " (fi = " << (f) << ")" << endl; );\
        s += mdl.param(i); mdl.template feedFeatureScore<M>((i), 1.0);}
        
#define FooXC(f, i, x) {ASSERT(POL_IDX(p) <= i && i < POL_IDX((p) + 1),\
        cerr << "FooX() : illegal index " << (i) << " in " << FEA_IDX(f) << "~" << (FEA_IDX((f) + 1) - 1);\
        cerr << " (fi = " << (f) << ")" << endl; );\
        s += mdl.param(i) * (x); mdl.template feedFeatureScore<M>((i), (x));}
        
        namespace TurnActionPolicySpace{
            
            enum{
                FEA_MINIMUM_STEPS, // シャンテン数
                FEA_ACCEPTABLE_NUM, // 受け入れ数
                FEA_RED, // 赤牌の保持
                FEA_ALL,
            };
            
            constexpr int feaNumTable[] = {
                1,
                1,
                1,
            };
            
            constexpr int FEA_NUM(unsigned int f){
                return feaNumTable[f];
            }
            
            constexpr int FEA_IDX(unsigned int f){
                return (f == 0) ? 0 : (FEA_IDX(f - 1) + FEA_NUM(f - 1));
            }
            
            constexpr int FEA_NUM_ALL = FEA_IDX(FEA_ALL);
            
            int commentToModelParam(std::ofstream& ofs, const double *const param){
                return 0;
            }
        }
        
        using TurnActionPolicy = SoftmaxPolicy<TurnActionPolicySpace::FEA_NUM_ALL, 1>;
        using TurnActionPolicyLearner = SoftmaxPolicyLearner<TurnActionPolicy>;
        
        int foutComment(const TurnActionPolicy& mdl, const std::string& fName){
            std::ofstream ofs(fName, std::ios::out);
            return TurnActionPolicySpace::commentToModelParam(ofs, mdl.param_);
        }
        
        template<int M = 0, class action_t, class field_t, class model_t>
        void calcTurnActionPolicyScore(double *const score, action_t *const action, const int actions,
                                       field_t& field, const ExtPiece drawn, const model_t& mdl){
            
            using namespace TurnActionPolicySpace;
            
            mdl.template initCalculatingScore<M>(actions);
            
            Player turnPlayer = field.turnPlayer;
            Hand& hand = field.hand[turnPlayer];
            
            /*auto finishPolicyFunc = [&](action_t *const pact)->double{
             // あがる場合の方策計算
             //int bonusScore = calcBonusScore(hand); // 実際のゲームでの得点計算
             return 0;
             };
             
             auto policyFunc = [&](action_t *const pact)->double{
             // 方策計算関数
             pol.template initCalculatingCandidateScore<M>();
             double s = 0;
             
             // 基本性質計算
             pac->nextHandInfo.minimumSteps = calcAcceptableBits();
             };
             auto reachDiffPolicyFunc[&](action_t *const pact, const double os)->double{
             // リーチをかける場合の差分方策計算関数
             const double s = os;
             
             return s + 1;
             };
             
             if(field.isInReach(turnPlayer)){
             // リーチ中はあがりかツモ切りしかできないので手牌にツモが加わっていないので注意!
             if(hand.){
             // ツモあがりを生成
             pa->
             }
             // ツモ切りを生成 TODO: あがれる場合にはどの程度選ばれうる?
             }
             
             // 現時点でシャンテン数が0かつあがれる役があるならツモあがりを生成
             if(hand.minimumSteps == 0){
             // TODO: 役があるかチェック
             pac->setFinish(); ++pa;
             }*/
            
            NextHandInfo nhi[N_MAX_TURN_ACTIONS];
            
            for(int i = 0; i < actions; ++i){
                double s = 0;
                const action_t a = action[i];
                DiffHandInfo dhi;
                
                doTurnAction(&hand, a, &dhi, &nhi[i]);
                
                PieceExistance acceptable;
                int steps = calcMinimumSteps(hand, &acceptable);
                
                s -= steps;
                s += acceptable.count() / N_PIECES;
                s += 0.15 * hand.countAllReds();
                
                undoTurnAction(&hand, a, dhi);
                
                score[i] = s;
                
                FASSERT(s,);
                double exps = exp(s / Settings::simulationTemperatureTurn);
                FASSERT(exps, cerr << "s = " << s << " T = " << Settings::simulationTemperatureTurn << endl;);
                
                mdl.template feedCandidateScore<M>(exps);
            }
        }
#undef Foo
#undef FooX
    }
}
        
#endif // DCURLING_AYUMU_POLICY_HPP_