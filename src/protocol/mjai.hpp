/*
 mjai.hpp
 Katsuki Ohto
 */

// mjaiプロトコルとのやりとり

#ifndef MAHJONG_PROTOCOL_MJAI_HPP_
#define MAHJONG_PROTOCOL_MJAI_HPP_

#include "../mahjong.hpp"

namespace Mahjong{
    namespace mjai{
        
        MatchType toMatchType(const std::string& str){
            if(str == "tonpu"){
                return MatchType::SINGLE;
            }
            return MatchType::DOUBLE;
        }
        
        PieceType toPieceType(const char ptc){
            std::string::size_type index = pieceTypeChar.find_first_of(ptc);
            if(index == std::string::npos){
                return PIECE_TYPE_NONE;
            }
            return static_cast<PieceType>(index + int(PIECE_TYPE_MIN));
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
            PieceType pt = toPieceType(str[1]);
            if(!isNumberType(pt)){ return EXT_PIECE_NONE; }
            if(str.size() > 2){ // 赤牌
                return toExtPiece(toPiece(pt, r), true);
            }
            return toExtPiece(toPiece(pt, r));
        }
    }
}

#endif // MAHJONG_PROTOCOL_MJAI_HPP_