/*
 action.hpp
 Katsuki Ohto
 */

// 役と行動の表現

#ifndef MAHJONG_STRUCTURE_ACTION_HPP_
#define MAHJONG_STRUCTURE_ACTION_HPP_

#include "piece.hpp"
#include "meld.hpp"

namespace Mahjong{
    
    /**************************手番での行動**************************/
    
    using TurnActionDataType = uint32_t;
    
    // 自分のツモ手番での行動
    enum TurnActionIndex{
        TURN_ACTION_INDEX_FINISH         = 0, // ツモあがり
        TURN_ACTION_INDEX_REACH          = 1, // リーチ
        TURN_ACTION_INDEX_PARROT         = 2, // ツモ切り(parrot)
        TURN_ACTION_INDEX_ADD_KONG       = 3, // 小明槓(ツモをポンに付け加え)
        TURN_ACTION_INDEX_DRAW_KONG      = 4, // 暗槓(ツモによるカン)
        TURN_ACTION_INDEX_DISCARDED      = 8, // 打牌 7ビット
        TURN_ACTION_INDEX_MINIMUM_STEPS  = 16, // シャンテン数(あがりやリーチ 可 or 不可の判定に使う) 4ビット
        TURN_ACTION_INDEX_PIECE          = 24, // カンの牌
        TURN_ACTION_INDEX_RED            = TURN_ACTION_INDEX_PIECE + 6, // 槓が赤牌を含むか(カンの牌の赤フラグと共用)
    };
    
    enum TurnActionBaseMask : TurnActionDataType{
        TURN_ACTION_BASE_DISCARDED_NAIVE = (1 << 6) - 1,
        TURN_ACTION_BASE_DISCARDED       = (1 << (6 + 1)) - 1,
        TURN_ACTION_BASE_MINIMUM_STEPS   = (1 << 4) - 1,
        TURN_ACTION_BASE_PIECE           = (1 << 6) - 1,
        TURN_ACTION_BASE_EXT_PIECE       = (1 << (6 + 1)) - 1,
    };
    
    enum TurnActionFlag : TurnActionDataType{
        TURN_ACTION_FINISH         = 1 << TURN_ACTION_INDEX_FINISH,
        TURN_ACTION_REACH          = 1 << TURN_ACTION_INDEX_REACH,
        TURN_ACTION_PARROT         = 1 << TURN_ACTION_INDEX_PARROT,
        TURN_ACTION_ADD_KONG       = 1 << TURN_ACTION_INDEX_ADD_KONG,
        TURN_ACTION_DRAW_KONG      = 1 << TURN_ACTION_INDEX_DRAW_KONG,
        TURN_ACTION_RED            = 1 << TURN_ACTION_INDEX_RED,
        TURN_ACTION_DISCARDED      = TURN_ACTION_BASE_DISCARDED << TURN_ACTION_INDEX_DISCARDED,
        TURN_ACTION_MINIMUM_STEPS  = TURN_ACTION_BASE_MINIMUM_STEPS << TURN_ACTION_INDEX_MINIMUM_STEPS,
        
        TURN_ACTION_KONG           = TURN_ACTION_ADD_KONG | TURN_ACTION_DRAW_KONG,
        TURN_ACTION_NOT_DISCARD    = TURN_ACTION_FINISH | TURN_ACTION_KONG, // 打牌でない
    };
    
    constexpr int N_MAX_TURN_ACTIONS = 64;
    
    struct TurnAction{
        TurnActionDataType dat_;
        
