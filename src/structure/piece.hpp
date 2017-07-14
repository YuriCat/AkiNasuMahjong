/*
 piece.hpp
 Katsuki Ohto
 */

// 牌と牌集合の表現

#ifndef MAHJONG_STRUCTURE_PIECE_HPP_
#define MAHJONG_STRUCTURE_PIECE_HPP_

#include "base.hpp"

namespace Mahjong{
    
    /**************************牌**************************/
    
    enum Piece : uint8_t{
        // 牌
        // 他のデータ構造との対応を楽にするため 16区切りとしてpaddingする
        M1 =  0, M2, M3, M4, M5, M6, M7, M8, M9, // characters
        P1 = 16, P2, P3, P4, P5, P6, P7, P8, P9, // circles
        S1 = 32, S2, S3, S4, S5, S6, S7, S8, S9, // bamboos
        E  = 48, S,  W,  N,  P,  F,  C, // honors
        PIECE_NONE = uint8_t(-1),
        PIECE_MIN = M1,
        PIECE_MAX = C,
        N_PIECES = N_RANKS * int(N_NUMBER_PIECE_TYPES) + N_HONORS, // MAX + 1でないので注意!
        PIECE_NUMBERS_MIN = M1,
        PIECE_NUMBERS_MAX = S9,
        PIECE_HONOR_MIN = E,
        PIECE_HONOR_MAX = C,
        PIECE_WIND_MIN = E,
        PIECE_WIND_MAX = N,
        PIECE_DRAGON_MIN = P,
        PIECE_DRAGON_MAX = C,
        PIECE_MASK_RANK = (1U << 4) - 1,
    };
    
    ENABLE_ARITHMETIC_OPERATORS(Piece)
    
    const std::string numberChar = "mps";
    
    constexpr int N_MAX_TYPE_PIECES = (N_RANKS > N_HONORS) ? (int)N_RANKS : N_HONORS;
    
    constexpr bool isHonor(Piece p)noexcept{
        return p >= PIECE_HONOR_MIN;
    }
    
    constexpr PieceType toPieceType(Piece p)noexcept{
        return static_cast<PieceType>(p / 16);
    }
    constexpr uint8_t toPieceTypePart(Piece p)noexcept{
        return p & ~uint8_t(15U); // 同一性等比較のとき用
    }
    constexpr Rank toRank(Piece p)noexcept{
        return static_cast<Rank>(p % 16);
    }
    constexpr Piece toPiece(PieceType pt, Rank r)noexcept{
        return static_cast<Piece>(pt * 16) + static_cast<Piece>(r);
    }
    // padding を入れたことでランク、タイプを入れ替えられるようになった
    constexpr Piece replaceRank(Piece p, Rank r)noexcept{
        return static_cast<Piece>((p & ~PIECE_MASK_RANK) | r);
    }
    constexpr Piece replacePieceType(Piece p, PieceType pt)noexcept{
        return static_cast<Piece>((p & PIECE_MASK_RANK) | pt * 16);
    }
    
    constexpr bool examPiece(Piece p)noexcept{
        return examPieceType(toPieceType(p))
        && (isNumberType(toPieceType(p)) ?
            examRank(toRank(p)) : examHonorRank(toRank(p)));
    }
    
    constexpr Piece toWindPiece(Wind w)noexcept{
        return static_cast<Piece>(w + (int)PIECE_WIND_MIN);
    }
    constexpr Piece toDragonPiece(Dragon d)noexcept{
        return static_cast<Piece>(d + (int)PIECE_DRAGON_MIN);
    }
    constexpr Wind toWind(Piece p)noexcept{
        return static_cast<Wind>(p - PIECE_WIND_MIN);
    }
    constexpr Dragon toDragon(Piece p)noexcept{
        return static_cast<Dragon>(p - PIECE_DRAGON_MIN);
    }
    constexpr bool isNumberPiece(Piece p)noexcept{
        return ((unsigned)p < PIECE_HONOR_MIN);
    }
    constexpr bool isWindPiece(Piece p)noexcept{
        return (PIECE_WIND_MIN <= p && p <= PIECE_WIND_MAX);
    }
    constexpr bool isDragonPiece(Piece p)noexcept{
        return (PIECE_DRAGON_MIN <= p && p <= PIECE_DRAGON_MAX);
    }
    constexpr bool isRedRankPiece(Piece p)noexcept{
        return isNumberPiece(p) && (toRank(p) == RANK_RED);
    }
    
    // ドラのための次の牌の計算
    constexpr Piece toNextNumberPiece(Piece p)noexcept{
        return toPiece(toPieceType(p), toNextRank(toRank(p)));
    }
    constexpr Piece toNextWindPiece(Piece p)noexcept{
        return toWindPiece(toNextWind(toWind(p)));
    }
    constexpr Piece toNextDragonPiece(Piece p)noexcept{
        return toDragonPiece(toNextDragon(toDragon(p)));
    }
    constexpr Piece toNextPiece(Piece p)noexcept{
        return isNumberType(toPieceType(p)) ? toNextNumberPiece(p)
        : (isWindPiece(p) ? toNextWindPiece(p) : toNextDragonPiece(p));
    }
    
    // 牌の出力
    const std::string pieceTypeChar = "mpsh";
    constexpr char pieceTypeCharNone = '/';
    constexpr char redPieceChar = 'r';
    
    char toPieceTypeChar(PieceType pt){
        return static_cast<unsigned int>(pt) < pieceTypeChar.size() ? pieceTypeChar[pt] : pieceTypeCharNone;
    }
    char toHonorChar(Piece p){
        if(isWindPiece(p)){
            return toWindChar(toWind(p));
        }else if(isDragonPiece(p)){
            return toDragonChar(toDragon(p));
        }
        return pieceTypeCharNone;
    }
    
    std::string toString(Piece p){
        std::ostringstream oss;
        if(examPiece(p)){
            if(isWindPiece(p)){
                oss << toWindChar(toWind(p));
            }else if(isDragonPiece(p)){
                oss << toDragonChar(toDragon(p));
            }else{
                oss << toRankChar(toRank(p)) << toPieceTypeChar(toPieceType(p));
            }
        }else{
            oss << "-";
        }
        return oss.str();
    }
    
    std::ostream& operator<<(std::ostream& ost, const Piece& p){
        ost << toString(p);
        return ost;
    }
    
