/*
 deal.hpp
 Katsuki Ohto
 */

// 牌の配布

#ifndef MAHJONG_EGGPLANT_DEAL_HPP_
#define MAHJONG_EGGPLANT_DEAL_HPP_

#include "../settings.h"
#include "../mahjong.hpp"
#include "../structure/world.hpp"
#include "../structure/record.hpp"
#include "../structure/field.hpp"

namespace Mahjong{
    namespace Eggplant{
        
        void remove(std::array<uint8_t, N_ALL_PIECES>& indice,
                    std::array<uint8_t, N_ALL_PIECES>& inv,
                    std::size_t& size,
                    std::size_t pos){
            indice[pos] = indice[--size];
            inv[indice[pos]] = pos;
        }
        
        template<class wall_t, class hands_t, class ucpieces_t, class field_t, class dice_t>
        void dealPiecesAllRandom(wall_t *const pwall,
                                 hands_t *const phands,
                                 ucpieces_t *const puncertain,
                                 const field_t& field,
                                 dice_t *const pdice){
            // 現在見えていない牌の中からランダムに割り当てるだけ
            ExtPieceSet uncertain = field.myUncertain();
            BitArray32<8, N_PIECE_TYPES> typeQty = 0;
            uint32_t sumQty = 0;
            for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
                uint32_t q = uncertain[pt].sum();
                typeQty.set(pt, q);
                sumQty += q;
            }
            // 山牌
            for(int i = field.turn + WALL_INDEX_START + 1; i < N_WALL_PIECES; ++i){ // 次の手番のツモ牌から
                if((*pwall)[i] == EXT_PIECE_NONE){ // 未確定
                    ExtPiece ep = dealExtPiece(uncertain, typeQty, sumQty, pdice); // ランダムに1つ取り出し
                    (*pwall)[i] = ep;
                    uncertain -= ep;
                    typeQty.subtr(toPieceType(ep), 1);
                    sumQty -= 1;
                }
            }
            // 手牌 不明な枚数ごとにランダム分割
            std::array<ExtPieceSet, N_PLAYERS> eps;
            BitArray32<4, N_PLAYERS> uncertainQty = 0;
            uint32_t uncertainSumQty = 0;
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                uint32_t tmp = field.pieces[pn] - field.hand[pn].allPieces; // 未確定の枚数
                uncertainQty.set(pn, tmp);
                uncertainSumQty += tmp;
                eps[pn].clear();
            }
            ASSERT(sumQty == uncertainQty.sum(),
                   cerr << "candidate " << sumQty << " pieces but dealt to " << uncertainQty << endl;
                   cerr << field.myUncertain() <<  endl;
                   for(Player pn = 0; pn < N_PLAYERS; ++pn){
                       cerr << field.hand[pn].allPieces << endl;
                   };);
            dealPieces(uncertain, &eps, uncertainQty, uncertainSumQty, pdice);
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                if(pn != field.myPlayerNum){ // 自分の手牌は再設定しなくてよい
                    (*phands)[pn].setConcealedInfoAll(eps[pn]);
                    (*puncertain)[pn] -= eps[pn];
                }
            }
        }
        
        /*template<class wall_t, class field_t, class sharedData_t, class threadTools_t>
        void dealPieces(wall_t *const dst, const field_t& field,
                        const sharedData_t& shared, threadTools_t *const ptools){
            // インデックス配列を準備
            std::array<uint8_t, N_ALL_PIECES> indice; // インデックスから牌への変換
            std::array<uint8_t, (PIECE_MAX + 1) * N_ONE_PIECE> inv; // 牌からインデックスへの変換
            std::size_t size = N_ALL_PIECES;
            
            int cnt = 0;
            iterateNaivePieces([&](Piece p)->void{
                for(int i = 0; i < N_ONE_PIECE; ++p){
                    int temp = (int)p * N_ONE_PIECE + i;
                    indice[cnt] = temp;
                    inv[temp] = cnt;
                    ++cnt;
                }
            });
            
            // すでに位置確定した牌の情報を設定
            for(int i = 0; i < field.turn + WALL_INDEX_START; ++i){
                if(pwall[indice[i]] != EXT_PIECE_NONE){ // 確定
                    remove(indice, inv, size, i);
                }
            }
            
            // 試合の最初に出た牌から順に配る
            for(int t = 0; t < field.turn; ++t){
                const auto turnRecord = shared.gameRecord.turn(t);
                ExtPiece discarded = turnRecord.turnAction.discarded();
                //if()
            }
        }*/
    }
}

#endif // MAHJONG_EGGPLANT_DEAL_HPP_