        TurnActionDataType data()const noexcept{
            return dat_;
        }
        TurnActionDataType finish()const noexcept{
            return dat_ & TURN_ACTION_FINISH;
        }
        TurnActionDataType reach()const noexcept{
            return dat_ & TURN_ACTION_REACH;
        }
        TurnActionDataType parrot()const noexcept{
            return dat_ & TURN_ACTION_PARROT;
        }
        bool discard()const noexcept{
            return !(dat_ & TURN_ACTION_NOT_DISCARD);
        }
        ExtPiece discarded()const noexcept{
            return static_cast<ExtPiece>((dat_ >> TURN_ACTION_INDEX_DISCARDED)
                                         & TURN_ACTION_BASE_DISCARDED);
        }
        Piece discardedNaive()const noexcept{
            return static_cast<Piece>((dat_ >> TURN_ACTION_INDEX_DISCARDED)
                                      & TURN_ACTION_BASE_DISCARDED_NAIVE);
        }
        Piece piece()const noexcept{
            return static_cast<Piece>((dat_ >> TURN_ACTION_INDEX_PIECE)
                                      & TURN_ACTION_BASE_PIECE);
        }
        ExtPiece extPiece()const noexcept{
            return static_cast<ExtPiece>((dat_ >> TURN_ACTION_INDEX_PIECE)
                                         & TURN_ACTION_BASE_EXT_PIECE);
        }
        TurnActionDataType minimumSteps()const noexcept{
            return (dat_ >> TURN_ACTION_INDEX_MINIMUM_STEPS)
            & TURN_ACTION_BASE_MINIMUM_STEPS;
        }
        TurnActionDataType drawKong()const noexcept{
            return dat_ & TURN_ACTION_DRAW_KONG;
        }
        TurnActionDataType addKong()const noexcept{
            return dat_ & TURN_ACTION_ADD_KONG;
        }
        TurnActionDataType red()const noexcept{
            return dat_ & TURN_ACTION_RED;
        }
        
        TurnAction& setFinish()noexcept{
            dat_ = TURN_ACTION_FINISH; // 他のフラグが付くことはない
            return *this;
        }
        TurnAction& setReach()noexcept{
            dat_ |= TURN_ACTION_REACH;
            return *this;
        }
        TurnAction& setParrot()noexcept{
            dat_ |= TURN_ACTION_PARROT;
            return *this;
        }
        TurnAction& setDiscarded(ExtPiece ep)noexcept{
            dat_ |= static_cast<TurnActionDataType>(ep) << TURN_ACTION_INDEX_DISCARDED;
            return *this;
        }
        TurnAction& setExtPiece(ExtPiece ep)noexcept{
            dat_ |= static_cast<TurnActionDataType>(ep) << TURN_ACTION_INDEX_PIECE;
            return *this;
        }
        TurnAction& setPiece(Piece p)noexcept{
            dat_ |= static_cast<TurnActionDataType>(p) << TURN_ACTION_INDEX_PIECE;
            return *this;
        }
        TurnAction& setMinimumSteps(TurnActionDataType ams)noexcept{
            dat_ |= ams << TURN_ACTION_INDEX_MINIMUM_STEPS;
            return *this;
        }
        TurnAction& setDrawKong()noexcept{
            dat_ |= TURN_ACTION_DRAW_KONG;
            return *this;
        }
        TurnAction& setAddKong()noexcept{
            dat_ |= TURN_ACTION_ADD_KONG;
            return *this;
        }
        TurnAction& setRed()noexcept{
            dat_ |= TURN_ACTION_RED;
            return *this;
        }
        
        TurnAction& clear()noexcept{
            dat_ = 0;
            return *this;
        }
        
        Meld toKongMeld()const noexcept{
            Meld meld;
            meld.clear().setKong(extPiece());
            return meld;
        }
        template<bool kCanBeNotDrawKong = true>
        ExtPieceSet toKongConsumed()const{ // 暗槓限定
            Piece p = piece();
            ExtPieceSet eps;
            eps.clear();
            if(kCanBeNotDrawKong && !drawKong()){ return eps; }
            eps.add(p, N_KONG_PIECES);
            if(red()){
                eps.flipToRed(replaceRank(p, RANK_RED));
            }
            return eps;
        }
        
        std::string toString()const{
            std::ostringstream oss;
            if(finish()){ // ツモあがり
                oss << "DWIN";
            }else{
                oss << discarded();
                if(parrot() || reach()){
                    oss << "-";
                    if(parrot()){
                        oss << "p";
                    }
                    if(reach()){ // リーチ
                        oss << "R";
                    }
                }
            }
            //oss << "(" << minimumSteps() << ")" << endl;
            return oss.str();
        }
    };
    
    std::ostream& operator<<(std::ostream& ost, const TurnAction& ta){
        ost << ta.toString();
        return ost;
    }
    
