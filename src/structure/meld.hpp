/*
 meld.hpp
 Katsuki Ohto
 */

// 役の表現

#ifndef MAHJONG_STRUCTURE_MELD_HPP_
#define MAHJONG_STRUCTURE_MELD_HPP_

#include "base.hpp"
#include "piece.hpp"

namespace Mahjong{
    
    /**************************鳴いた役の情報**************************/
    
    using MeldDataType = uint32_t;
    
    enum MeldInfoIndex{
        MELD_INDEX_QTY    = 0, // 枚数 4ビット(3ビットで良いけど...)
        MELD_INDEX_SEQ    = 4, // sequeunce(チー)フラグ
        MELD_INDEX_GROUP  = 5, // group(ポン, カン)フラグ
        MELD_INDEX_PIECE  = 8, // 基準となる牌
        MELD_INDEX_RED    = 14, // 赤を使うか(ExtPieceの赤フラグと共用)
    };
    
    enum MeldBaseMask : MeldDataType{
        MELD_BASE_QTY = (1 << 4) - 1,
        MELD_BASE_PIECE = (1 << 6) - 1,
        MELD_BASE_EXT_PIECE = (1 << (6 + 1)) - 1,
    };
    
    enum MeldFlag : MeldDataType{
        MELD_QTY,
        MELD_SEQ   = 1 << MELD_INDEX_SEQ,
        MELD_GROUP = 1 << MELD_INDEX_GROUP,
        MELD_PIECE = MELD_BASE_PIECE << MELD_INDEX_PIECE,
        MELD_EXT_PIECE = MELD_BASE_EXT_PIECE << MELD_INDEX_PIECE,
        MELD_RED   = 1 << MELD_INDEX_RED,
        
        // 以下包括で判定
        MELD_CHOW  = MELD_SEQ   | (N_CHOW_PIECES << MELD_INDEX_QTY),
        MELD_PONG  = MELD_GROUP | (N_PONG_PIECES << MELD_INDEX_QTY),
        MELD_KONG  = MELD_GROUP | (N_KONG_PIECES << MELD_INDEX_QTY),
    };
    
    // 役を構成する最小の牌の枚数
    constexpr int N_MIN_MELD_PIECES = cmin(cmin(N_CHOW_PIECES, N_PONG_PIECES), N_KONG_PIECES);
    // 一人の手牌から生成可能な役の数の最大数
    constexpr int N_MAX_MELDS = N_DEALT_PIECES / N_MIN_MELD_PIECES;
    
    template<typename data_t>
    struct MeldInRegister{
        
        using this_t = MeldInRegister<data_t>;
        data_t dat_;
        
        data_t data()const noexcept{
            return dat_;
        }
        bool any()const noexcept{
            return dat_ ? true : false;
        }
        data_t qty()const noexcept{
            return (dat_ >> MELD_INDEX_QTY) & MELD_BASE_QTY;
        }
        data_t chow()const noexcept{
            return dat_ & MELD_SEQ;
        }
        data_t group()const noexcept{
            return dat_ & MELD_GROUP;
        }
        bool pong()const noexcept{
            return holdsBits(dat_, static_cast<data_t>(MELD_PONG));
        }
        bool kong()const noexcept{
            return holdsBits(dat_, static_cast<data_t>(MELD_KONG));
        }
        Piece piece()const noexcept{
            return static_cast<Piece>((dat_ >> MELD_INDEX_PIECE) & MELD_BASE_PIECE);
        }
        ExtPiece extPiece()const noexcept{
            return static_cast<ExtPiece>((dat_ >> MELD_INDEX_PIECE) & MELD_BASE_EXT_PIECE);
        }
        data_t red()const noexcept{
            return dat_ & MELD_RED;
        }
        this_t& setQty(data_t q)noexcept{
            dat_ |= q << MELD_INDEX_QTY;
            return *this;
        }
        this_t& setSeq()noexcept{
            dat_ |= MELD_SEQ;
            return *this;
        }
        this_t& setGroup()noexcept{
            dat_ |= MELD_GROUP;
            return *this;
        }
        this_t& setKong()noexcept{
            dat_ |= MELD_KONG;
            return *this;
        }
        this_t& setPong()noexcept{
            dat_ |= MELD_PONG;
            return *this;
        }
        this_t& setChow()noexcept{
            dat_ |= MELD_CHOW;
            return *this;
        }
        this_t& setKong(Piece p)noexcept{
            dat_ |= MELD_KONG;
            dat_ |= static_cast<data_t>(p) << MELD_INDEX_PIECE;
            return *this;
        }
        this_t& setKong(ExtPiece ep)noexcept{
            dat_ |= MELD_KONG;
            dat_ |= static_cast<data_t>(ep) << MELD_INDEX_PIECE;
            return *this;
        }
        this_t& setPong(Piece p)noexcept{
            dat_ |= MELD_PONG;
            dat_ |= static_cast<data_t>(p) << MELD_INDEX_PIECE;
            return *this;
        }
        this_t& setChow(Piece p)noexcept{
            dat_ |= MELD_CHOW;
            dat_ |= static_cast<data_t>(p) << MELD_INDEX_PIECE;
            return *this;
        }
        this_t& setPiece(Piece p)noexcept{
            dat_ |= static_cast<data_t>(p) << MELD_INDEX_PIECE;
            return *this;
        }
        this_t& setExtPiece(ExtPiece ep)noexcept{
            dat_ |= static_cast<data_t>(ep) << MELD_INDEX_PIECE;
            return *this;
        }
        this_t& setRed()noexcept{
            dat_ |= MELD_RED;
            return *this;
        }
        this_t& clear()noexcept{
            dat_ = 0;
            return *this;
        }
        