    // 全ての牌番号のループ
    template<class callback_t>
    void iterateNaiveNumber(const callback_t& callback)noexcept{
        for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
            for(Rank r = RANK_MIN; r <= RANK_MAX; ++r){
                callback(toPiece(pt, r));
            }
        }
    }
    
    template<class callback_t>
    void iterateHonor(const callback_t& callback)noexcept{
        for(Rank r = RANK_HONOR_MIN; r <= RANK_HONOR_MAX; ++r){
            callback(toPiece(HONOR, r));
        }
    }
    
    template<class callback_t>
    void iterateNaivePiece(const callback_t& callback)noexcept{
        iterateNaiveNumber(callback);
        iterateHonor(callback);
    }
    
    /**************************牌番号(パディングなし)**************************/
    
    // 全ての牌のループや一様選択を手軽に行うための牌番号
    
    enum PieceIndex{
        PIECE_INDEX_MIN = 0,
        PIECE_INDEX_MAX = (int)N_PIECES - 1,
    };
    
    ENABLE_ARITHMETIC_OPERATORS(PieceIndex)
    
    constexpr PieceType toPieceType(PieceIndex pi)noexcept{
        return static_cast<PieceType>(pi / N_MAX_TYPE_PIECES);
    }
    constexpr Rank toRank(PieceIndex pi)noexcept{
        return static_cast<Rank>(pi % N_MAX_TYPE_PIECES);
    }
    constexpr Piece toPiece(PieceIndex pi)noexcept{
        return toPiece(toPieceType(pi), toRank(pi));
    }
    
    constexpr bool examPieceIndex(PieceIndex pi)noexcept{
        return ((unsigned int)pi <= PIECE_INDEX_MAX);
    }
    
    /**************************牌(赤情報付き)**************************/
    
    enum ExtPiece : uint8_t{
        EXT_PIECE_MASK_NAIVE = (1 << 6) - 1,
        EXT_PIECE_FLAG_RED = 1 << 6,
        EXT_PIECE_NONE = uint8_t(-1),
    };
    
    ENABLE_ARITHMETIC_OPERATORS(ExtPiece)
    
    constexpr Piece toPiece(ExtPiece ep)noexcept{
        return static_cast<Piece>(ep & EXT_PIECE_MASK_NAIVE);
    }
    
    constexpr bool examExtPiece(ExtPiece ep)noexcept{
        return examPiece(toPiece(ep));
    }
    
    constexpr bool isRed(ExtPiece ep)noexcept{
        return ep & EXT_PIECE_FLAG_RED;
    }
    constexpr PieceType toPieceType(ExtPiece ep)noexcept{
        return static_cast<PieceType>(toPiece(ep) / 16);
    }
    constexpr Rank toRank(ExtPiece ep)noexcept{
        return static_cast<Rank>(ep % 16); // 赤情報は勝手に消える
    }
    constexpr ExtPiece toExtPiece(Piece p, bool isRed = false)noexcept{
        return isRed ? static_cast<ExtPiece>((static_cast<ExtPiece>(p) | EXT_PIECE_FLAG_RED))
        : static_cast<ExtPiece>(p);
    }
    constexpr ExtPiece toExtPiece(PieceType pt, Rank r, bool isRed = false)noexcept{
        return toExtPiece(toPiece(pt, r), isRed);
    }
    constexpr ExtPiece toExtPiece(ExtPieceType ept, Rank e)noexcept{
        return static_cast<ExtPiece>(ept * 16 | e);
    }
    constexpr ExtPieceType toExtPieceType(ExtPiece ep)noexcept{
        return static_cast<ExtPieceType>(ep / 16);
    }
    
    std::string toString(ExtPiece ep){
        std::ostringstream oss;
        if(examExtPiece(ep)){
            oss << toPiece(ep);
            if(isRed(ep)){
                oss << redPieceChar;
            }
        }else{
            oss << "-";
        }
        return oss.str();
    }
    
    std::ostream& operator<<(std::ostream& ost, const ExtPiece& ep){
        ost << toString(ep);
        return ost;
    }
    
    // 全ての牌番号のループ(赤牌は別処理の場合)
    template<class normalCallback_t, class redCallback_t>
    void iterateNumber(const normalCallback_t& normalCallback,
                       const redCallback_t& redCallback)noexcept{
        for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
            for(Rank r = RANK_MIN; r < RANK_RED; ++r){
                normalCallback(toPiece(pt, r));
            }
            redCallback(toPiece(pt, RANK_RED));
            for(Rank r = RANK_RED + 1; r <= RANK_MAX; ++r){
                normalCallback(toPiece(pt, r));
            }
        }
    }
    
    template<class normalCallback_t, class redCallback_t>
    void iteratePiece(const normalCallback_t& normalCallback,
                       const redCallback_t& redCallback)noexcept{
        iterateNumber(normalCallback, redCallback);
        iterateHonor(normalCallback);
    }
    
    /**************************種類ごとの牌集合に対するアルゴリズム**************************/
    
    // 牌集合の定数
    constexpr uint64_t PQR_NULL = 0ULL;
    constexpr uint64_t PQR_1    = 0x1111111111111111;
    constexpr uint64_t PQR_2    = 0x2222222222222222;
    constexpr uint64_t PQR_3    = 0x4444444444444444;
    constexpr uint64_t PQR_4    = 0x8888888888888888;
    constexpr uint64_t PQR_12   = 0x3333333333333333;
    constexpr uint64_t PQR_13   = 0x5555555555555555;
    constexpr uint64_t PQR_14   = 0x9999999999999999;
    constexpr uint64_t PQR_23   = 0x6666666666666666;
    constexpr uint64_t PQR_24   = 0xaaaaaaaaaaaaaaaa;
    constexpr uint64_t PQR_34   = 0xcccccccccccccccc;
    constexpr uint64_t PQR_123  = 0x7777777777777777;
    constexpr uint64_t PQR_124  = 0xbbbbbbbbbbbbbbbb;
    constexpr uint64_t PQR_134  = 0xdddddddddddddddd;
    constexpr uint64_t PQR_234  = 0xeeeeeeeeeeeeeeee;
    constexpr uint64_t PQR_1234 = 0xffffffffffffffff;
    
    constexpr uint64_t rankMask4(Rank r)noexcept{
        return (15ULL) << (r << 2);
    }
        
    // TypePieces定数
    constexpr uint64_t TYPE_PIECES_NULL           = 0;
    constexpr uint64_t TYPE_PIECES_NUMBERS_ALL    = ((1ULL << (N_ONE_PIECE * N_RANKS )) - 1ULL) & PQR_3; // PQR_3は4枚
    constexpr uint64_t TYPE_PIECES_HONORS_ALL     = ((1ULL << (N_ONE_PIECE * N_HONORS)) - 1ULL) & PQR_3;
    
    constexpr uint64_t TYPE_MASK_NUMBERS_ALL    = (1ULL << (N_ONE_PIECE * N_RANKS )) - 1ULL;
    constexpr uint64_t TYPE_MASK_HONORS_ALL     = (1ULL << (N_ONE_PIECE * N_HONORS)) - 1ULL;
    constexpr uint64_t TYPE_MASK_NUMBERS_EDGE   = rankMask4(RANK_MIN) | rankMask4(RANK_MAX);
    constexpr uint64_t TYPE_MASK_NUMBERS_INSIDE = TYPE_MASK_NUMBERS_ALL & ~TYPE_MASK_NUMBERS_EDGE;
    constexpr uint64_t TYPE_MASK_DRAGONS_ALL    = ((1ULL << (N_ONE_PIECE * toRank(PIECE_DRAGON_MAX) + 1)) - 1ULL) & ~((1ULL << (N_ONE_PIECE * toRank(PIECE_DRAGON_MIN))) - 1ULL);
    
    // pqr, sc定数
    constexpr uint64_t TYPE_PQR_NUMBERS_ALL = ((1ULL << (N_ONE_PIECE * N_RANKS )) - 1ULL) & PQR_4;
    constexpr uint64_t TYPE_PQR_HONORS_ALL  = ((1ULL << (N_ONE_PIECE * N_HONORS)) - 1ULL) & PQR_4;
    
    constexpr uint64_t TYPE_SC_NUMBERS_ALL = TYPE_MASK_NUMBERS_ALL & PQR_1234;
    constexpr uint64_t TYPE_SC_HONORS_ALL  = TYPE_MASK_HONORS_ALL & PQR_1234;
    
    
    uint64_t convQR_PQR(const uint64_t qr)noexcept{
        // qr -> pqr 変換
        const uint64_t iqr = ~qr;
        const uint64_t qr_l1 = (qr << 1);
        const uint64_t r = (PQR_1 & qr & (iqr >> 1)) | (PQR_2 & qr & (iqr << 1)) | ((qr & qr_l1) << 1) | (qr_l1 & PQR_4);
        return r;
    }
    uint64_t convPQR_SC(const uint64_t pqr)noexcept{
        // pqr -> sc はビットを埋めていくだけ
        uint64_t r = pqr;
        r |= (r & PQR_234) >> 1;
        r |= (r & PQR_34) >> 2;
        return r;
    }
    
    // ランク重合
    template<int N>
    inline uint64_t polymRanks(const uint64_t i){
        return ((i >> ((N - 1) << 2)) & polymRanks<cmax(N - 1, 0)>(i));
    }
    template<>inline constexpr uint64_t polymRanks<0>(const uint64_t i){ return 0; }
    
    inline constexpr uint64_t polymJump(const uint64_t i)noexcept{ // 1ランクとばし
        return i & (i >> 8);
    }
    
    // ランク展開
    template<int N>
    inline uint64_t extractRanks(const uint64_t i)noexcept{
        return (i << ((N - 1) << 2)) | extractRanks<cmax(0, N - 1)>(i);
    }
    
    template<>inline constexpr uint64_t extractRanks<0>(const uint64_t i)noexcept{ return 0; }
    template<>inline constexpr uint64_t extractRanks<1>(const uint64_t i)noexcept{ return i; }
    template<>inline constexpr uint64_t extractRanks<2>(const uint64_t i)noexcept{ return i | (i << 4); }
    template<>inline constexpr uint64_t extractRanks<3>(const uint64_t i)noexcept{ return i | (i << 4) | (i << 8); }
    template<>inline uint64_t extractRanks<4>(const uint64_t i)noexcept{
        uint64_t r = i | (i << 4);
        return r | (r << 8);
    }
    template<>inline uint64_t extractRanks<5>(const uint64_t i)noexcept{
        uint64_t r = i | (i << 4);
        return r | (i << 8) | (r << 12);
    }
    
    /**************************種類ごとの牌集合**************************/
    
    // ゲーム中に存在する牌の総数
    constexpr int N_ALL_PIECES = N_ONE_PIECE * N_PIECES;
    
    // 種類別の牌集合
    using TypePieces3 = BitArray32<3, N_MAX_TYPE_PIECES>;
    
    struct alignas(BitArray64<4, N_MAX_TYPE_PIECES>) TypePieces4 : public BitArray64<4, N_MAX_TYPE_PIECES>{
        using base_t = BitArray64<4, N_MAX_TYPE_PIECES>;
        using this_t = TypePieces4;
        
        this_t& operator +=(Rank r)noexcept{
            base_t::add(r, 1);
            return *this;
        }
        this_t& operator +=(const this_t tp)noexcept{
            base_t::operator +=(tp);
            return *this;
        }
        this_t& operator -=(Rank r)noexcept{
            base_t::subtr(r, 1);
            return *this;
        }
        this_t& operator -=(const this_t tp)noexcept{
            base_t::operator -=(tp);
            return *this;
        }
        constexpr this_t operator +(Rank r)const noexcept{
            return this_t(base_t::operator +(1ULL << base_t::place(r)));
        }
        constexpr this_t operator -(Rank r)const noexcept{
            return this_t(base_t::operator -(1ULL << base_t::place(r)));
        }
        constexpr this_t operator +(const TypePieces4 tp)const noexcept{
            return this_t(base_t::operator +(base_t(tp)));
        }
        constexpr this_t operator -(const TypePieces4 tp)const noexcept{
            return this_t(base_t::operator -(base_t(tp)));
        }
        
        // グループ型演算
        this_t& add(Rank r, int n)noexcept{
            base_t::add(r, n);
            return *this;
        }
        this_t& sub(Rank r, int n)noexcept{
            base_t::subtr(r, n);
            return *this;
        }
        // 階段型演算
        template<int N>
        this_t& addSeq(Rank r){
            base_t::operator +=(extractRanks<N>(1) << base_t::place(r));
            return *this;
        }
        template<int N>
        this_t& subSeq(Rank r){
            base_t::operator -=(extractRanks<N>(1) << base_t::place(r));
            return *this;
        }
        template<int N>
        this_t& addSeqExcept(Rank r, Rank ex){
            base_t::operator +=((extractRanks<N>(1) << base_t::place(r)) - (1ULL << base_t::place(ex)));
            return *this;
        }
        template<int N>
        this_t& subSeqExcept(Rank r, Rank ex){
            base_t::operator -=((extractRanks<N>(1) << base_t::place(r)) - (1ULL << base_t::place(ex)));
            return *this;
        }
        
        // 限定
        this_t pick(Rank r)const noexcept{
            return this_t(base_t::get_part(r));
        }
        this_t& limit(Rank r)noexcept{
            operator &=(base_t::mask(r));
            return *this;
        }
        this_t& clearRank(Rank r)noexcept{ // 1つのランク消す
            operator &=(~base_t::mask(r));
            return *this;
        }
        TypePieces4& clear()noexcept{
            base_t::clear();
            return *this;
        }
        
        Rank minRank()const{
            return static_cast<Rank>(min_index());
        }
        Rank maxRank()const{
            return static_cast<Rank>(max_index());
        }
        bool any()const noexcept{
            return bool(base_t::any());
        }
        uint64_t contains(Rank r)const noexcept{
            return base_t::get_part(r);
        }
        
        bool exam()const{
            for(int i = 0; i < (64 / 4); ++i){
                unsigned int q = (*this)[i];
                if(q > N_ONE_PIECE){ // 1つの牌の総枚数オーバー
                    cerr << "PieceTypes4::exam() : num of piece out of bounds ";
                    cerr << q << " in " << N_ONE_PIECE << " " << *this << endl;
                    return false;
                }
            }
            return true;
        }
        
        constexpr TypePieces4(): base_t(){}
        constexpr TypePieces4(const uint64_t& a): base_t(a){}
        constexpr TypePieces4(const base_t& ab): base_t(ab){}
        constexpr TypePieces4(const this_t& atp): base_t(atp){}
        constexpr TypePieces4(const Rank r):
        base_t(){
            clear();
            operator +=(r);
        }
    };
    
    template<class callback_t>
    void iterateRankWithQty(const TypePieces4& tp4, const callback_t& callback)noexcept{
        iterateOn(tp4, [&callback](unsigned int r, unsigned int q)->void{
            callback(static_cast<Rank>(r), q);
        });
    }
    template<class callback_t>
    void iteratePieceWithQty(PieceType pt, const TypePieces4& tp4, const callback_t& callback)noexcept{
        Piece p = toPiece(pt, RANK_MIN); // ベースの牌位置
        iterateOn(tp4, [p, &callback](unsigned int r, unsigned int q)->void{
            callback(p + static_cast<Piece>(r - (int)RANK_MIN), q);
        });
    }
    template<class callback_t>
    void iterateRank(const TypePieces4& tp4, const callback_t& callback)noexcept{
        iterateOnTimes(tp4, [&callback](unsigned int r)->void{
            callback(static_cast<Rank>(r));
        });
    }
    template<class callback_t>
    void iteratePiece(PieceType pt, const TypePieces4& tp4, const callback_t& callback)noexcept{
        Piece p = toPiece(pt, RANK_MIN); // ベースの牌位置
        iterateOnTimes(tp4, [p, &callback](unsigned int r)->void{
            callback(p + static_cast<Piece>(r - (int)RANK_MIN));
        });
    }
    template<class callback_t>
    int iterateRankWithCount(const TypePieces4& tp4, const callback_t& callback)noexcept{
        return iterateOnTimesWithCount(tp4, [&callback](unsigned int index, unsigned int r)->void{
            callback(index, static_cast<Rank>(r));
        });
    }
    template<class callback_t>
    int iteratePieceWithCount(PieceType pt, const TypePieces4& tp4, const callback_t& callback)noexcept{
        Piece p = toPiece(pt, RANK_MIN); // ベースの牌位置
        return iterateOnTimesWithCount(tp4, [p, &callback](unsigned int index, unsigned int r)->void{
            callback(index, p + static_cast<Piece>(r - (int)RANK_MIN));
        });
    }
    
    using TypePieces = TypePieces4;
    
    /**************************牌集合**************************/
    
    // 赤牌を特別視しない牌集合
    using PieceSet3 = std::array<TypePieces3, N_PIECE_TYPES>;
    
    // 256ビット定数
    const bits256_t PQR256_NULL = bits256_t::filled8(0x00);
    const bits256_t PQR256_1    = bits256_t::filled8(0x11);
    const bits256_t PQR256_2    = bits256_t::filled8(0x22);
    const bits256_t PQR256_3    = bits256_t::filled8(0x44);
    const bits256_t PQR256_4    = bits256_t::filled8(0x88);
    const bits256_t PQR256_12   = bits256_t::filled8(0x33);
    const bits256_t PQR256_13   = bits256_t::filled8(0x55);
    const bits256_t PQR256_14   = bits256_t::filled8(0x99);
    const bits256_t PQR256_23   = bits256_t::filled8(0x66);
    const bits256_t PQR256_24   = bits256_t::filled8(0xaa);
    const bits256_t PQR256_34   = bits256_t::filled8(0xcc);
    const bits256_t PQR256_123  = bits256_t::filled8(0x77);
    const bits256_t PQR256_124  = bits256_t::filled8(0xbb);
    const bits256_t PQR256_134  = bits256_t::filled8(0xdd);
    const bits256_t PQR256_234  = bits256_t::filled8(0xee);
    const bits256_t PQR256_1234 = bits256_t::filled8(0xff);
    
    const bits256_t B4_MASK   = bits256_t::filled8(0x0f);
    const bits256_t B8_MASK   = bits256_t::filled16(0x00ff);
    const bits256_t B16_MASK  = bits256_t::filled32(0x0000ffff);
    const bits256_t B32_MASK  = bits256_t::filled64(0x00000000ffffffff);
    const bits256_t B64_MASK  = bits256_t::packed64(-1LL, 0, -1LL, 0);
    const bits256_t B128_MASK = bits256_t::packed64(-1LL, -1LL, 0, 0);
    
    const bits256_t PIECE_SET_NULL = bits256_t::zero();
    const bits256_t PIECE_SET_ALL  = bits256_t::packed64(TYPE_PIECES_NUMBERS_ALL,
                                                         TYPE_PIECES_NUMBERS_ALL,
                                                         TYPE_PIECES_NUMBERS_ALL,
                                                         TYPE_PIECES_HONORS_ALL);
    
    const bits256_t NAIVE_MASK_ALL = bits256_t::packed64(TYPE_MASK_NUMBERS_ALL,
                                                         TYPE_MASK_NUMBERS_ALL,
                                                         TYPE_MASK_NUMBERS_ALL,
                                                         TYPE_MASK_HONORS_ALL);
    
    const bits256_t PIECE_SET_DOUBLE = NAIVE_MASK_ALL | PQR256_2; // 七対子
    
    const bits256_t NAIVE_MASK_9T9P = bits256_t::packed64(TYPE_MASK_NUMBERS_EDGE,
                                                          TYPE_MASK_NUMBERS_EDGE,
                                                          TYPE_MASK_NUMBERS_EDGE,
                                                          TYPE_MASK_HONORS_ALL); // 九種九牌
    const bits256_t NAIVE_MASK_INSIDE_NUMBERS = bits256_t::packed64(TYPE_MASK_NUMBERS_INSIDE,
                                                                    TYPE_MASK_NUMBERS_INSIDE,
                                                                    TYPE_MASK_NUMBERS_INSIDE,
                                                                    0); // 断么
    
    
    const bits256_t SET_PQR_ALL = bits256_t::packed64(TYPE_PQR_NUMBERS_ALL,
                                                      TYPE_PQR_NUMBERS_ALL,
                                                      TYPE_PQR_NUMBERS_ALL,
                                                      TYPE_PQR_HONORS_ALL);
    
    const bits256_t SET_SC_ALL = bits256_t::packed64(TYPE_SC_NUMBERS_ALL,
                                                     TYPE_SC_NUMBERS_ALL,
                                                     TYPE_SC_NUMBERS_ALL,
                                                     TYPE_SC_HONORS_ALL);
    
    const bits256_t SET_SC_FULL_ORPHANS = NAIVE_MASK_9T9P & PQR256_1;
    
    
    using Pqr = Pack256<TypePieces4, N_PIECE_TYPES>;
    using Sc = Pack256<TypePieces4, N_PIECE_TYPES>;
    
    struct alignas(bits256_t) PieceSet4 : public bits256_t{
        using base_t = bits256_t;
        using each_type_t = TypePieces4;
        using this_t = PieceSet4;
        
        void assertBeforeAdd(Piece p, int n = 1)const{
            ASSERT(0 < n && n + operator [](p) <= N_ONE_PIECE,
                   cerr << n << " on " << operator [](p) << " (piece " << p << ")" << endl;);
        }
        void assertBeforeSub(Piece p, int n = 1)const{
            ASSERT(0 < n && operator [](p) >= n,
                   cerr << n << " by " << operator [](p) << " (piece " << p << ")" << endl;);
        }
        static bits256_t mask(Piece p){
            bits256_t b;
            b.clear();
            reinterpret_cast<each_type_t&>(b[toPieceType(p)]).set(toRank(p), 15);
            return b;
        }
        
        bits256_t long_data()const noexcept{
            return base_t(*this);
        }
        operator bits256_t()const noexcept{
            return long_data();
        }
        std::array<each_type_t, N_PIECE_TYPES> array()const noexcept{
            std::array<each_type_t, N_PIECE_TYPES> ar;
            for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
                ar[pt] = operator [](pt);
            }
            return ar;
        }
        
        int operator [](Piece p)const{
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            return (operator [](toPieceType(p)))[toRank(p)];
        }
        
        each_type_t& operator [](PieceType pt){
            ASSERT(examPieceType(pt), cerr << (int)pt << endl;);
            return reinterpret_cast<each_type_t&>(base_t::operator [](pt));
        }
        
        each_type_t operator [](PieceType pt)const{
            ASSERT(examPieceType(pt), cerr << (int)pt << endl;);
            return static_cast<each_type_t>(base_t::operator [](pt));
        }
        
        this_t& operator +=(Piece p){
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            operator [](toPieceType(p)) += toRank(p);
            return *this;
        }
        this_t& operator +=(const this_t& ps){
            base_t::operator +=(ps);
            return *this;
        }
        this_t& operator -=(Piece p){
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            operator [](toPieceType(p)) -= toRank(p);
            return *this;
        }
        this_t& operator -=(const this_t& ps){
            base_t::operator -=(ps);
            return *this;
        }
        
        this_t operator +(Piece p)const{
            this_t tmp = *this;
            tmp += p;
            return tmp;
        }
        this_t operator +(const this_t& ps)const noexcept{
            return this_t(base_t::operator +(ps));
        }
        this_t operator -(Piece p)const{
            this_t tmp = *this;
            tmp -= p;
            return tmp;
        }
        this_t operator -(const this_t& ps)const noexcept{
            return this_t(base_t::operator -(ps));
        }
        
        // グループ系演算
        this_t& add(Piece p, int n){
            assertBeforeAdd(p, n);
            operator[] (toPieceType(p)).add(toRank(p), n);
            return *this;
        }
        this_t& sub(Piece p, int n){
            assertBeforeSub(p, n);
            operator[] (toPieceType(p)).sub(toRank(p), n);
            return *this;
        }
        // 階段型演算
        template<int N = N_CHOW_PIECES>
        this_t& addSeq(Piece p){
            operator[] (toPieceType(p)).addSeq<N>(toRank(p));
            return *this;
        }
        template<int N = N_CHOW_PIECES>
        this_t& subSeq(Piece p){
            operator[] (toPieceType(p)).subSeq<N>(toRank(p));
            return *this;
        }
        template<int N = N_CHOW_PIECES>
        this_t& addSeqExcept(Piece p, Piece ex){
            operator[] (toPieceType(p)).addSeqExcept<N>(toRank(p), toRank(ex));
            return *this;
        }
        template<int N = N_CHOW_PIECES>
        this_t& subSeqExcept(Piece p, Piece ex){
            operator[] (toPieceType(p)).subSeqExcept<N>(toRank(p), toRank(ex));
            return *this;
        }
        
        // 限定
        this_t pick(Piece p)const{
            return operator &(this_t::mask(p));
        }
        this_t& limit(Piece p){
            for(PieceType pt = PIECE_TYPE_MIN; pt < toPieceType(p); ++pt){
                operator [](pt).clear();
            }
            operator [](toPieceType(p)).limit(toRank(p));
            for(PieceType pt = toPieceType(p) + 1; pt <= PIECE_TYPE_MAX; ++pt){
                 operator [](pt).clear();
            }
            return *this;
        }
        this_t& clearPiece(Piece p){
            operator [](toPieceType(p)).clearRank(toRank(p));
            return *this;
        }
        
        this_t operator ~()const noexcept{
#ifdef HAVE_AVX2
            return this_t(PIECE_SET_ALL - bits256_t(*this));
#else
            this_t ans;
            for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
                ans[pt] = TYPE_PIECES_NUMBERS_ALL - (operator [](pt));
            }
            ans[HONOR] = TYPE_PIECES_HONORS_ALL - (operator [](HONOR));
            return ans;