    template<class action_t, class field_t>
    int genTurnActions(action_t *const pac0, Player turnPlayer, field_t& field, ExtPiece drawn){
        // 行動生成して最低限の情報を付加する
        // 基本的に使わなくなればいいな
        action_t *pac = pac0;
        
        auto& hand = field.hand[turnPlayer];
        const bool reachable = field.isReachable(turnPlayer);
        PieceExistance acceptable;
        
        if(field.isInReach(turnPlayer)){
            if(hand.isAcceptable(toPiece(drawn))){
                pac->setFinish(); ++pac;
            }else{
                pac->clear().setDiscarded(drawn); ++pac;
            }
        }else{
            // 受け入れならツモあがりを生成
            if(hand.isAcceptable(toPiece(drawn))){
                //pac->setFinish(); ++pac;
            }
            
            // 暗槓を生成 最初から持っている場合も検討必須
            /*iteratePieceWithQty(hand.piece, [&pac](Piece p, int n)->void{
                if(n == N_KONG_PIECES){
                    pac->clear().setDrawKong().setPiece(p);
                    if(isRedRankPiece(p)){
                        pac->setRed();
                    }
                    ++pac;
                }
            });
            // 小明槓を生成 最初から持っている場合も検討必須
            iterateExtPieceWithQty(hand.piece, [&pac, &hand](ExtPiece ep, int n)->void{
                if(hand.hasOpenedGroup(toPiece(ep))){ // ポンしている
                    pac->clear().setAddKong().setExtPiece(ep);
                    ++pac;
                }
            });*/
            
            // 打牌を生成
            ExtPieceSet eps = hand.piece;
            iterateExtPieceWithQty
            (eps, [&pac, &hand, reachable](ExtPiece ep, int n)->void{
                pac->clear().setDiscarded(ep);
                hand.subAll(ep);
                if((hand.minimumSteps == 0) && reachable){ // リーチを生成
                    ++pac;
                    pac->clear().setDiscarded(ep).setReach();
                }
                hand.addAll(ep);
                ++pac;
             });
        }
        return pac - pac0;
    }
    
    /**************************他プレーヤーの打牌後の行動**************************/
    
    enum ResponseActionIndex{
        // Meld 型と被らない位置に配置
        RESPONSE_ACTION_INDEX_FINISH = 15, // ロンあがり
        RESPONSE_ACTION_INDEX_DISCARDED = 16, // 打牌(カン以外)
    };
    
    enum ResponseActionBaseMask : MeldDataType{
        RESPONSE_ACTION_BASE_DISCARDED = (1 << 7) - 1,
    };
    
    enum ResponseActionFlag : MeldDataType{
        RESPONSE_ACTION_FINISH = 1 << RESPONSE_ACTION_INDEX_FINISH,
    };
    
    constexpr int N_MAX_RESPONSE_ACTIONS = 128;
    
    struct ResponseAction : public Meld{
        
        MeldDataType finish()const noexcept{
            return Meld::dat_ & RESPONSE_ACTION_FINISH;
        }
        MeldDataType responseKong()const noexcept{
            return kong();
        }
        ExtPiece discarded()const noexcept{
            return static_cast<ExtPiece>((dat_ >> RESPONSE_ACTION_INDEX_DISCARDED)
                                         & RESPONSE_ACTION_BASE_DISCARDED);
        }
        
        ResponseAction& setResponseKong(Piece p)noexcept{
            Meld::setKong(p);
            return *this;
        }
        ResponseAction& setPong(Piece p)noexcept{
            Meld::setPong(p);
            return *this;
        }
        ResponseAction& setChow(Piece p)noexcept{
            Meld::setChow();
            return *this;
        }
        ResponseAction& setDiscarded(ExtPiece ep)noexcept{
            Meld::dat_ |= ep << RESPONSE_ACTION_INDEX_DISCARDED;
            return *this;
        }
        
        ResponseAction& clear()noexcept{
            dat_ = 0;
            return *this;
        }
        
        std::string toString()const{
            std::ostringstream oss;
            if(any()){
                if(finish()){
                    oss << "RWIN";
                }else{
                    oss << Meld::toString();
                    if(chow() || pong()){
                        oss << " dis - " << discarded();
                    }
                }
            }else{
                oss << "pass";
            }
            return oss.str();
        }
    };
    
    std::ostream& operator<<(std::ostream& ost, const ResponseAction& ra){
        ost << ra.toString();
        return ost;
    }
    