        template<bool kCanBeNull = true>
        ExtPieceSet toConsumed()const{
            Piece p = piece();
            ExtPieceSet eps;
            eps.clear();
            if(kCanBeNull && !any()){ return eps; }
            if(chow()){ // 順子を生成
                eps.addSeq<N_CHOW_PIECES>(p, false);
            }else{ // 刻子，槓子を生成
                if(pong()){ // ポン
                    eps.add(p, N_PONG_PIECES, false);
                }else{ // カン
                    eps.add(p, N_KONG_PIECES, false);
                }
            }
            if(red()){
                eps.flipToRed(replaceRank(p, RANK_RED));
            }
            return eps;
        }
        
        std::string toString()const{
            std::ostringstream oss;
            if(any()){
                if(chow()){
                    oss << "ch";
                }else if(pong()){
                    oss << "po";
                }else if(kong()){
                    oss << "ko";
                }
                oss << " " << toConsumed();
            }else{
                oss << "null";
            }
            return oss.str();
        }
        
        constexpr MeldInRegister():dat_(){}
        constexpr MeldInRegister(const data_t& a):dat_(a){}
    };
    
    template<typename data_t>
    std::ostream& operator<<(std::ostream& ost, const MeldInRegister<data_t>& m){
        ost << m.toString();
        return ost;
    }
    
    using Meld = MeldInRegister<uint32_t>;
    
    constexpr uint32_t MELD_NONE = (uint32_t)-1;
    
    // 牌集合から役情報を作成(通信による情報受け取り時用なので速度関係なし)
    Meld toSeqMeld(const ExtPieceSet& eps){
        for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){ // 数牌のみ
            TypePieces tp = eps.naive(pt);
            if(tp.any()){
                Rank r = tp.minRank();
                Meld m;
                m.clear().setSeq().setQty(tp.sum()).setPiece(toPiece(pt, r));
                if(eps.red()){
                    m.setRed();
                }
                return m;
            }
        }
        return MELD_NONE;
    }
    Meld toGroupMeld(const ExtPieceSet& eps){
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            TypePieces tp = eps.naive(pt);
            if(tp.any()){
                Rank r = tp.minRank();
                Meld m;
                m.clear().setGroup().setQty(tp.sum()).setPiece(toPiece(pt, r));
                if(eps.red()){
                    m.setRed();
                }
                return m;
            }
        }
        return MELD_NONE;
    }
    // 基本情報から役情報を生成
    Meld toSeqMeld(Piece p, int qty, bool red)noexcept{
        Meld m;
        m.clear().setSeq().setPiece(p).setQty(qty);
        if(red){ m.setRed(); }
        return m;
    }
    Meld toGroupMeld(Piece p, int qty, bool red)noexcept{
        Meld m;
        m.clear().setGroup().setPiece(p).setQty(qty);
        if(red){ m.setRed(); }
        return m;
    }
    
    /**************************役のハッシュキー**************************/
    
    constexpr uint64_t OPENED_HASH_NULL = 0;
}

#endif // MAHJONG_STRUCTURE_MELD_HPP_