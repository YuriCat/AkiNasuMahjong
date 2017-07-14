/*
 turnAction.hpp
 Katsuki Ohto
 */

// ツモ手番行動用の方策

#ifndef MAHJONG_EGGPLANT_TURNACTION_HPP_
#define MAHJONG_EGGPLANT_TURNACTION_HPP_

#include "../mahjong.hpp"

namespace Mahjong{
   
#define Foo(f, i) {ASSERT(FEA_IDX(p) <= i && i < FEA_IDX((f) + 1),\
    cerr << "Foo() : illegal index " << (i) << " in " << FEA_IDX(f) << "~" << (FEA_IDX((f) + 1) - 1);\
    cerr << " (fi = " << (f) << ")" << endl; );\
    s += mdl.param(i); mdl.template feedFeatureScore<M>((i), 1.0);}
    
#define FooXC(f, i, x) {ASSERT(POL_IDX(p) <= i && i < POL_IDX((p) + 1),\
    cerr << "FooX() : illegal index " << (i) << " in " << FEA_IDX(f) << "~" << (FEA_IDX((f) + 1) - 1);\
    cerr << " (fi = " << (f) << ")" << endl; );\
    s += mdl.param(i) * (x); mdl.template feedFeatureScore<M>((i), (x));}
    
    namespace TurnPolicySpace{
        
        enum{
            FEA_MINIMUM_STEPS, // シャンテン数
            FEA_ACCEPTABLE_NUM, // 受け入れ数
            FEA_ALL,
        };
        
        constexpr int feaNumTable[] = {
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
    
    using TurnPolicy = SoftmaxPolicy<TurnPolicySpace::FEA_NUM_ALL>;
    using TurnPolicyLearner = SoftmaxPolicyLearner<TurnPolicy>;
    
    int foutComment(const TurnPolicy& mdl, const std::string& fName){
        std::ofstream ofs(fName, std::ios::out);
        return TurnPolicySpace::commentToModelParam(ofs, mdl.param_);
    }
    
    template<int M = 0, class action_t, class field_t, class model_t>
    void genTurnActionsToCalcPolicyScore
    (double *const score, action_t *const a,
     field_t& bd, const ExtPiece drawn, const model_t& mdl){
        
        using namespace TurnPolicySpace;

        pol.template initCalculatingScore<M>(NMoves);
        
        Player turnPlayer = field.turnPlayer;
        Hand& hand = field.hand[turnPlayer];
        action_t *pa = a;
        
        auto finishPolicyFunc = [&](action_t *const pact)->double{
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
        }
        

        
        iteratePieces
        ([&](Piece p)->void{ // normal callback
            if(hand[p] > 0){
                pac->clear().setDiscarded(toExtPiece(p));
                score[i] = policyFunc();
                i += 1;
                if(pac[i - 1].nextHandInfo.minimumSteps() == 1){ // シャンテン数1ならリーチ生成
                    
                }
            }
        },
         [&pac, &hand](Piece p)->void{ // red callback
             if(hand[p] > 0){
                 if(hand.red(toPieceType(p))){
                     pac->clear().setDiscarded(toExtPiece(p, true)); ++pac;
                     if(hand[p] > 1){ // 赤牌でないものを生成
                         pac->clear().setDiscarded(toExtPiece(p)); ++pac;
                     }
                 }else{
                     pac->clear().setDiscarded(toExtPiece(p)); ++pac;
                 }
             }
         });
            
    
            
            double s = 0;
            
            
            score[i] = s;
            
            FASSERT(s,);
            double exps = exp(s / pol.temperature());
            FASSERT(exps, cerr << "s = " << s << " T = " << pol.temperature() << endl;);
            
            pol.template feedCandidateScore<M>(exps);
        }
    }
#undef Foo
#undef FooX
    
    template<class move_t, class board_t, class policy_t, class tools_t>
    void playWithPolicy(move_t *const pmv, const board_t& bd, const policy_t& pol, tools_t *const ptools){
        
        MovePolicy buf[64];
        
#ifdef USE_HANDMADE_MOVES
        const int NMoves = genChosenVMove(buf, bd);
#else
        const int NMoves = genSimpleVMove(buf, bd);
#endif
        
        int chosen = 0;
#ifdef USE_POLICY_SCORE
        if(NMoves != 1){
            //DERR << NMoves << " moves." << endl;
            calcPolicyScore<0>(buf, NMoves, bd, pol);
            fpn_t sum = 0;
            for(int i = 0; i < NMoves; ++i){
                buf[i].weight = exp(buf[i].weight / pol.temperature());
                sum += buf[i].weight;
                //DERR << i << ",";
            }
            sum *= ptools->ddice.drand();
            
            for(int i = 0; i < NMoves; ++i){
                sum -= buf[i].weight;
                if(sum <= 0){ chosen = i; break; }
            }
            
            //for(int i = 0; i < NMoves; ++i){
            //DERR << buf[i] << " " << buf[i].weight;
            //if(chosen == i){ DERR << " <-"; }
            //DERR << endl;
            //}
            //getchar();
        }
#else
        chosen = ptools->ddice.rand() % NMoves;
#endif
        //cerr << buf[chosen] << endl;
        realizeMove(pmv, bd, buf[chosen]);
    }
    
    template<class move_t, class board_t, class policy_t, class tools_t>
    void playWithBestPolicy(move_t *const pmv, const board_t& bd, const policy_t& pol, tools_t *const ptools){
        
        std::array<MovePolicy, 64> buf;
        
#ifdef USE_HANDMADE_MOVES
        const int NMoves = genChosenVMove(&buf[0], bd);
#else
        const int NMoves = genSimpleVMove(&buf[0], bd);
#endif
        
        int chosen = 0;
#ifdef USE_POLICY_SCORE
        if(NMoves != 1){
            int bestIndex[NMoves];
            bestIndex[0] = -1;
            int NBestMoves = 0;
            fpn_t bestScore = -99999;
            //DERR << NMoves << " moves." << endl;
            calcPolicyScore<0>(&buf[0], NMoves, bd, pol);
            
            for(int i = 0; i < NMoves; ++i){
                if(buf[i].weight > bestScore){
                    bestScore = buf[i].weight;
                    bestIndex[0] = i;
                    NBestMoves = 1;
                }else if(buf[i].weight == bestScore){
                    bestIndex[NBestMoves] = i;
                    ++NBestMoves;
                }
            }
            
            cerr << NMoves << " moves" << endl;;
            fpn_t probSum = 0;
            for(int i = 0; i < NMoves; ++i){
                probSum += exp(buf[i].weight / pol.temperature());
            }
            std::array<MovePolicy, 64> tmpBuf = buf;
            std::stable_sort(tmpBuf.begin(), tmpBuf.begin() + NMoves, [](const auto& m0, const auto& m1)->bool{
                return m0.weight >= m1.weight;
            });
            for(int i = 0; i < NMoves; ++i){
                fMoveXY<> fmv;
                realizeMove(&fmv, bd, tmpBuf[i]);
                cerr << i << " " << tmpBuf[i] << " " << exp(tmpBuf[i].weight / pol.temperature()) / probSum << " " << fmv << endl;
            }
            
            chosen = (NBestMoves <= 1) ? bestIndex[0] : bestIndex[ptools->dice.rand() % NBestMoves];
        }else{
            chosen = 0;
        }
#else
        chosen = ptools->ddice.rand() % NMoves;
#endif
        realizeMove(pmv, bd, buf[chosen]);
    }
}

#endif // DCURLING_AYUMU_POLICY_HPP_