    bool isPreferred(const ResponseAction& a, const ResponseAction& b){
        if(!b.any()){
            return true;
        }
        if(a.group()){
            return true;
        }
        return false; // チー、ポン、カンどれも被らない
    }
    
    template<class action_t, class field_t>
    int genResponseActions(action_t *const pac0, field_t& field, ExtPiece discarded, Player pn){
        
        assert(pn != field.turnPlayer);
        
        // この時点で捨て牌は加算されていない
        action_t *pac = pac0;
        
        auto& hand = field.hand[pn];
        const Piece p = toPiece(discarded);
        const PieceType pt = toPieceType(p);
        const Rank r = toRank(p);
        const ExtPieceSet& orgEps = hand.piece;
        ExtPieceSet eps = orgEps;
        
        // パスを生成
        pac->clear(); ++pac;
        
        // 上がれる場合は上がりを生成
        
        if(!field.isInReach(pn)){
            // 大明槓を生成
            /*if(hand[p] == N_KONG_PIECES - 1){
             pac->clear().setResponseKong(p);
             if(isRedRankPiece(p)){
             // 赤牌含みの牌ならカンで赤確定
             // それ以外では赤にはならない
             pac->setRed();
             }
             ++pac;
             }*/
            // ポンを生成
            if(hand[p] >= N_PONG_PIECES - 1){
                // 捨て牌を生成
                /*ps.sub(p, N_PONG_PIECES - 1);
                 if(isRed(discarded) && orgPs.red(p) && !ps.contains(p)){ // 必ず赤牌を使う
                 iterateExtPieceWithQty(ps, [p, &pac](ExtPiece ep, int n)->void{
                 pac->clear().setPong(p).setDiscarded(ep).setRed(); ++pac;
                 });
                 }else{ // 赤牌を使わなくてよい
                 iterateExtPieceWithQty(ps, [p, &pac](ExtPiece ep, int n)->void{
                 pac->clear().setPong(p).setDiscarded(ep); ++pac;
                 });
                 }
                 ps.add(p, N_PONG_PIECES - 1);*/
            }
            
            // チーを生成(手番限定)
            // 存在階段型とのandを取って抽出
            if(nextPlayer(field.turnPlayer) == pn){
                // 自分の前のプレーヤーの打牌のみチー可能
                /*const BitSet64 existance = hand.existance;
                 if(BitSet64(existance & (existance >> 1)).test(p - 2)){
                 ps.subSeqExcept<N_CHOW_PIECES>(p - 2, p);
                 if(isRedRankSeqExcept)
                 iterateExtPieceWithQty(ps, [p, &pac](ExtPiece ep, int n)->void{
                 pac->clear().setChow(p - 2).setDiscarded(ep); ++pac;
                 });
                 ps.addSeqExcept<N_CHOW_PIECES>(p - 2, p);
                 }
                 if(BitSet64(existance & (existance >> 2)).test(p - 1)){
                 ps.subSeqExcept<N_CHOW_PIECES>(p - 1, p);
                 iterateExtPieceWithQty(ps, [p, &pac](ExtPiece ep, int n)->void{
                 pac->clear().setChow(p - 1).setDiscarded(ep); ++pac;
                 });
                 ps.addSeqExcept<N_CHOW_PIECES>(p - 1, p);
                 }
                 if(BitSet64((existance >> 1) & (existance >> 2)).test(p)){
                 ps.subSeqExcept<N_CHOW_PIECES>(p, p);
                 iterateExtPieceWithQty(ps, [p, &pac](ExtPiece ep, int n)->void{
                 pac->clear().setChow(p).setDiscarded(ep); ++pac;
                 });
                 ps.addSeqExcept<N_CHOW_PIECES>(p, p);
                 }*/
            }
        }
        
        return pac - pac0;
    }
    
    /**************************他プレーヤーの加える槓後の行動**************************/
    
    struct AddedAction{
        bool a;
        bool any()const noexcept{ return a; }
        void clear()noexcept{ a = false; }
        bool finish()const noexcept{ return a; }
    };
    
    constexpr int N_MAX_ADDED_ACTIONS = 2;
    
    template<class action_t, class field_t>
    int genAddedActions(action_t *const pac0, field_t& field){
        pac0->clear();
        return 1;
    }
}

#endif // MAHJONG_STRUCTURE_ACTION_HPP_