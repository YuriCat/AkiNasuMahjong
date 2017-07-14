/*
 monteCarlo.hpp
 Katsuki Ohto
 */

// モンテカルロ

#ifndef MAHJONG_EGGPLANT_MONTECARLO_HPP_
#define MAHJONG_EGGPLANT_MONTECARLO_HPP_

#include "../settings.h"
#include "../mahjong.hpp"
#include "../structure/world.hpp"
#include "../structure/record.hpp"
#include "../structure/field.hpp"

#include "eggplant.h"
#include "eggplantStructure.hpp"

#include "value.hpp"
#include "deal.hpp"
#include "turnActionPolicy.hpp"

namespace Mahjong{
    namespace Eggplant{
        
        struct SimulationResult{
            Score nextScore;
            std::array<double, N_PLAYERS> distribution; // 予測順位
            bool drawWin, responseWin, present;
            SimulationResult(){
                drawWin = responseWin = present = false;
            }
        };
        
        template<class field_t, class sharedData_t, class threadTools_t>
        inline void doSimulation(SimulationResult *const presult,
                                 field_t *const pfield,
                                 FieldTemporalInfo *const pfti,
                                 const Player simulationOwner,
                                 const FieldStatus status,
                                 sharedData_t *const pshared,
                                 threadTools_t *const ptools){
            
            auto *const pdice = &ptools->dice;
            BitSet32 wonPlayers = 0; // 勝利したプレーヤー集合(複数の場合がある)
            std::array<TurnAction, N_MAX_TURN_ACTIONS> taBuffer; // 生成バッファ
            std::array<AddedAction, N_MAX_ADDED_ACTIONS> aaBuffer; // 生成バッファ
            std::array<ResponseAction, N_MAX_RESPONSE_ACTIONS> raBuffer; // 生成バッファ
            
            switch(status){
                // 最初の行動指定
                case STATUS_TURN_PROCESS: goto TURN_PROCESS; break;
                case STATUS_ADDED_PROCESS: goto ADDED_OTHER_RESPONSE; break;
                case STATUS_RESPONSE_PROCESS: goto DISCARDED_OTHER_RESPONSE; break;
                // 状態から開始
                default: break;
            }
            
            // 以下試合ループ
        TURN_START:{
            if(pfield->turn >= N_TURNS){ goto EXHAUSTED_DRAW; }
            
            ASSERT(pfield->examInSimulation(), cerr << pfield->toDebugString(););
            DERR << pfield->toString();
            Player turnPlayer = pfield->turnPlayer;
            // ツモ
            ExtPiece drawn = pfield->wall[pfield->turn + WALL_INDEX_START];
            pfield->setTurn(turnPlayer, drawn);
            if(pfield->isInReach(turnPlayer)){ // リーチ時
                // あがれるなら上がる
                if(pfield->hand[turnPlayer].isAcceptable(toPiece(drawn))){
                    DERR << "draw-win" << endl;
                    wonPlayers.set(turnPlayer);
                    presult->drawWin = true;
                    
                    std::array<BonusStatus, N_PLAYERS> bonus;
                    calcScoreWin(pfield, wonPlayers, presult->drawWin, &bonus);
                    goto END_GAME;
                }
                pfti->turnAction.clear().setDiscarded(drawn).setParrot();
            }else{ // リーチ時以外
                const int actions = genTurnActions(taBuffer.data(), turnPlayer, *pfield, drawn);
                double score[N_MAX_TURN_ACTIONS];
                calcTurnActionPolicyScore(score, taBuffer.data(), actions, *pfield, drawn,
                                          pshared->baseTurnActionPolicy);
                MaxSelector selector(score, actions);
                int index = selector.select(pdice); // 行動選択
                pfti->turnAction = taBuffer[index];
            }
        }
        TURN_PROCESS:{
            ExtPiece drawn = pfield->wall[pfield->turn + WALL_INDEX_START];
            DERR << "turn : " << pfield->turnPlayer << ", " << drawn << " -> " << pfti->turnAction << endl;
            if(pfti->turnAction.drawKong()){ // 暗槓
                DERR << "draw kong " << pfti->turnAction.toKongMeld() << endl;
                pfield->drawKong(pfield->turnPlayer, pfti->turnAction.toKongMeld(),
                                 pfti->turnAction.toKongConsumed());
                pfti->afterKong = true;
                pfield->procTurn(pfield->turnPlayer);
                goto TURN_START;
            }else if(pfti->turnAction.addKong()){ // 小明槓
                DERR << "added kong " << pfti->turnAction.extPiece() << endl;
                goto ADDED;
            }else{ // 打牌
                DERR << "discarded " << pfti->turnAction.discarded() << endl;
                pfield->discard(pfield->turnPlayer, pfti->turnAction.discarded(), pfti->turnAction.parrot());
                pfti->afterKong = false;
                if(pfti->turnAction.reach()){
                    pfield->reach(pfield->turnPlayer); // リーチ処理
                    if(pfield->isAllReach()){ // 四家立直
                        DERR << "all reach" << endl;
                        calcScoreIrregularDraw(pfield); // 連荘の場合の引き分け処理
                        goto END_GAME;
                    }
                }
                goto DISCARDED;
            }
        }
        ADDED: // 槍槓思考開始
            /*for(Player pn = 0; pn < N_PLAYERS; ++pn){
                if(pn != turnPlayer){
                    const int actions = genArrowActions()
                }
            }*/
            
        ADDED_OTHER_RESPONSE: // 槍槓他プレーヤーのみ思考開始
            
        ADDED_PROCESS:{
            // 槍槓処理
            BitSet8 addedActionPlayers = 0;
            // TODO
            if(addedActionPlayers){ // 槍槓
                // TODO
            }else{
                pfield->addKong(pfield->turnPlayer, pfti->turnAction.extPiece());
                pfti->afterKong = true;
                pfield->procTurn(pfield->turnPlayer);
                goto TURN_START;
            }
        }
        DISCARDED:{
            // 反応行動
            ExtPiece discarded = pfti->turnAction.discarded();
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                if(pn != pfield->turnPlayer){
                    const int actions = genResponseActions(raBuffer.data(), *pfield, discarded, pn);
                    // TODO: 方策
                    pfti->responseAction[pn] = raBuffer[0];
                }
            }
            goto RESPONSE_PROCESS;
        }
        DISCARDED_OTHER_RESPONSE:{
            // 反応行動(simulationOwner以外)
            ExtPiece discarded = pfti->turnAction.discarded();
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                if(pn != pfield->turnPlayer && pn != simulationOwner){
                    const int actions = genResponseActions(raBuffer.data(), *pfield, discarded, pn);
                    // TODO: 方策
                    pfti->responseAction[pn] = raBuffer[0];
                }
            }
            goto RESPONSE_PROCESS;
        }
        RESPONSE_PROCESS:{
            // 各プレーヤーの反応行動から優先度によって選ぶ
            ExtPiece discarded = pfield->lastDiscarded(pfield->turnPlayer);
            Player responsePlayer = NONE_PLAYER;
            ResponseAction realizedResponseAction;
            realizedResponseAction.clear();
            for(Player pn = 0; pn < pfield->turnPlayer; ++pn){
                if(pn != pfield->turnPlayer){
                    const ResponseAction& ra = pfti->responseAction[pn];
                    DERR << "response " << pn << " : " << ra << endl;
                    if(pfti->responseAction[pn].finish()){ // 上がりのあるプレーヤーがいるか調べる
                        wonPlayers.set(pn);
                    }else if(isPreferred(ra, realizedResponseAction)){ // 優先度高い
                        responsePlayer = pn;
                        realizedResponseAction = ra;
                    }
                }
            }
            DERR << "realized response : " << responsePlayer << ", " << realizedResponseAction << endl;
            if(wonPlayers.any()){ goto WIN_PROCESS; }
            if(realizedResponseAction.any()){
                if(realizedResponseAction.chow()){
                    pfield->chow(responsePlayer, realizedResponseAction, discarded,
                                 realizedResponseAction.toConsumed() - discarded);
                }else if(realizedResponseAction.pong()){
                    pfield->pong(responsePlayer, realizedResponseAction, discarded,
                                 realizedResponseAction.toConsumed() - discarded);
                }else if(realizedResponseAction.responseKong()){
                    pfield->responseKong(responsePlayer, realizedResponseAction, discarded,
                                         realizedResponseAction.toConsumed() - discarded);
                }
                pfield->procTurn(nextPlayer(responsePlayer));
            }else{
                pfield->procTurn();
            }
            
            goto TURN_START;
        }
        WIN_PROCESS:{
            // 勝者が出た場合の処理
            if(wonPlayers.any()){
                if(wonPlayers.count() >= N_WON_PLAYERS_DRAW){ // 上がりプレーヤーが多すぎるときは途中流局
                    calcScoreIrregularDraw(pfield); // 連荘の場合の引き分け処理
                }else{ // 上がりが有効
                    std::array<BonusStatus, N_PLAYERS> bonus;
                    calcScoreWin(pfield, wonPlayers, presult->drawWin, &bonus);
                }
                goto END_GAME;
            }
        }
        EXHAUSTED_DRAW: // 何もなく通常の流局
            DERR << "exhausted draw" << endl;
            calcScoreExhaustiveDraw(pfield);
        END_GAME:
            DERR << "simulation owner score " << pfield->score[simulationOwner] << endl;
            presult->nextScore = pfield->score[simulationOwner];
            calcExpectedDistribution(*pfield, simulationOwner, &presult->distribution); // 得点等から予測勝率を計算
        }
        template<class action_t, class fti_t>
        FieldStatus setSimulationOwnerAction(Player, const action_t&, fti_t *const);
        
        template<class fti_t>
        FieldStatus setSimulationOwnerAction(Player pn, const TurnAction& a, fti_t *const pfti){
            pfti->afterKong = false;
            pfti->turnAction = a;
            return STATUS_TURN_PROCESS;
        }
        template<class fti_t>
        FieldStatus setSimulationOwnerAction(Player pn, const AddedAction& a, fti_t *const pfti){
            pfti->afterKong = false;
            pfti->addedAction[pn] = a;
            return STATUS_ADDED_PROCESS;
        }
        template<class fti_t>
        FieldStatus setSimulationOwnerAction(Player pn, const ResponseAction& a, fti_t *const pfti){
            pfti->afterKong = false;
            pfti->responseAction[pn] = a;
            return STATUS_RESPONSE_PROCESS;
        }
        
        template<class root_t, class field_t, class sharedData_t, class threadTools_t>
        int monteCarloThread(int threadIndex,
                             root_t *const proot,
                             const field_t *const pfield,
                             sharedData_t *const pshared,
                             threadTools_t *const ptools){
            
            ClockMS clms;
            clms.start();
            int worldIndex = 0;
            while(clms.stop() < TIME_LIMIT_MS){
                // 不完全情報を設定
                field_t field = *pfield;
                dealPiecesAllRandom(&field.wall, &field.hand, &field.uncertain, field, &ptools->dice);
                field.myPlayerNum = NONE_PLAYER; // 客観s
                //cerr << field.wall << endl;
                // 全ての候補行動に対して同じ世界でシミュレーション
                for(int i = 0; i < proot->actions; ++i){
                    const auto& a = proot->action[i].action;
                    if(!a.finish()){
                        DERR << "world " << worldIndex << " action " << a << endl;
                        field_t tfield = field;
                        //doAction(&tfield, proot->action[i]);
                        FieldTemporalInfo fti;
                        // 初手の設定
                        FieldStatus status = setSimulationOwnerAction(pfield->myPlayerNum, a, &fti);
                        SimulationResult result;
                        doSimulation(&result, &tfield, &fti, pfield->myPlayerNum, status, pshared, ptools);
                        proot->feed(i, result);
                    }
                }
                worldIndex += 1;
            }
            return 0;
        }
        
        template<class root_t, class field_t, class sharedData_t, class threadTools_t>
        int doMonteCarloSearch(root_t *const proot,
                               const field_t& field,
                               sharedData_t *const pshared,
                               threadTools_t tools[]){
            
            if(Settings::NThreads > 1){
                // open threads
                std::vector<std::thread> thr;
                for(int ith = 0; ith < Settings::NThreads; ++ith){
                    thr.emplace_back(std::thread(&monteCarloThread<root_t, field_t, sharedData_t, threadTools_t>,
                                                 ith, proot, &field, pshared, &tools[ith]));
                }
                for(auto& th : thr){
                    th.join();
                }
            }else{
                // call function
                monteCarloThread<root_t, field_t, sharedData_t, threadTools_t>(0, proot, &field, pshared, &tools[0]);
            }
            return 0;
        }
    }
}

#endif // MAHJONG_EGGPLANT_MONTECARLO_HPP_