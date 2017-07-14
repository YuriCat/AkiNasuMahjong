/*
 score.hpp
 Katsuki Ohto
 */

#ifndef MAHJONG_STRUCTURE_SCORE_HPP_
#define MAHJONG_STRUCTURE_SCORE_HPP_

#include "piece.hpp"

namespace Mahjong{
    
    /**************************役**************************/
    
    // 役と得点計算
    enum Bonus{
        // 1翻
        BONUS_REACH, // リーチ
        IMMEDIATE, // リーチ一発
        CONCEALED_DRAW, // 門前清自摸和(メンゼンツモ)
        ALL_RUNS, // 平和(ピンフ)
        ALL_SIMPLES, // 断么(タンヤオ)
        DOUBLE_RUN, // 一盃口(イーペイコウ) 同じ順子が2つ
        BONUS_PFC, // 白発中 これらの刻子(槓子)がある
        OWN_WIND, // 門風牌(メンフォンパイ) 自風と同じ刻子(槓子)
        FIELD_WIND, // 荘風牌(チャンフォンパイ) 場風と同じ刻子(槓子)
        LAST_DRAW_WIN, // 河底撈魚(ホウテイラオユイ) 最後の引き牌でツモあがり
        LAST_RESPONSE_WIN, // 海底撈月(ハイテイラオユエ) 最後の捨て牌でロンあがり
        
        // 2翻 -> 1翻
        FULL_STRAIGHT, // 一気通貫(イッキツウカン) 数牌の1~9
        THREE_COLOR_RUNS, // 三色同順(サンショクドウジュン) 全ての数牌タイプで同じ数字の順子を揃える
        MIXED_OUTSIDE_HAND, // 混全帯ヤオ九(チャンタ) 字牌 + 全ての面子に1か9が絡む
        
        // 2翻
        ALL_DOUBLES, // 七対子(チートイツ) 同じ牌2つを揃える
        ALL_TRIPLES, // 対々和(トイトイ) 4つ全て刻子(槓子)
        THREE_CONCEALED_TRIPLES, // 三暗刻(サンアンコウ) 3つの暗刻(暗槓)
        
        // 3翻 -> 2翻
        HALF_FLUSH, // 混一色(ホンイツ) 1種類の数牌と字牌だけ使用
        PURE_OUTSIDE_HAND, // 純全帯幺九(ジュンチャン) 全ての面子に1か9が絡む
        
        // 6翻 -> 5翻
        FULL_FLUSH, // 清一色(チンイツ) 1種類の数牌だけ使用
        
        // 役満
        ALL_CONCEALED_TRIPLES, // 四暗刻(スーアンコウ) 4つ全て暗刻(暗槓)
        FULL_ORPHANS, // 国士無双 数牌の端と字牌1つ以上
        FULL_HONORS, // 大三元 三元牌の役を全て作る
        HEAVEN_OWNER, // 天和(テンホー) 親が初手でツモあがり
        HEAVEN_GENERAL, // 千和(チーホー) 親以外が初手でツモあがり(誰も鳴いていない状況限定)
        
        // ダブル役満
        ALL_CONCEALED_TRIPLES_HEAD, // 四暗刻単騎待ち 四暗刻かつ雀頭待ちであがり
        
        // 加点(役では無いので上がりには使えない)
        NORMAL_DORA, // 通常のドラ
        RED_DORA, // 赤ドラ
        BACK_DORA, // 裏ドラ
    };
    
    enum BonusFlag{
        // ドラ全体
        FLAG_DORA = (1ULL << NORMAL_DORA) | (1 << RED_DORA) | (1 << BACK_DORA),
        // 食い下がり(門前でない場合は1翻下がる)
        FLAG_RESPONSE_DOWN = (1ULL << FULL_STRAIGHT),
    };
    
    // ボーナスの付き方を表すビット集合
    using BonusStatus = BitSet64;
    
    bool finishBonus(const BonusStatus& bs)noexcept{ // 上がり条件になるBonusに限定する
        return bs & ~FLAG_DORA;
    }
    
    /**************************得点**************************/
    
    enum Score : int{
        SCORE_INITIAL = 25000, // 初期状態
        SCORE_END = 0, // 飛びライン
        SCORE_REACH = 1000, // リーチの積み棒
        SCORE_EXHAUSTIVE_DRAW_BASE = 1000, // 流局時の一口
    };
    
    ENABLE_ARITHMETIC_OPERATORS(Score)
    
    std::ostream& operator <<(std::ostream& ost, const Score& sc){
        ost << static_cast<int>(sc);
        return ost;
    }
    