#endif
        }
        
        uint32_t sum()const noexcept{
            // 全枚数合計
            return long_data().sum4();
        }
        base_t typeSum()const noexcept{
            // 種類ごとの枚数合計
            return base_t(long_data().sum4_per64());
        }
        
        auto contains(Piece p)const noexcept{
            return operator [](toPieceType(p)).contains(toRank(p));
        }
        this_t& fill()noexcept{
#ifdef HAVE_AVX2
            (*this) = PIECE_SET_ALL;
#else
            for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
                operator [](pt) = TYPE_PIECES_NUMBERS_ALL;
            }
            operator [](HONOR) = TYPE_PIECES_HONORS_ALL;
#endif
            
            return *this;
        }
        this_t& fillPqr()noexcept{
            for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
                operator [](pt) = TYPE_PQR_NUMBERS_ALL;
            }
            operator [](HONOR) = TYPE_PQR_HONORS_ALL;
            return *this;
        }
        this_t& fillSc()noexcept{
            for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
                operator [](pt) = TYPE_SC_NUMBERS_ALL;
            }
            operator [](HONOR) = TYPE_SC_HONORS_ALL;
            return *this;
        }
        
        bool exam()const{
            for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
                if(!operator [](pt).exam()){
                    cerr << "PieceSet4::exam() : " << "illegal type-pieces " << pt << endl;
                    return false;
                }
            }
            return true;
        }
        
        PieceSet4(): base_t(){}
        
        PieceSet4(Piece p): base_t(){
            clear();
            operator [](toPieceType(p)).add(toRank(p), 1);
        }
        PieceSet4(const bits256_t& b): base_t(b){}
        //PieceSet4(const std::array<TypePieces4, N_PIECE_TYPES>& ar)
        //:base_t(ar){}
        //PieceSet4(const base_t& base): base_t(base){}
    };
    
    std::ostream& operator<<(std::ostream& ost, const PieceSet4& ps){
        ost << "{";
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            for(Rank r = RANK_MIN; r < N_RANKS; ++r){
                for(int i = 0; i < ps[pt][r]; ++i){
                    ost << toPiece(pt, r) << ", ";
                }
            }
        }
        ost << "}";
        return ost;
    }
    
    std::string toTableString(const PieceSet4& ps){ // テーブル表示
        std::ostringstream oss;
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            for(Rank r = RANK_MIN; r <= RANK_MAX; ++r){
                oss << " " << ps[pt][r];
            }oss << endl;
        }
        return oss.str();
    }
    std::string to2TableStrings(const PieceSet4& ps0,
                                const PieceSet4& ps1){ // 横に並べたテーブル表示
        std::ostringstream oss;
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            for(Rank r = RANK_MIN; r <= RANK_MAX; ++r){
                oss << " " << ps0[pt][r];
            }
            oss << "  ";
            for(Rank r = RANK_MIN; r <= RANK_MAX; ++r){
                oss << " " << ps1[pt][r];
            }
            oss << endl;
        }
        return oss.str();
    }
    
    using PieceSet = PieceSet4;
    
    template<class callback_t>
    void iteratePieceWithQty(const PieceSet4& ps, const callback_t& callback)noexcept{
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            iteratePieceWithQty(pt, ps[pt], callback);
        }
    }
    template<class callback_t>
    void iteratePiece(const PieceSet4& ps, const callback_t& callback)noexcept{
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            iteratePiece(pt, ps[pt], callback);
        }
    }
    template<class callback_t>
    int iteratePieceWithCount(const PieceSet4& ps, const callback_t& callback)noexcept{
        int cnt = 0;
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            cnt += iteratePieceWithCount(pt, ps[pt], [cnt, &callback](int c, Piece p)->void{
                callback(cnt + c, p);
            });
        }
        return cnt;
    }
    
    template<class callback_t>
    void iteratePqr(const PieceSet4& pqr, const callback_t& callback)noexcept{
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            iterate(BitSet64(pqr[pt].data()), [pt, &callback](unsigned int i)->void{
                callback(toPiece(pt, static_cast<Rank>(i / 4)));
            });
        }
    }
    template<class callback_t>
    void iterateSc(const PieceSet4& sc, const callback_t& callback)noexcept{
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            iterate(BitSet64(sc[pt].data()), [pt, &callback](unsigned int i)->void{
                callback(toPiece(pt, static_cast<Rank>(i / 4)));
            });
        }
    }
    template<class callback_t>
    void iterateScWithCount(const PieceSet4& sc, const callback_t& callback)noexcept{
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            iterateWithIndex(BitSet64(sc[pt].data()), [pt, &callback](unsigned int index, unsigned int i)->void{
                callback(index, toPiece(pt, static_cast<Rank>(i / 4)));
            });
        }
    }
    
    /**************************枚数情報を最小に詰めた牌集合**************************/
    
    constexpr uint32_t ipow5Table[N_RANKS] = { // 5進数テーブル
        ipow(N_ONE_PIECE + 1, 0), ipow(N_ONE_PIECE + 1, 1), ipow(N_ONE_PIECE + 1, 2),
        ipow(N_ONE_PIECE + 1, 3), ipow(N_ONE_PIECE + 1, 4), ipow(N_ONE_PIECE + 1, 5),
        ipow(N_ONE_PIECE + 1, 6), ipow(N_ONE_PIECE + 1, 7), ipow(N_ONE_PIECE + 1, 8),
    };
    
    // ランク展開
    template<int N>
    inline uint64_t extractRanksMin(const uint64_t i)noexcept{
        return i + extractRanksMin<cmax(0, N - 1)>(i) * (N_ONE_PIECE + 1);
    }
    template<>inline constexpr uint64_t extractRanksMin<0>(const uint64_t i)noexcept{ return 0; }
    
    
    // テーブル参照のために最小のビット数で TypePieces を表現
    struct TypePiecesMin{
        uint32_t dat_;
        TypePiecesMin& operator +=(Rank r){
            ASSERT(examRank(r), cerr << (int)r << endl;);
            dat_ += ipow5Table[r];
            return *this;
        }
        TypePiecesMin& operator -=(Rank r){
            ASSERT(examRank(r), cerr << (int)r << endl;);
            dat_ -= ipow5Table[r];
            return *this;
        }
        // グループ系演算
        TypePiecesMin& add(Rank r, int n){
            ASSERT(examRank(r), cerr << (int)r << endl;);
            dat_ += ipow5Table[r] * n;
            return *this;
        }
        TypePiecesMin& sub(Rank r, int n){
            ASSERT(examRank(r), cerr << (int)r << endl;);
            dat_ -= ipow5Table[r] * n;
            return *this;
        }
        // 階段型演算
        template<int N>
        TypePiecesMin& addSeq(Rank r){
            dat_ += extractRanksMin<N>(1) * ipow5Table[r];
            return *this;
        }
        template<int N>
        TypePiecesMin& subSeq(Rank r){
            dat_ -= extractRanksMin<N>(1) * ipow5Table[r];
            return *this;
        }
        template<int N>
        TypePiecesMin& addSeqExcept(Rank r, Rank ex){
            dat_ += extractRanksMin<N>(1) * ipow5Table[r] - ipow5Table[ex];
            return *this;
        }
        template<int N>
        TypePiecesMin& subSeqExcept(Rank r, Rank ex){
            dat_ -= extractRanksMin<N>(1) * ipow5Table[r] - ipow5Table[ex];
            return *this;
        }
        TypePiecesMin& set(const TypePieces& tp){
            dat_ = 0;
            for(Rank r = RANK_MIN; r <= RANK_MAX; ++r){
                dat_ += ipow5Table[r] * tp[r];
            }
            return *this;
        }
        TypePiecesMin& clear()noexcept{
            dat_ = 0;
            return *this;
        }
        
        constexpr uint32_t data()const noexcept{
            return dat_;
        }
        
        constexpr TypePiecesMin():dat_(){}
        constexpr TypePiecesMin(const uint32_t& ad):dat_(ad){}
        TypePiecesMin(const TypePieces& tp):dat_(){
            set(tp);
        }
    };
    
    constexpr TypePiecesMin TYPE_PIECES_MIN_NULL = 0;
    constexpr TypePiecesMin TYPE_PIECES_MIN_NUMBERS_ALL = ipow(N_ONE_PIECE + 1, N_RANKS) - 1;
    constexpr TypePiecesMin TYPE_PIECES_MIN_HONORS_ALL = ipow(N_ONE_PIECE + 1, N_HONORS) - 1;
    
    std::ostream& operator <<(std::ostream& ost, const TypePiecesMin& tpm){
        std::array<int, N_RANKS> ar;
        uint32_t d = tpm.data();
        for(Rank r = RANK_MIN; r <= RANK_MAX; ++r){
            ar[r] = d % (N_ONE_PIECE + 1);
            d /= N_ONE_PIECE + 1;
        }
        ost << ::toString(ar);
        return ost;
    }
    
    struct PieceSetMin : public Pack128<TypePiecesMin, N_PIECE_TYPES>{
        using base_t = Pack128<TypePiecesMin, N_PIECE_TYPES>;
        using each_type_t = TypePiecesMin;
        using this_t = PieceSetMin;
        this_t& operator +=(Piece p){
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            base_t::operator [](toPieceType(p)) += toRank(p);
            return *this;
        }
        this_t& operator -=(Piece p){
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            base_t::operator [](toPieceType(p)) -= toRank(p);
            return *this;
        }
        // グループ系演算
        this_t& add(Piece p, int n){
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            base_t::operator [](toPieceType(p)).add(toRank(p), n);
            return *this;
        }
        this_t& sub(Piece p, int n){
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            base_t::operator [](toPieceType(p)).sub(toRank(p), n);
            return *this;
        }
        // 階段系演算
        template<int N>
        this_t& addSeq(Piece p){
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            base_t::operator [](toPieceType(p)).addSeq<N>(toRank(p));
            return *this;
        }
        template<int N>
        this_t& subSeq(Piece p){
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            base_t::operator [](toPieceType(p)).subSeq<N>(toRank(p));
            return *this;
        }
        template<int N>
        this_t& addSeqExcept(Piece p, Piece ex){
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            base_t::operator [](toPieceType(p)).addSeqExcept<N>(toRank(p), toRank(ex));
            return *this;
        }
        template<int N>
        this_t& subSeqExcept(Piece p, Piece ex){
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            base_t::operator [](toPieceType(p)).subSeqExcept<N>(toRank(p), toRank(ex));
            return *this;
        }
        
        this_t& clear()noexcept{
            base_t::fill(TYPE_PIECES_MIN_NULL);
            return *this;
        }
        this_t& fill()noexcept{
            for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
                base_t::operator [](pt) = TYPE_PIECES_MIN_NUMBERS_ALL;
            }
            base_t::operator [](HONOR) = TYPE_PIECES_MIN_HONORS_ALL;
            return *this;
        }
    };
    
    /**************************枚数集合**************************/
    
    constexpr uint32_t TYPE_SUM_MASK_NUMBERS = (1U << (8 * N_NUMBER_PIECE_TYPES)) - 1;
    constexpr uint32_t TYPE_SUM_MASK_HONORS  = ((1U << (8 * N_HONOR_PIECE_TYPES)) - 1) << (N_NUMBER_PIECE_TYPES * 8);
    
    constexpr BitArray32<8, N_PIECE_TYPES> TYPE_SUM_BASE         = 0x01010101U;
    constexpr BitArray32<8, N_PIECE_TYPES> TYPE_SUM_NUMBERS_BASE = TYPE_SUM_BASE & TYPE_SUM_MASK_NUMBERS;
    constexpr BitArray32<8, N_PIECE_TYPES> TYPE_SUM_HONORS_BASE  = TYPE_SUM_BASE & TYPE_SUM_MASK_HONORS;
    constexpr BitArray32<8, N_PIECE_TYPES> TYPE_SUM_NUMBERS      = TYPE_SUM_NUMBERS_BASE * uint32_t(N_RANKS * N_ONE_PIECE);
    constexpr BitArray32<8, N_PIECE_TYPES> TYPE_SUM_HONORS       = TYPE_SUM_HONORS_BASE * uint32_t(N_HONORS * N_ONE_PIECE);
    constexpr BitArray32<8, N_PIECE_TYPES> TYPE_SUM_ALL          = TYPE_SUM_NUMBERS + TYPE_SUM_HONORS;
    
    //static_assert(TYPE_SUM_ALL.sum() == N_ALL_PIECES,"");
    
    /**************************各牌1ビットの牌集合**************************/
    
    // 1つの種類
    constexpr BitSet64 TYPE_EXISTANCE_NUMBERS      = (1 << N_RANKS) - 1;
    constexpr BitSet64 TYPE_EXISTANCE_HONORS       = (1 << N_HONORS) - 1;
    constexpr BitSet64 TYPE_EXISTANCE_RUN_NUMBERS  = (1 << (N_RANKS - 3 + 1)) - 1;
    constexpr BitSet64 TYPE_EXISTANCE_RUN_HONORS   = 0; // 字牌は順子作成不可
    constexpr BitSet64 TYPE_EXISTANCE_RED_NUMBERS  = 1 << RANK_RED;
    constexpr BitSet64 TYPE_EXISTANCE_RED_HONORS   = 0; // 字牌は赤なし
    
    // 全体
    constexpr uint64_t PIECE_EXISTANCE_BASE         = 0x0001000100010001ULL;
    constexpr uint64_t PIECE_EXISTANCE_MASK_NUMBERS = (1ULL << (16 * N_NUMBER_PIECE_TYPES)) - 1;
    constexpr uint64_t PIECE_EXISTANCE_MASK_HONORS  = ((1ULL << (16 * N_HONOR_PIECE_TYPES)) - 1) << (N_NUMBER_PIECE_TYPES * 16);
    constexpr BitSet64 PIECE_EXISTANCE_NUMBERS      = PIECE_EXISTANCE_MASK_NUMBERS & (PIECE_EXISTANCE_BASE * TYPE_EXISTANCE_NUMBERS);
    constexpr BitSet64 PIECE_EXISTANCE_HONORS       = PIECE_EXISTANCE_MASK_HONORS  & (PIECE_EXISTANCE_BASE * TYPE_EXISTANCE_HONORS);
    constexpr BitSet64 PIECE_EXISTANCE_RUN_NUMBERS  = PIECE_EXISTANCE_MASK_NUMBERS & (PIECE_EXISTANCE_BASE * TYPE_EXISTANCE_RUN_NUMBERS);
    constexpr BitSet64 PIECE_EXISTANCE_RUN_HONORS   = 0; // 字牌は順子作成不可
    constexpr BitSet64 PIECE_EXISTANCE_ALL          = PIECE_EXISTANCE_NUMBERS | PIECE_EXISTANCE_HONORS;
    constexpr BitSet64 PIECE_EXISTANCE_RUN_ALL      = PIECE_EXISTANCE_RUN_NUMBERS | PIECE_EXISTANCE_RUN_HONORS;
    constexpr BitSet64 PIECE_EXISTANCE_RED_ALL      = PIECE_EXISTANCE_MASK_NUMBERS & (PIECE_EXISTANCE_BASE * TYPE_EXISTANCE_RED_NUMBERS);
    
    using PieceExistance = BitSet64;
    
    /**************************牌集合(赤情報付き)**************************/
    
    template<class _base_t>
    struct ExtPieceSetN : public _base_t{
        using base_t = _base_t;
        using this_t = ExtPieceSetN<_base_t>;
        
        PieceExistance red_; // 赤牌
        
        static BitSet64 redMask(Piece p)noexcept{
            return BitSet64(1ULL << p);
        }
        
        void assertBeforeAdd(ExtPiece ep, int n = 1)const{
            base_t::assertBeforeAdd(toPiece(ep), n);
            ASSERT(!isRed(ep) || !red(toPiece(ep)), cerr << "double red in " << toPiece(ep) << endl;);
        }
        void assertBeforeSub(ExtPiece ep, int n = 1)const{
            base_t::assertBeforeSub(toPiece(ep), n);
            ASSERT(!isRed(ep) || red(toPiece(ep)), cerr << "no red in " << toPiece(ep) << endl;);
        }
        
        this_t& operator +=(ExtPiece ep){
            base_t::operator [](toPieceType(ep)) += toRank(ep);
            if(isRed(ep)){
                red_.set(toPiece(ep));
            }
            return *this;
        }
        this_t& operator +=(const this_t& eps)noexcept{
            base_t::operator +=(eps);
            red_ += eps.red_;
            return *this;
        }
        this_t& operator -=(ExtPiece ep){
            base_t::operator [](toPieceType(ep)) -= toRank(ep);
            if(isRed(ep)){
                red_.reset(toPiece(ep));
            }
            return *this;
        }
        this_t& operator -=(const this_t& eps)noexcept{
            base_t::operator -=(eps);
            red_ -= eps.red_;
            return *this;
        }
        this_t operator +(ExtPiece ep)const noexcept{
            BitSet64 tred = red_;
            if(isRed(ep)){
                tred.set(toPiece(ep));
            }
            return this_t(naive() + toPiece(ep), tred);
        }
        this_t operator +(const this_t& eps)const noexcept{
            return this_t(naive() + eps.naive(), red_ + eps.red_);
        }
        this_t operator -(ExtPiece ep)const noexcept{
            BitSet64 tred = red_;
            if(isRed(ep)){
                tred.reset(toPiece(ep));
            }
            return this_t(naive() + toPiece(ep), tred);
        }
        this_t operator -(const this_t& eps)const noexcept{
            return this_t(naive() - eps.naive(), red_ - eps.red_);
        }
        this_t operator ~()const noexcept{
            return this_t(base_t::operator ~(),
                          PIECE_EXISTANCE_RED_ALL & ~red_);
        }
        bool operator ==(const this_t& eps)const noexcept{
            return (base_t::operator ==(eps)) && (red_ == eps.red_);
        }
        bool operator !=(const this_t& eps)const noexcept{
            return !(operator ==(eps));
        }
        // 枚数加減算
        // red を true にしないと赤を後回しにするので注意
        this_t& add(Piece p, int n, bool red = false){
            base_t::assertBeforeAdd(p, n);
            base_t::add(p, n);
            if(red){
                red_.set(p);
            }else{
                if(base_t::operator [](p) >= N_ONE_PIECE){ // TODO: もっと速くできる
                    red_.set(p);
                }
            }
            return *this;
        }
        this_t& sub(Piece p, int n, bool red = false){
            base_t::assertBeforeSub(p, n);
            base_t::sub(p, n);
            if(red){
                red_.reset(p);
            }else{
                if(!contains(p)){
                    red_.reset(p);
                }
            }
            return *this;
        }
        
        // 階段型演算
        template<int N>
        this_t& addSeq(Piece p, bool red = false){
            base_t::operator [](toPieceType(p)).template addSeq<N>(toRank(p));
            const Piece rp = replaceRank(p, RANK_RED);
            if(red){
                red_.set(rp);
            }else{
                if(base_t::operator [](rp) >= N_ONE_PIECE){ // TODO: もっと速くできる
                    red_.set(rp);
                }
            }
            return *this;
        }
        template<int N>
        this_t& subSeq(Piece p, bool red = false){
            base_t::operator [](toPieceType(p)).template subSeq<N>(toRank(p));
            const Piece rp = replaceRank(p, RANK_RED);
            if(red){
                red_.reset(rp);
            }else{
                if(!contains(rp)){
                    red_.reset(rp);
                }
            }
            return *this;
        }
        template<int N>
        this_t& addSeqExcept(Piece p, Piece ex, bool red = false){
            base_t::operator [](toPieceType(p)).template addSeqExcept<N>(toRank(p), toRank(ex));
            const Piece rp = replaceRank(p, RANK_RED);
            if(red){
                red_.set(rp);
            }else{
                if(base_t::operator [](rp) >= N_ONE_PIECE){ // TODO: もっと速くできる
                    red_.set(rp);
                }
            }
            return *this;
        }
        template<int N>
        this_t& subSeqExcept(Piece p, Piece ex, bool red = false){
            base_t::operator[] (toPieceType(p)).template subSeqExcept<N>(toRank(p), toRank(ex));
            const Piece rp = replaceRank(p, RANK_RED);
            if(red){
                red_.reset(rp);
            }else{
                if(!contains(rp)){
                    red_.reset(rp);
                }
            }
            return *this;
        }
        
        // 限定
        this_t pick(Piece p)const{
            return this_t(base_t::pick(p), red_ & redMask(p));
        }
        this_t& limit(Piece p){
            base_t::limit(p);
            red_ &= redMask(p);
            return *this;
        }
        this_t& clearPiece(Piece p){
            base_t::clearPiece(p);
            red_.reset(p);
            return *this;
        }
        
        // 赤 <-> 通常 変換
        this_t& flipToRed(Piece p){
            //base_t::operator -=(p);
            red_.set(p);
            return *this;
        }
        this_t& flipToNormal(Piece p){
            //base_t::operator +=(p);
            red_.reset(p);
            return *this;
        }
        PieceExistance red(Piece p)const noexcept{
            return red_.test(p);
        }
        PieceExistance red()const noexcept{
            return red_.any();
        }
        bool contains(Piece p)const{
            return base_t::contains(p);
        }
        bool contains(ExtPiece ep)const{
            return contains(toPiece(ep)) && (!isRed(ep) || red(toPiece(ep)));
        }
        base_t naive()const noexcept{
            return PieceSet4(*this);
        }
        typename base_t::each_type_t naive(PieceType pt)const{
            return base_t::operator [](pt);
        }
        
        this_t& clear()noexcept{
            base_t::clear();
            red_.reset();
            return *this;
        }
        this_t& fill()noexcept{
            base_t::fill();
            red_ = PIECE_EXISTANCE_RED_ALL;
            return *this;
        }
        
        bool exam()const{
            if(!base_t::exam()){
                cerr << "ExtPieceSetN::exam() : illegal naive piece-set" << endl;
                return false;
            }
            // 赤の限定性チェック
            if(!PIECE_EXISTANCE_RED_ALL.holds(red_)){
                cerr << "ExtPieceSetN::exam() : illegal red piece" << endl;
            }
            // 赤情報との無矛盾性チェック
            for(PieceIndex pi = PIECE_INDEX_MIN; pi <= PIECE_INDEX_MAX; ++pi){
                Piece p = toPiece(pi);
                if(red(p) && base_t::operator [](p) == 0){
                    // 赤フラグがあるのに1枚もないのはおかしい
                    cerr << "ExtPieceSetN::exam() : has red piece but qty of piece ";
                    cerr << p << " is 0 " << (*this) << " r " << red_ << endl;
                    return false;
                }
                if(PIECE_EXISTANCE_RED_ALL.holds(p) && !red(p) && base_t::operator [](p) == N_ONE_PIECE){
                    // 全部揃っているのに赤フラグがないのはおかしい
                    cerr << "ExtPieceSetN::exam() : qty of piece " << p;
                    cerr << " is full but no red piece " << (*this) << " r " << red_ << endl;
                    return false;
                }
            }
            return true;
        }
        
        ExtPieceSetN():
        base_t(), red_(){}
        
        ExtPieceSetN(Piece p):
        base_t(p), red_(0){}
        
        ExtPieceSetN(ExtPiece ep):
        base_t(toPiece(ep)), red_(0){
            if(isRed(ep)){
                red_.set(toPiece(ep));
            }
        }
        ExtPieceSetN(const base_t& ps, const PieceExistance& red):
        base_t(ps), red_(red){}
        ExtPieceSetN(const this_t& eps):
        base_t(eps), red_(eps.red_){}
    };
    
    using ExtPieceSet3 = ExtPieceSetN<PieceSet3>;
    using ExtPieceSet4 = ExtPieceSetN<PieceSet4>;

    template<class base_t>
    std::ostream& operator<<(std::ostream& ost, const ExtPieceSetN<base_t>& eps){
        ost << "{";
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            for(Rank r = RANK_MIN; r < N_RANKS; ++r){
                if(eps.red(toPiece(pt, r))){
                    for(int i = 0; i < eps[pt][r] - 1; ++i){
                        ost << toPiece(pt, r) << ", ";
                    }
                    ost << "\\" << toPiece(pt, r) << ", "; // 赤牌
                }else{
                    for(int i = 0; i < eps[pt][r]; ++i){
                        ost << toPiece(pt, r) << ", ";
                    }
                }
            }
        }
        ost << "}";
        return ost;
    }
    
    using ExtPieceSet = ExtPieceSet4;
    
    const ExtPieceSet EXT_PIECE_SET_ALL = ExtPieceSet(PIECE_SET_ALL, PIECE_EXISTANCE_RED_ALL);
    
    // イテレーション
    // 赤牌の扱いが面倒すぎるのでもう別々にしたい...
    template<class callback_t>
    void iteratePieceWithQty(const ExtPieceSet& eps, const callback_t& callback){
        // 赤を特別視しない場合
        iteratePieceWithQty(eps.naive(), callback);
    }
    template<class callback_t>
    void iterateExtPieceWithQty(const ExtPieceSet& eps, const callback_t& callback){
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            typename ExtPieceSet::base_t::each_type_t tp = eps[pt];
            if(eps.red(toPiece(pt, RANK_RED))){
                callback(toExtPiece(pt, RANK_RED, true), 1);
                tp -= RANK_RED;
            }
            // 通常牌 1枚少なくなるので注意
            iteratePieceWithQty(pt, tp, [&callback](Piece p, int n)->void{
                callback(toExtPiece(p), n);
            });
        }
    }
    template<class callback_t>
    void iterateExtPiece(const ExtPieceSet& eps, const callback_t& callback){
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            typename ExtPieceSet::base_t::each_type_t tp = eps[pt];
            if(eps.red(toPiece(pt, RANK_RED))){
                callback(toExtPiece(pt, RANK_RED, true));
                tp -= RANK_RED;
            }
            iteratePiece(pt, tp, [&callback](Piece p)->void{
                callback(toExtPiece(p));
            });
        }
    }
    template<class callback_t>
    int iterateExtPieceWithCount(const ExtPieceSet& eps, const callback_t& callback){
        int cnt = 0;
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            typename ExtPieceSet::base_t::each_type_t tp = eps[pt];
            if(eps.red(toPiece(pt, RANK_RED))){
                callback(cnt, toExtPiece(pt, RANK_RED, true));
                tp -= RANK_RED;
                cnt += 1;
            }
            cnt += iteratePieceWithCount(pt, tp, [cnt, &callback](int c, Piece p)->void{
                callback(cnt + c, toExtPiece(p));
            });
        }
        return cnt;
    }
    
    /**************************牌集合に対するアルゴリズム**************************/
    
    PieceSet4 convQR_PQR_each(const PieceSet4& qr)noexcept{
        // 64ビットずつ計算
        PieceSet4 val;
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            val[pt] = convQR_PQR(qr[pt]);
        }
        return val;
    }
    PieceSet4 convPQR_SC_each(const PieceSet4& pqr)noexcept{
        // 64ビットずつ計算
        PieceSet4 val;
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            val[pt] = convPQR_SC(pqr[pt]);
        }
        return val;
    }
    
    bits256_t convQR_PQR(const bits256_t& qr){
        // 256ビット一気に計算
        const bits256_t iqr = ~qr;
        const bits256_t qr_l1 = (qr << 1);
        const bits256_t r = (PQR256_1 & qr & (iqr >> 1)) | (PQR256_2 & qr & (iqr << 1)) | ((qr & qr_l1) << 1) | (qr_l1 & PQR256_4);
        return r;
    }
    bits256_t convPQR_SC(const bits256_t& pqr){
        // 256ビット一気に計算
        bits256_t r = pqr;
        r |= (r & PQR256_234) >> 1;
        r |= (r & PQR256_34) >> 2;
        return r;
    }
    
    /**************************牌ハッシュ**************************/
    
    // ハッシュ値計算用
    // 赤牌も一緒に扱えるように128種類にしておく
    /*constexpr uint64_t pieceHashTable[1 << (6 + 1)] = { // ただの乱数テーブル
        0xdc4c2e18f55b3db9, 0x523c509521427925, 0x85c3903476284e73, 0xd0c90896151d31fc,
        0x51fe5b9ea4a1aae3, 0x4d39bdb66a501fe6, 0x1b2b7c37d1aee2e5, 0x76b02032dab4802b,
        0x077fc6e362f84724, 0x20d0e6d01b25b9f9, 0xef9ac05cc46588e7, 0xd3a2859947122e14,
        0x611198e21a3650da, 0xf5438af92eea8e77, 0x592802b589bb7ec0, 0x66cdab8f332e334b,
        0x95d3d4767bd518f1, 0x3b144b982e1352bc, 0x97c4e63b85be079d, 0x4bf92944e1c7bcd8,
        0xf0e2d9f09864e480, 0xc00535fbd239ed76, 0x5a3b03433d8cdb65, 0x9210435e7265ebaa,
        0xab65554e639f6d9e, 0x2fb64a706e812c65, 0x3d89b1d9760460ed, 0x081b29ee0dfbac53,
        0xf596fb9d6a16e442, 0xdb974b3c1b9fe55c, 0x071a3b3c8b9e8d27, 0x79ffd498212ca328,
        0xf789e33c182d0a48, 0x228f2612afda75c6, 0xcea9724f2720162d, 0xb7320fe6f891da33,
        0xd0514a0f4ab87791, 0xf6b1b94df27aa18c, 0x099b5b15e0480115, 0x89b61dcf58f77c09,
        0x726585434bb4e7d0, 0x7671f6f80ad33751, 0xd6b01d22ea4faab6, 0x874b82578af27426,
        0xcdf8000f57cf25b1, 0x9679fd2d19515299, 0xae1b0459ca77ebc5, 0x15a1fa9e8b636c50,
        0x14ace8b7134b9b26, 0xaac6a4f4a09810ba, 0x6c953ac8978c0ee6, 0x107947b1f09c5bd3,
        0xb9e8607eeb9d485b, 0x716c2c969fd62e2b, 0x1c50b58d80673c63, 0x8bff134705fde6dd,
        0x3af8c674b3d35f69, 0xf588eeca34081bae, 0xaa7b6bc966a3c2e9, 0x215872a3bc07a43e,
        0xb42b6a347cd7b5a5, 0x129bd455921b28ae, 0x8ecc0c25f860c0dd, 0x4d9b46c7540a719e,
        0x4a34cac64703ea7c, 0x46418a892936a05a, 0xe3bf67cb2e73c681, 0x32be473799cfcc92,
        0x0d681a15927f53ac, 0xd1ecd0a1e8732ac2, 0xb071e330bfab6c5b, 0x294fc1d80d56c23c,
        0xcf8c78d5b5ed9c28, 0x292c0b0c83ebc54b, 0x640b0d43c8ff49f3, 0x0d5ea5d6532160c2,
        0xe94e661fbd9ff4a4, 0xce47ac35e8394cdc, 0xa962cd54a42bfc61, 0x68deb6df10596dd8,
        0xffeabfa92362a83c, 0x8ba8456ec7c80593, 0xaf40ed8f2cab9249, 0xa8bfd21c784adf74,
        0xac5a85132591b0bb, 0xdfbb4e817f526fa7, 0x5d014b8523e591dc, 0x4ea7fb9ecede8514,
        0x1f8f4c025d077299, 0x2d000050f5d3bbec, 0x4478f8df690e0d9d, 0xff256581deb1aa8d,
        0x3cf850df87b65c04, 0xb1ae2a3d18b37fff, 0xaf6706c0f7fc490c, 0xf5bb038dc26f876a,
        0x1388dbf8b4863703, 0x3fa19bcb7c404bd1, 0xd0263dc4ee973166, 0xec723516b9274dad,
        0x37b57bc6e1cc4395, 0x995fb61bd7444ff9, 0x6564d46ac3f37dd5, 0x0cc9e13446c93585,
        0x65d7eee5d8797d30, 0xdb5b81f2fbf58cc0, 0x9cea975264efaebd, 0x99be22e6768df6a9,
        0x09240525912903ac, 0x612ea350b82b21d0, 0xb7db347a6936a188, 0xc26d06219f4324d5,
        0xe31f6a07b9b8835d, 0x013120bad262e448, 0xea7dd501a59f8e17, 0xa0a7df0c75fc0baf,
        0x2fcd597f5263a7e6, 0xe8ab0f13c2528865, 0x6de377aa61faf3a2, 0x4a72fd0bf19ca156,
        0x4a2cbaa8b6ec5c4a, 0x58922de64f2144ae, 0x90455fa4b210e81b, 0x1cd89d8495fc98ca,
        0x550ed400d2d9cf63, 0x2e7f7db02e38c07b, 0x724d4e33287021a9, 0xd06f909fd54017de,
    };*/
    
    std::array<uint64_t, (1 << (6 + 1))> pieceHashTable;
    std::array<uint64_t, (1 << (6 + 1))> seqPieceHashTable; // sequence 一括計算用
    
    constexpr uint64_t PIECE_HASH_NULL = 0;
    //constexpr uint64_t PIECE_HASH_ALL  = 0xf327fa7f338f81a1; // テーブルからの事前計算の結果
    uint64_t PIECE_HASH_ALL;
    
    uint64_t getPieceHashKeyByPQR(const PieceSet4& pqr, const PieceExistance red){
        uint64_t key = 0;
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            iterate(BitSet64(pqr[pt]), [&key, pt](unsigned int i){
                Rank r = static_cast<Rank>(i / 4);
                int n = (i % 4) + 1;
                key += pieceHashTable[toPiece(pt, r)] * n;
            });
            if(red.test(toPiece(pt, RANK_RED))){
                key -= pieceHashTable[toPiece(pt, RANK_RED)];
                key += pieceHashTable[toExtPiece(pt, RANK_RED, true)];
            }
        }
        return key;
    }
    uint64_t getPieceHashKey(const ExtPieceSet& eps){
        uint64_t key = 0;
        /*for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            iterateAnyWithIndex(eps[pt], [&key, pt](int r, int n){
                key += pieceHashTable[toPiece(pt, static_cast<Rank>(r))] * n;
            });
            if(eps.red(toPiece(pt, RANK_RED))){
                key -= pieceHashTable[toPiece(pt, RANK_RED)];
                key += pieceHashTable[toExtPiece(pt, RANK_RED, true)];
            }
        }*/
        iterateExtPieceWithQty(eps, [&key](ExtPiece ep, int n)->void{
            key += pieceHashTable[ep] * n;
        });
        return key;
    }
    
    int initHash()noexcept{
        XorShift64 dice(1114);
        for(int i = 0; i < 64; ++i){
            pieceHashTable[i] = dice.rand();
        }
        for(int i = 64; i < 128; ++i){
            if(isRedRankPiece(static_cast<Piece>(i - 64))){
                pieceHashTable[i] = dice.rand();
            }else{
                pieceHashTable[i] = pieceHashTable[i - 64];
            }
        }
        
        ExtPieceSet eps;
        eps.fill();
        uint64_t allKey = getPieceHashKey(eps);
        //printf("all key = 0x%016llx\n", allKey);
        PIECE_HASH_ALL = allKey;
        
        seqPieceHashTable.fill(0);
        for(PieceIndex pi = PIECE_INDEX_MIN; pi <= PIECE_INDEX_MAX; ++pi){
            Piece p = toPiece(pi);
            for(int s = 0; s < N_CHOW_PIECES; ++s){
                seqPieceHashTable[p] += pieceHashTable[p + s];
            }
        }
        // 赤含みseq
        for(PieceIndex pi = PIECE_INDEX_MIN; pi <= PIECE_INDEX_MAX; ++pi){
            Piece p = toPiece(pi);
            ExtPiece ep = toExtPiece(p, true);
            for(int s = 0; s < N_CHOW_PIECES; ++s){
                if(isRedRankPiece(p + s)){
                    seqPieceHashTable[ep] += pieceHashTable[toExtPiece(p + s, true)];
                }else{
                    seqPieceHashTable[ep] += pieceHashTable[toExtPiece(p + s, false)];
                }
            }
        }
        
        return 0;
    }
    
    /**************************基本性質計算**************************/
    
    uint32_t *shantenTable;
    uint64_t *acceptableTable0;
    uint64_t *acceptableTable1;
    
    uint32_t minimumStepsInfoTable[ipow(N_ONE_PIECE + 1, N_RANKS)];
    uint64_t acceptableInfoTable[ipow(N_ONE_PIECE + 1, N_RANKS) * 2];
    
    int initShantenTable(){
        std::ifstream ifs("./data/shanten_table.txt");
        if(!ifs){
            cerr << "failed to open Shanten Table." << endl; return -1;
        }
        
        shantenTable = (uint32_t*)malloc(sizeof(uint32_t) * 0x07ffffff);
        if(shantenTable == nullptr){
            cerr << "failed to obtain memories of Shanten Table." << endl; return -1;
        }
        memset(shantenTable, 0, sizeof(uint32_t) * 0x07ffffff);
        const char delim = ' ';
        std::string str;
        while(std::getline(ifs, str)){
            auto result = split(str, delim);
            shantenTable[atoi(result[0].c_str())] = atoi(result[1].c_str());
        }
        
        return 0;
    }
    
    int initAcceptableTable(){
        std::ifstream ifs("./data/acceptable_table.txt");
        if(!ifs){
            cerr << "failed to open Acceptable Table." << endl; return -1;
        }
        acceptableTable0 = (uint64_t*)malloc(sizeof(uint64_t) * ipow(5, N_RANKS));
        if(acceptableTable0 == nullptr){
            cerr << "failed to obtain memories of Acceptable Table 0." << endl; return -1;
        }
        acceptableTable1 = (uint64_t*)malloc(sizeof(uint64_t) * ipow(5, N_RANKS));
        if(acceptableTable1 == nullptr){
            cerr << "failed to obtain memories of Acceptable Table 1." << endl; return -1;
        }
        memset(acceptableTable0, 0, sizeof(uint64_t) * ipow(5, N_RANKS));
        memset(acceptableTable1, 0, sizeof(uint64_t) * ipow(5, N_RANKS));
        
        std::string str;
        const char delim = ' ';
        while(std::getline(ifs, str)){
            auto result = split(str, delim);
            if(result[2] == "0"){
                acceptableTable0[atoi(result[0].c_str())] = atoll(result[1].c_str());
            }else{
                acceptableTable1[atoi(result[0].c_str())] = atoll(result[1].c_str());
            }
        }
        return 0;
    }
    
    int initMinimumStepsInfoTable(){
        std::ifstream ifs("./data/minimumStepsInfoTable.dat");
        if(!ifs){
            cerr << "failed to open minimumStepsInfoTable.dat!" << endl;
            return -1;
        }
        memset(minimumStepsInfoTable, 0, sizeof(minimumStepsInfoTable));
        uint32_t mmax = 0;
        std::string str;
        int cnt = 0;
        while(std::getline(ifs, str)){
            minimumStepsInfoTable[cnt] = atoi(str.c_str());
            cnt += 1;
            mmax = max(mmax, minimumStepsInfoTable[cnt - 1] & 3);
        }
        cerr << mmax << endl;
        return 0;
    }
    
    int initAcceptableInfoTable(){
        std::ifstream ifs("./data/acceptableInfoTable.dat");
        if(!ifs){
            cerr << "failed to open acceptableInfoTable.dat!" << endl;
            return -1;
        }
        memset(acceptableInfoTable, 0, sizeof(acceptableInfoTable));
        std::string str;
        int cnt = 0;
        while(std::getline(ifs, str)){
            acceptableInfoTable[cnt] = atoll(str.c_str());
            cnt += 1;
        }
        return 0;
    }
    
    template<class hand_t>
    int calcAcceptableBits(const hand_t& hand, int fu_ro, BitSet64 *const pac){
        
        //cerr << "calc shanten of " << hand << endl;
        
        int j_m = 0; // 字牌が3つ以上揃っている個数
        int j_t = 0; // 字牌がちょうど2つ揃っている個数
        int val = N_DEALT_PIECES + 1;
        int shanten[N_NUMBER_PIECE_TYPES][N_RANKS];
        int m[3] = {0}; // 数?
        uint64_t ukeire_val[3] = {0};
        
        // 数牌
        for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
            // jに数牌の3x9ビットの情報を押し込む
            int j = hand[pt][RANK_MIN];
            //ASSERT((unsigned)j <= N_ONE_PIECE, cerr << j << endl;);
            for(Rank r = RANK_MIN + 1; r <= RANK_MAX; ++r){
                j <<= 3;
                j |= hand[pt][r];
                //ASSERT((unsigned)hand[pt][r] <= N_ONE_PIECE, cerr << j << endl;);
            }
            DERR << BitArray32<3, 9>(j) << endl;
            
            uint32_t kv = shantenTable[j]; // テーブルを引く
            m[pt] = (kv & 0x00000003);
            
            DERR << "m kv = " << kv << " m = " << m[pt] << endl;
            
            kv >>= 2;
            for(int i = 0; i < m[pt]; ++i){
                shanten[pt][i * 3] = (kv & 0x00000001);
                kv >>= 1;
                shanten[pt][i * 3 + 1] = (kv & 0x00000007);
                kv >>= 3;
                shanten[pt][i * 3 + 2] = (kv & 0x00000007);
                kv >>= 3;
            }
        }
        
        // 字牌 2個以上のときのみ関係あり
        for(Piece p = PIECE_HONOR_MIN; p <= PIECE_HONOR_MAX; ++p){
            if(hand[p] == 0){ continue; }
            else if(hand[p] == 2){ ++j_t; }
            else if(hand[p] >= 3){
                ++j_m;
            }
        }
    
        // それぞれの数牌の組数に対して全探索
        std::vector<int> pro; // パターンを全部入れる
        for(int k = 0; k < m[0]; ++k){
            for(int kk = 0; kk < m[1]; ++kk){
                for(int kkk = 0; kkk < m[2]; ++kkk){
                    //cerr << k << " " << kk << " " << kkk << endl;
                    
                    int head_ex = shanten[0][k * 3] + shanten[1][kk * 3] + shanten[2][kkk * 3] + j_t >= 1;
                    int mentu = shanten[0][k * 3 + 2] + shanten[1][kk * 3 + 2] + shanten[2][kkk * 3 + 2] + j_m + fu_ro;
                    int tarts = shanten[0][k * 3 + 1] + shanten[1][kk * 3 + 1] + shanten[2][kkk * 3 + 1] + j_t + shanten[0][k * 3]
                    + shanten[1][kk * 3] + shanten[2][kkk * 3] - head_ex;
                    
                    DERR << head_ex << ", " << mentu << ", " << tarts << endl;
                    
                    int tmp;
                    if(mentu + tarts > 4){
                        tmp = 8 - 2 * mentu - 4 + mentu - head_ex; // 4 - mentu - head
                    }else{
                        tmp = 8 - 2 * mentu - tarts - head_ex;
                    }
                    if(val > tmp){
                        pro.clear();
                        val = tmp;
                        pro.push_back(mentu+tarts);
                        pro.push_back(shanten[0][k * 3]);
                        pro.push_back(shanten[1][kk * 3]);
                        pro.push_back(shanten[2][kkk * 3]);
                        pro.push_back(j_t);
                    }else if(val == tmp){
                        int flag = 1;
                        for(int i = 0; i < (signed)pro.size(); i += 5){
                            if(mentu + tarts == pro[i]
                               && shanten[0][k * 3] == pro[i + 1]
                               && shanten[1][kk * 3] == pro[i + 2]
                               && shanten[2][kkk * 3] == pro[i + 3]
                               && j_t == pro[i + 4]){
                                flag = 0;
                            }
                        }
                        if(flag == 1){
                            pro.push_back(mentu + tarts);
                            pro.push_back(shanten[0][k * 3]);
                            pro.push_back(shanten[1][kk * 3]);
                            pro.push_back(shanten[2][kkk * 3]);
                            pro.push_back(j_t);
                        }
                    }
                }
            }
        }
        pac->reset();
        for(int pat = 0; pat < (signed)pro.size(); pat += 5){
            
            int flag[3] = {0};
            for(int i = 0; i < 3; ++i){
                if(pro[pat + i + 1] == 0){
                    flag[i] = 1;
                }
            }
            int atama[4] = {0};
            atama[0] = pro[pat + 2] + pro[pat + 3]+pro[pat + 4];
            atama[1] = pro[pat + 1] + pro[pat + 3]+pro[pat + 4];
            atama[2] = pro[pat + 1] + pro[pat + 2]+pro[pat + 4];
            atama[3] = pro[pat + 1] + pro[pat + 2]+pro[pat + 3] + pro[pat + 4];
            
            int shift[3] = {0};
            for(int i = 0; i < 3; ++i){
                if(pro[pat] < 3){ shift[i] = 54; flag[i] = 0; }
                else if(pro[pat] == 3 && atama[i] == 0 && pro[pat + i + 1] == 0){ shift[i] = 45; }
                else if(pro[pat] == 3 && atama[i] == 0){ shift[i] = 45; }
                else if(pro[pat] == 3 && atama[i] > 0 && pro[pat + i + 1] == 0){ shift[i] = 36; }
                else if(pro[pat] == 3 && atama[i] > 0){ shift[i] = 36; }
                else if(pro[pat] == 4 && atama[i] == 0 && pro[pat + i + 1] == 0){ shift[i] = 27; }
                else if(pro[pat] == 4 && atama[i] == 0){ shift[i] = 27; }
                else if(pro[pat] == 4 && atama[i] > 0 && pro[pat + i + 1] == 0){ shift[i] = 18; }
                else if(pro[pat] == 4 && atama[i] > 0){ shift[i] = 18; }
                else if(pro[pat] > 4 && atama[i] == 0  && pro[pat + i + 1] == 0){ shift[i] = 9; }
                else if(pro[pat] > 4 && atama[i] == 0){ shift[i] = 9; }
                else if(pro[pat] > 4 && atama[i] > 0 && pro[pat + i + 1] == 0){ shift[i] = 0; }
                else if(pro[pat] > 4 && atama[i] > 0){ shift[i] = 0; }
            }
            
            for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
                int j = hand[pt][RANK_MIN];
                for(Rank r = RANK_MIN + 1; r <= RANK_MAX; ++r){
                    j *= 5;
                    j += hand[pt][r];
                }
                if(flag[pt] == 0){
                    ukeire_val[pt] = acceptableTable0[j];
                }else{
                    ukeire_val[pt] = acceptableTable1[j];
                }
                
                DERR << "m " << TypePiecesMin(j) << " flag " << flag[pt] << " " << ukeire_val[pt] << endl;
            }
            if(pro[pat] < 4 || atama[3] == 0){
                for(Piece p = PIECE_HONOR_MIN; p <= PIECE_HONOR_MAX; ++p){
                    if(hand[p] >= 1 && hand[p] <= 2){
                        pac->set(p);
                    }
                }
            }else if(!((atama[3] == 1) && pro[pat + 4] == 1)){
                for(Piece p = PIECE_HONOR_MIN; p <= PIECE_HONOR_MAX; ++p){
                    if(hand[p] == 2){
                        pac->set(p);
                    }
                }
            }
            for(int i = 0; i < 3; ++i){
                ukeire_val[i] >>= shift[i];
                ukeire_val[i] &= 0x000001ff;
            }
            for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
                for(Rank r = RANK_MAX; (signed)r >= (signed)RANK_MIN; --r){
                    //cerr << pt << " " << r << endl;
                    pac->set_value(toPiece(pt, r), ukeire_val[pt] & 0x00000001);
                    ukeire_val[pt] >>= 1;
                }
            }
        }
        
        return val;
    }
    
    template<class hand_t>
    int calcAfterAcceptableBits(const hand_t& hand, Piece p, int fu_ro, BitSet64 *const pacceptable){
        PieceSet ps = hand.piece;
        ps -= p;
        return calcAcceptableBits(ps, fu_ro, pacceptable);
    }
    
    template<class hand_t>
    int calcMinimumSteps(const hand_t& hand, PieceExistance *const pac){
        const uint64_t honorPqr = hand.pqr[HONOR]; // 字牌のPQR
        const int opened = hand.openedMelds(); // 副露数
        const int h3 = countBits64(honorPqr & PQR_34); // 字牌が3つ以上揃っている個数
        const int h2 = countBits64(honorPqr & PQR_2); // 字牌がちょうど2つ揃っている個数
        int steps = N_DEALT_PIECES + 1; // シャンテン数
        int shanten[N_NUMBER_PIECE_TYPES][N_RANKS];
        int m[3]; // 数? (0 ~ 3)
        uint64_t ukeire_val[3];
        
        // 数牌
        for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
            uint32_t kv = minimumStepsInfoTable[hand.pieceMin[pt].data()]; // テーブルを引く
            m[pt] = kv & 3; // 0 ~ 3
            //DERR << "kv = " << kv << " m = " << m[pt] << endl;
            
            kv >>= 2;
            for(int i = 0; i < m[pt]; ++i){
                shanten[pt][i * 3] = kv & 0x00000001;
                kv >>= 1;
                shanten[pt][i * 3 + 1] = kv & 0x00000007;
                kv >>= 3;
                shanten[pt][i * 3 + 2] = kv & 0x00000007;
                kv >>= 3;
            }
        }
        
        // それぞれの数牌の組数に対して全探索
        std::array<int, ipow(3, N_NUMBER_PIECE_TYPES) * 5> pro; // パターンを全部入れる
        int index = 0;
        for(int k = 0; k < m[0]; ++k){
            for(int kk = 0; kk < m[1]; ++kk){
                for(int kkk = 0; kkk < m[2]; ++kkk){
                    int head_ex = shanten[0][k * 3] + shanten[1][kk * 3] + shanten[2][kkk * 3] + h2 >= 1; // どれかの種類に頭がある
                    //ASSERT(head_ex <= 1,);
                    int mentu = shanten[0][k * 3 + 2] + shanten[1][kk * 3 + 2] + shanten[2][kkk * 3 + 2] + h3 + opened;
                    
                    int tarts = shanten[0][k * 3 + 1] + shanten[1][kk * 3 + 1] + shanten[2][kkk * 3 + 1] + h2 + shanten[0][k * 3]
                    + shanten[1][kk * 3] + shanten[2][kkk * 3] - head_ex; // 頭の数を引く
                    
                    //DERR << head_ex << ", " << mentu << ", " << tarts << endl;
                    
                    int tmp;
                    if(mentu + tarts > 4){
                        tmp = 8 - 2 * mentu - 4 + mentu - head_ex; // 4 - mentu - head_ex
                    }else{
                        tmp = 8 - 2 * mentu - tarts - head_ex;
                    }
                    if(tmp < steps){ // 最小更新
                        index = 0;
                        steps = tmp;
                        pro[index    ] = mentu + tarts;
                        pro[index + 1] = shanten[0][k * 3];
                        pro[index + 2] = shanten[1][kk * 3];
                        pro[index + 3] = shanten[2][kkk * 3];
                        pro[index + 4] = h2;
                        index += 5;
                    }else if(tmp == steps){ // 同点
                        int flag = 1;
                        // 同じパターンが見つかったら
                        for(int i = 0; i < index; i += 5){
                            if(mentu + tarts == pro[i]
                               && shanten[0][k * 3] == pro[i + 1]
                               && shanten[1][kk * 3] == pro[i + 2]
                               && shanten[2][kkk * 3] == pro[i + 3]
                               && h2 == pro[i + 4]){
                                flag = 0;
                                break;
                            }
                        }
                        if(flag == 1){
                            pro[index    ] = mentu + tarts;
                            pro[index + 1] = shanten[0][k * 3];
                            pro[index + 2] = shanten[1][kk * 3];
                            pro[index + 3] = shanten[2][kkk * 3];
                            pro[index + 4] = h2;
                            index += 5;
                        }
                    }
                }
            }
        }
        pac->reset();
        for(int pat = 0; pat < (signed)index; pat += 5){
            // 全パターンどれでも減ればOK
            //int atama[4];
            //atama[3] = pro[pat + 1] + pro[pat + 2] + pro[pat + 3] + pro[pat + 4];
            //atama[0] = pro[pat + 2] + pro[pat + 3] + pro[pat + 4];
            //atama[1] = pro[pat + 1] + pro[pat + 3] + pro[pat + 4];
            //atama[2] = pro[pat + 1] + pro[pat + 2] + pro[pat + 4];
            
            int headAll = pro[pat + 1] + pro[pat + 2] + pro[pat + 3] + pro[pat + 4];
            
            for(PieceType pt = PIECE_TYPE_NUMBERS_MIN; pt <= PIECE_TYPE_NUMBERS_MAX; ++pt){
                int head = headAll - pro[pat + 1 + pt];
                int flag, shift;
                if(pro[pat] < 3){
                    flag = 0;
                    shift = 54;
                }else{
                    flag = (pro[pat + pt + 1] == 0) ? 1 : 0;
                    shift = 9 * ((max(0, 5 - pro[pat])) * 2 + ((head == 0) ? 1 : 0));
                }
                
                /*int shift = 0;
                if(pro[pat] < 3){ shift = 54; flag = 0; }
                else if(pro[pat] == 3 && head == 0){ shift = 45; }
                else if(pro[pat] == 3 && head > 0){ shift = 36; }
                else if(pro[pat] == 4 && head == 0){ shift = 27; }
                else if(pro[pat] == 4 && head > 0){ shift = 18; }
                else if(pro[pat] > 4 && head == 0){ shift = 9; }
                //else if(pro[pat] > 4 && head > 0){ shift = 0; }*/
                
                uint64_t accdata = acceptableInfoTable[hand.pieceMin[pt].data() * 2 + flag];
                pac->operator |=(((accdata >> shift) & ((1 << N_RANKS) - 1)) << toPiece(pt, RANK_MIN));
            }
            // 特別な場合
            if(pro[pat] < 4 || headAll == 0){ // 1枚以上2枚以下の字牌がOK
                pac->operator |=(pext(honorPqr | (honorPqr << 1), PQR_2) << toPiece(HONOR, RANK_MIN));
            }else if(!((headAll == 1) && pro[pat + 4] == 1)){ // ちょうど2枚の字牌がOK
                pac->operator |=(pext(honorPqr & PQR_2, PQR_2) << toPiece(HONOR, RANK_MIN));
            }
        }
        return steps;
    }
    
    /**************************牌集合からの性質計算**************************/
    
    // qrから判定
    bool isAllInsideNumbers(const PieceSet& ps)noexcept{ // 断么
        return holds(NAIVE_MASK_INSIDE_NUMBERS, ps);
    }
    bool isAllDoubles(const PieceSet& ps)noexcept{ // 七対子
        // 同じ牌4つを2-2とはできないのでこれでOK
        return holds(PIECE_SET_DOUBLE, ps);
    }
    
    // pqrから判定
    bool is9Types9PiecesPqr(const PieceSet& pqr)noexcept{ // 九種九牌
        return (pqr & NAIVE_MASK_9T9P).sum1() >= 9;
    }
    
    // scから判定
    bool isFullOrphansSc(const PieceSet& sc){ // 国士無双
        return holds(sc, SET_SC_FULL_ORPHANS);
    }
    
    /**************************牌のランダム分配**************************/
    
    // 牌を一つランダムに取り出す
    template<class dice_t>
    Rank dealRank(const TypePieces& tp, uint32_t sumQty, dice_t *const pdice){
        ASSERT(tp.sum() == sumQty, cerr << tp << " <-> " << sumQty << endl;);
        int n = pdice->rand() % sumQty;
        Rank r = RANK_MIN;
        for(; r <= RANK_MAX; ++r){
            n -= tp[r];
            if(n < 0){ break; }
        }
        return r;
    }
    template<class dice_t>
    Rank dealRank(const TypePieces& tp, dice_t *const pdice){
        uint32_t sumQty = tp.sum();
        return dealRank(tp, sumQty, pdice);
    }
    
    template<class dice_t, class array_t>
    Piece dealPiece(const PieceSet& ps, const array_t& typeSum, unsigned int sumQty, dice_t *const pdice){
        // まずどの種類にするか決定
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            ASSERT(ps[pt].sum() == typeSum[pt], cerr << ps << " <-> " << typeSum << endl;);
        }
        ASSERT(ps.sum() == sumQty, cerr << typeSum << " <-> " << sumQty << endl;);
        int r = pdice->rand() % sumQty;
        PieceType pt = PIECE_TYPE_MIN;
        for(; pt <= PIECE_TYPE_MAX; ++pt){
            r -= typeSum[pt];
            if(r < 0){ break; }
        }
        return toPiece(pt, dealRank(ps[pt], typeSum[pt], pdice));
    }
    template<class dice_t, class array_t>
    ExtPiece dealExtPiece(const ExtPieceSet& eps, const array_t& typeSum, unsigned int sumQty, dice_t *const pdice){
        Piece p = dealPiece(eps, typeSum, sumQty, pdice);
        if(eps.red(p)){
            return toExtPiece(p, (pdice->rand() % eps[p]) == 0);
        }
        return toExtPiece(p);
    }
    template<class dice_t>
    Piece dealPiece(const PieceSet& ps, dice_t *const pdice){
        auto typeSum = ps.typeSum();
        uint32_t sumQty = 0;
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            sumQty += typeSum[pt];
        }
        return dealPiece(ps, typeSum, sumQty, pdice);
    }
    template<class dice_t>
    ExtPiece dealExtPiece(const ExtPieceSet& eps, dice_t *const pdice){
        Piece p = dealPiece(eps, pdice);
        if(eps.red(p)){
            return toExtPiece(p, (pdice->rand() % eps[p]) == 0);
        }
        return toExtPiece(p);
    }
    
    // 牌集合を指定された枚数に分割する
    template<class hands_t, class array_t, class dice_t>
    void dealPieces(const ExtPieceSet& eps, hands_t *const pdst, const array_t& qty, unsigned int sumQty, dice_t *const pdice){
        ASSERT(qty.sum() == sumQty, cerr << qty << " <-> " << sumQty << endl;);
        ASSERT(eps.sum() == sumQty, cerr << eps << " <-> " << sumQty << endl;);
        array_t tmpQty = qty;
        iterateExtPiece(eps, [pdst, &tmpQty, &sumQty, &pdice](ExtPiece ep)->void{
            int r = pdice->rand() % sumQty;
            std::size_t i = 0;
            for(; i < tmpQty.size(); ++i){
                r -= tmpQty[i];
                if(r < 0){ break; }
            }
            (*pdst)[i] += ep;
            sumQty -= 1;
            tmpQty.subtr(i, 1);
        });
    }
    
}

#endif // MAHJONG_STRUCTURE_PIECE_HPP_