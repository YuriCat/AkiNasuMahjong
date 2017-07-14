/*
 tenho.hpp
 Katsuki Ohto
 */

// 天鳳棋譜ファイルの読み込み
// 日本語が使われている

#ifndef MAHJONG_PROTOCOL_TENHO_HPP_
#define MAHJONG_PROTOCOL_TENHO_HPP_

#include "../mahjong.hpp"

namespace Mahjong{
    namespace Tenho{
        
        const std::string pieceTypeRedChar = "MPS";
        const std::vector<std::string> windString = {"東", "南", "西", "北"};
        const std::vector<std::string> dragonString = {"白", "発", "中"};
        
        std::map<std::string, Bonus> stringBonusMap;
        std::map<std::string, FieldStatus> stringStatusMap;
        
        MatchType toMatchType(const std::string& str){
            if(str.find_first_of("東") != std::string::npos){
                return MatchType::SINGLE;
            }
            return MatchType::DOUBLE;
        }
        
        ExtPieceType toExtPieceType(const char ptc){
            std::string::size_type index = pieceTypeChar.find_first_of(ptc);
            if(index != std::string::npos){
                return toExtPieceType(static_cast<PieceType>(index + int(PIECE_TYPE_MIN)), false);
            }
            std::string::size_type rindex = pieceTypeRedChar.find_first_of(ptc);
            if(rindex != std::string::npos){
                return toExtPieceType(static_cast<PieceType>(index + int(PIECE_TYPE_MIN)), true);
            }
            return EXT_PIECE_TYPE_NONE;
        }
        
        Wind toWind(const std::string& str){
            if(str.size() != 1){ return WIND_NONE; }
            std::size_t index = find(windString.begin(), windString.end(), str) - windString.begin();
            if(index >= windString.size()){
                return WIND_NONE;
            }
            return static_cast<Wind>(index + int(WIND_MIN));
        }
        
        Dragon toDragon(const std::string& str){
            if(str.size() != 1){ return DRAGON_NONE; }
            std::size_t index = find(dragonString.begin(), dragonString.end(), str) - dragonString.begin();
            if(index >= dragonString.size()){
                return DRAGON_NONE;
            }
            return static_cast<Dragon>(index + int(DRAGON_MIN));
        }
        
        ExtPiece toExtPiece(const std::string& str){
            Wind w = toWind(str);
            if(examWind(w)){ // 風牌
                return toExtPiece(toWindPiece(w));
            }
            Dragon d = toDragon(str);
            if(examDragon(d)){ // 三元牌
                return toExtPiece(toDragonPiece(d));
            }
            // 数牌
            if(str.size() < 2){ return EXT_PIECE_NONE; }
            Rank r = toRank(str[0]);
            if(!examRank(r)){ return EXT_PIECE_NONE; }
            ExtPieceType ept = toExtPieceType(str[1]);
            return toExtPiece(ept, r);
        }
        
        ExtPieceSet toExtPieceSet(const std::string& str){
            ExtPieceSet eps;
            eps.clear();
            int index = 0;
            while(index < str.size()){
                for(int i = index + 2; i <= min((int)str.size(), index + 4); ++i){
                    ExtPiece ep = toExtPiece(str.substr(index, i - index));
                    if(examExtPiece(ep)){
                        eps += ep;
                        index = i;
                        goto TO_NEXT_LOOP;
                    }
                }
                index += 1; // unfound
            TO_NEXT_LOOP:;
            }
            return eps;
        }
        
        /*PlayerTurnAction toPlayerTurnAction(const std::string& str){
            Player pn = atoi(str[0]);
            TurnAction a;
            a.clear();
            
        }*/
        
        int initTenho(){
            
            // ゲーム終了状態一覧初期化
            stringStatusMap["九種九牌"] = STATUS_DRAW_9T9P;
            stringStatusMap["ツモ"] = STATUS_DRAW_9T9P;
            stringStatusMap["ロン"] = STATUS_DRAW_9T9P;
            stringStatusMap["流局"] = STATUS_DRAW_EXHAUSTED;
            
            
            // 上がり役一覧初期化
            // stringBonusMap[""] = ;
            
            // 1翻
            stringBonusMap["立直"] = BONUS_REACH;
            stringBonusMap["一発"] = IMMEDIATE;
            stringBonusMap["平和"] = ALL_RUNS;
            stringBonusMap["自風"] = OWN_WIND;
            stringBonusMap["場風"] = FIELD_WIND;
            stringBonusMap["断幺九"] = ALL_SIMPLES;
            stringBonusMap["門前清自摸和"] = CONCEALED_DRAW;
            stringBonusMap["役牌"] = BONUS_PFC;
            stringBonusMap["一盃口"] = DOUBLE_RUN;
            stringBonusMap["海底摸月"] = LAST_DRAW_WIN;
            
            // 2翻 -> 1翻
            stringBonusMap["一気通貫"] = BONUS_REACH;
            stringBonusMap["三色同順"] = THREE_COLOR_RUNS;
            stringBonusMap["混全帯幺九"] = MIXED_OUTSIDE_HAND;
            
            // 2翻
            stringBonusMap["七対子"] = ALL_DOUBLES;
            stringBonusMap["対々和"] = ALL_TRIPLES;
            
            // 3翻 -> 2翻
            stringBonusMap["混一色"] = HALF_FLUSH;
            stringBonusMap["純全帯幺九"] = PURE_OUTSIDE_HAND;
            
            // 6翻 -> 5翻
            stringBonusMap["清一色"] = FULL_FLUSH;
            
            // 加点
            stringBonusMap["ドラ"] = NORMAL_DORA;
            stringBonusMap["赤ドラ"] = RED_DORA;
            stringBonusMap["裏ドラ"] = BACK_DORA;
            
            return 0;
        }
    }
}

#endif // MAHJONG_PROTOCOL_TENHO_HPP_