    constexpr bool isEndScore(Score s){ // 飛びで強制終了するかの判定
        return s <= SCORE_END;
    }
    
    /**************************得点計算**************************/
    
    template<class field_t>
    void calcScoreIrregularDraw(field_t *const pfield){ // 途中流局時
        // 得点移動なし
        pfield->drawToLeaveOwner(); // 親継続
    }
    
    template<class field_t>
    void calcScoreExhaustiveDraw(field_t *const pfield){ // 流局時
        // 各プレーヤーが聴牌状態であるか調べる
        BitSet8 fishing(0);
        int NFishings = 0;
        for(Player pn = 0; pn < N_PLAYERS; ++pn){
            if(pfield->hand[pn].isFishing()){
                fishing.set(pn);
                NFishings += 1;
            }
        }
        if(NFishings == 0 || NFishings == N_PLAYERS){
            // 得点移動なし
        }else{
            Score got, given;
            if(NFishings <= N_PLAYERS / 2){ // 聴牌人数の方が少ない
                got = (N_PLAYERS - NFishings) * SCORE_EXHAUSTIVE_DRAW_BASE / NFishings;
                given = -SCORE_EXHAUSTIVE_DRAW_BASE;
            }else{ // 聴牌人数の方が多い
                got = SCORE_EXHAUSTIVE_DRAW_BASE;
                given = -NFishings * SCORE_EXHAUSTIVE_DRAW_BASE / (N_PLAYERS - NFishings);
            }
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                if(fishing.test(pn)){
                    pfield->score[pn] += got;
                }else{
                    pfield->score[pn] += given;
                }
            }
        }
        if(!fishing.test(pfield->owner)){
            // 親が聴牌状態でなければ親交代
            pfield->drawToSwitchOwner();
        }else{
            pfield->drawToLeaveOwner();
        }
    }
    
    template<class field_t>
    int calcScoreWin(field_t *const pfield, BitSet32 wonPlayers, bool drawWin,
                     std::array<BonusStatus, N_PLAYERS> *const pstatus){
        // 得点計算
        // 計算が複雑なのでここで足し引き全部やってしまう
        while(wonPlayers.any()){
            const Player pn = wonPlayers.bsf();
            wonPlayers.pop_lsb();
            BonusStatus bs(0);
            int bonus = 0;
            const int maxBonus = 100;
            Score score = static_cast<Score>(0);
            const Hand& hand = pfield->hand[pn];
            
            PieceSet mine = hand.piece;
            const ExtPieceSet myAll = hand.piece + hand.openedPiece;
            const bool concealed = pfield->isConcealed(pn);
            
            while(1){
                // 得点の高い役が優先されることがあるのでその順で考える
                if(pfield->lastResponseTurn < 0){
                    if(pfield->turn == 0){ // 天和
                        bs.set(HEAVEN_OWNER); bonus = maxBonus; break;
                    }
                    if(pfield->isFirstTurn()){ // 地和
                        bs.set(HEAVEN_GENERAL); bonus = maxBonus; break;
                    }
                }
                if(concealed && ((hand.pqr & PQR256_124).sum1() == 1)){ // 四暗刻
                    bs.set(ALL_CONCEALED_TRIPLES); bonus = maxBonus; break;
                }
                if(isFullOrphansSc(hand.sc)){ // 国士無双
                    bs.set(FULL_ORPHANS); bonus = maxBonus; break;
                }
                unsigned int pfc = countBits((hand.pqr[HONOR] & TYPE_MASK_DRAGONS_ALL & PQR_34).data()); // 三元牌の役の数
                if(pfc == N_DRAGONS){ // 大三元
                    bs.set(FULL_HONORS); bonus = maxBonus; break;
                }
                
                // 役満以外
                if(pfield->isInReach(pn)){ // リーチ
                    bs.set(BONUS_REACH); bonus += 1;
                    if(pfield->isImmediate(pn)){ // リーチ一発
                        bs.set(IMMEDIATE); bonus += 1;
                    }
                }
                if(isAllDoubles(myAll)){ // 七対子
                    bs.set(ALL_DOUBLES); bonus += 1;
                }else{
                    if(pfc){ // 三元牌の役
                        bs.set(BONUS_PFC); bonus += pfc;
                    }
                }
                break;
            }
            (*pstatus)[pn] = bs;
        }
        if(!wonPlayers.test(pfield->owner)){
            // 親が上がりでなければ親交代
            pfield->winToSwitchOwner();
        }else{
            pfield->winToLeaveOwner();
        }
        return 0;
    }
}

#endif // MAHJONG_MAHJONG_HPP_