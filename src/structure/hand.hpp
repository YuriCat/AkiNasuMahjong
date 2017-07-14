/*
 hand.hpp
 Katsuki Ohto
 */

// 手牌の詳しい表現

#ifndef MAHJONG_STRUCTURE_HAND_HPP_
#define MAHJONG_STRUCTURE_HAND_HPP_

#include "base.hpp"
#include "piece.hpp"
#include "action.hpp"

namespace Mahjong{

    /**************************詳しい牌情報**************************/

    // 1人の牌情報

    // 牌の情報は主に以下の3つに分かれる
    // 1. 手牌の情報
    // 2. 鳴いた組牌の情報
    // 3. 1,2に絡めてゲームに関わる発展的情報
    
    // 特に3については、牌を引いた瞬間に正確になっておけばよいものが多いので
    // 加減算を非対称に組める
    
    // TODO: 組み牌の赤情報が実際の盤面を反映しないことがある
    // 例えば 3-4-5 の seq と 5-5-5 の group があるときにどちらに赤があるのかわからないが問題ある?
    
    // 更新順序
    // 1stHalf
    //   - piece (通常qr)
    //   - pieceMin (5進数)
    //   - pqr
    // 2ndHalf
    //   -

    struct Hand{
        
        // 1. 手牌の情報
        ExtPieceSet piece; // 手牌一覧
        PieceSetMin pieceMin; // シャンテン数、受け入れ数計算用の最小型
        PieceSet4 pqr, sc; // 特徴要素等計算用
        PieceExistance existance; // 牌が1枚でもあるかどうか
        //BitSet64 concealedTriQuad; // 門前を崩さない刻子と槓子(大明槓含む)
        PieceExistance runExistance; // 順子が少なくとも1組ある位置
        BitArray32<8, N_PIECE_TYPES> pieces; // 各種類の枚数
        uint32_t allPieces; // 全枚数
        uint64_t pieceHashKey; // 手牌のハッシュキー
        
        // 2. オープンな組牌の情報
        ExtPieceSet openedPiece; // 鳴きによって確定した牌一覧
        // 鳴きによって完成しているgroup 3 or 4 なのでこれで十分だろう
        static constexpr int kGroupSizePatterns = N_ONE_PIECE - N_MIN_OPENED_GROUP_PIECES + 1;
        std::array<PieceExistance, kGroupSizePatterns> openedGroup;
        PieceSet4 openedSeqQr; // 鳴きによって完成しているsequenceのQR
        int openedGroups, openedSeqs; // 数
        int numPicked; // 鳴いた数(門前判定に利用)
        uint64_t openedHashKey; // オープン状態のハッシュキー
        
        // 3. ゲームに関わる発展的情報
        int minimumSteps; // シャンテン数
        BitSet64 acceptable; // 受け入れ牌ビット
        std::array<uint32_t, N_NUMBER_PIECE_TYPES> minimumStepsInfo; // シャンテン数計算用情報
        
        // 性質アクセス
        bool isFishing()const noexcept{ // 聴牌状態
            return minimumSteps == 0;
        }
        int openedMelds()const noexcept{ // オープンな役の総数
            return openedGroups + openedSeqs;
        }
        bool isConcealed()const noexcept{ // 門前判定
            return numPicked == 0; // 暗槓は含まない
        }
        bool isAcceptable(Piece p)const noexcept{ // 受け入れ状態
            return acceptable.test(p);
        }
        PieceExistance openedGroupAll()const noexcept{
            return openedGroup[0] | openedGroup[1];
        }
        bool hasOpenedGroup(Piece p)const noexcept{ // 加槓可能性判定用
            return openedGroupAll().test(p);
        }
        int countAllReds()const noexcept{
            return countBits(piece.red() | openedPiece.red());
        }
        
        // 手牌の基本情報アクセス
        int operator [](Piece p)const{
            ASSERT(examPiece(p), cerr << (int)p << endl;);
            return piece[p];
        }
        TypePieces4 operator [](PieceType pt)const{
            ASSERT(examPieceType(pt), cerr << (int)pt << endl;);
            return piece[pt];
        }
        uint64_t red(Piece p)const{
            return piece.red(p);
        }
        uint64_t red()const{
            return piece.red();
        }
        
        void setStepInfo(){
            minimumSteps = calcMinimumSteps(*this, &acceptable);
        }
        
        // 設定関数 遅いので毎回呼ぶのは厳禁
        void setConcealedInfo(const ExtPieceSet4& eps,
                              BitArray32<8, N_PIECE_TYPES> apieces, uint32_t apiecesSum)noexcept{
            piece = eps;
            existance.reset();
            for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
                pieceMin[pt].set(eps[pt]); // pieceMinを設定(遅い)
                existance |= pext(sc[pt], PQR_1) << toPiece(pt, RANK_MIN); // 存在型を設定
            }
            runExistance = existance & (existance >> 1) & (existance >> 2); // 階段存在型を設定
            // pqr, scを設定
            pqr = convQR_PQR(eps);
            sc = convPQR_SC(pqr);
            pieces = apieces;
            allPieces = apiecesSum;
        }
        void setConcealedInfo(const ExtPieceSet4& eps, BitArray32<8, N_PIECE_TYPES> apieces)noexcept{
            setConcealedInfo(eps, apieces, apieces.sum());
        }
        void setConcealedInfo(const ExtPieceSet4& eps)noexcept{
            BitArray32<8, N_PIECE_TYPES> tpieces = 0;
            for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
                tpieces.set(pt, eps[pt].sum());
            }
            setConcealedInfo(eps, tpieces);
        }
        void setConcealedInfoAll(const ExtPieceSet4& eps)noexcept{
            setConcealedInfo(eps);
            pieceHashKey = getPieceHashKeyByPQR(pqr, piece.red_); // ハッシュ値設定
            setStepInfo();
        }
        void set(const ExtPieceSet4& eps){
            setConcealedInfo(eps);
            clearOpenedInfo();
        }
        void setAll(const ExtPieceSet4& eps){
            setConcealedInfoAll(eps);
            clearOpenedInfo();
        }
        
        /*Hand& add1stHalf(ExtPiece ep){
         
         PieceType pt = toPieceType(ep);
         Rank r = toRank(p);
         
         const uint64_t rankMask = 15ULL << (r << 2);
         
         if(piece[pt][r]){
         pqr[pt] = (pqr[pt] & ~rankMask) | ((pqr[pt] & rankMask) << 1);
         sc[pt]  |= ((sc[pt] & rankMask) << 1);
         }else{
         pqr[pt] |= 1ULL << (r << 2);
         sc[pt]  |= 1ULL << (r << 2);
         }
         
         piece += ep;
         pieceMin += toPiece(ep);
         
         pieces.add(pt, 1);
         
         return *this;
         }*/
        
        // 加算関数
        // 単体
        void add(ExtPiece ep){
            const Piece p = toPiece(ep);
            const PieceType pt = toPieceType(ep);
            const Rank r = toRank(ep);
            
            if(!piece.contains(p)){ // 元々同じ牌はなかった
                pqr[pt] |= 1ULL << (r << 2);
                sc[pt]  |= 1ULL << (r << 2);
                existance.set(p);
                runExistance = existance & (existance >> 1) & (existance >> 2);
            }else{ // あった
                const uint64_t rankMask = 15ULL << (r << 2);
                const uint64_t dpqr = (pqr[pt] & rankMask) << 1;
                pqr[pt] = (pqr[pt] & ~rankMask) | dpqr;
                sc[pt] |= dpqr;
            }
            piece += ep;
            pieceMin += p;
            
            pieces.add(pt, 1);
            allPieces += 1;
        }
        // グループ系 暗槓のとき以外は枚数全部
        void addGroup(Piece p, int n, bool red){
            const PieceType pt = toPieceType(p);
            const Rank r = toRank(p);
            
            if(!piece.contains(p)){ // 元々同じ牌はなかった
                pqr[pt] |= 1ULL << ((r << 2) + n);
                sc[pt]  |= ((1ULL << (n + 1)) - 1) << (r << 2);
                existance.set(p);
                runExistance = existance & (existance >> 1) & (existance >> 2);
            }else{ // あった
                const uint64_t rankMask = 15ULL << (r << 2);
                const uint64_t dpqr = ((pqr[pt] & rankMask) << n);
                pqr[pt] = (pqr[pt] & ~rankMask) | ((pqr[pt] & rankMask) << n);
                sc[pt]  |= ((1ULL << (piece[p] + n + 1)) - 1) << (r << 2);
            }
            piece.add(p, n, red);
            pieceMin.add(p, n);
            
            pieces.add(pt, n);
            allPieces += n;
        }
        // 階段系
        void addSeq(Piece p, bool red){
            const PieceType pt = toPieceType(p);
            const Rank r = toRank(p);
            
            const uint64_t rankMask = extractRanks<N_CHOW_PIECES>(15ULL) << (r << 2);
            pqr[pt] = ((pqr[pt] & rankMask) << 1) | (~sc[pt] & rankMask & PQR_1) | (pqr[pt] & ~rankMask);
            sc[pt] |= ((sc[pt]  & rankMask) << 1) | (rankMask & PQR_1);
            
            existance &= ~(0xFFFFULL << toPiece(pt, RANK_MIN));
            existance |= pext(sc[pt], PQR_1) << toPiece(pt, RANK_MIN);
            
            runExistance = existance & (existance >> 1) & (existance >> 2);
            
            piece.addSeq<N_CHOW_PIECES>(p, red);
            pieceMin.addSeq<N_CHOW_PIECES>(p);
            
            pieces.add(pt, N_CHOW_PIECES);
            allPieces += N_CHOW_PIECES;
        }
        void addSeqExcept(Piece p, Piece ex, bool red){
            const PieceType pt = toPieceType(p);
            const Rank r = toRank(p);
            const Rank exr = toRank(ex);
            
            const uint64_t rankMask = (extractRanks<N_CHOW_PIECES>(15ULL) << (r << 2)) - (15ULL << (exr << 2));
            pqr[pt] = ((pqr[pt] & rankMask) << 1) | (~sc[pt] & rankMask & PQR_1) | (pqr[pt] & ~rankMask);
            sc[pt] |= ((sc[pt]  & rankMask) << 1) | (rankMask & PQR_1);
            
            existance &= ~(0xFFFFULL << toPiece(pt, RANK_MIN));
            existance |= pext(sc[pt], PQR_1) << toPiece(pt, RANK_MIN);
            
            runExistance = existance & (existance >> 1) & (existance >> 2);
            
            piece.addSeqExcept<N_CHOW_PIECES>(p, ex, red);
            pieceMin.addSeqExcept<N_CHOW_PIECES>(p, ex);
            
            pieces.add(pt, N_CHOW_PIECES - 1);
            allPieces += N_CHOW_PIECES - 1;
        }
        
        void addAll(ExtPiece ep){
            add(ep);
            pieceHashKey += pieceHashTable[ep];
            setStepInfo();
        }
        void addGroupAll(Piece p, int n, bool red){
            addGroup(p, n, red);
            pieceHashKey += pieceHashTable[p] * (n - 1) + pieceHashTable[toExtPiece(p, red)];
            setStepInfo();
        }
        void addSeqAll(Piece p, bool red){
            addSeq(p, red);
            pieceHashKey += seqPieceHashTable[toExtPiece(p, red)];
            setStepInfo();
        }
        void addSeqExceptAll(Piece p, Piece ex, bool red){
            addSeqExcept(p, ex, red);
            pieceHashKey += seqPieceHashTable[toExtPiece(p, red)];
            pieceHashKey -= pieceHashTable[toExtPiece(ex, red)];
            setStepInfo();
        }
        
        /*Hand& sub1stHalf(ExtPiece ep){
         piece -= ep;
         pieceMin -= toPiece(ep);
         
         PieceType pt = toPieceType(ep);
         Rank r = toRank(p);
         
         const uint64_t rankMask = 15ULL << (r << 2);
         pqr[pt] = (pqr[pt] & ~rankMask) | ((pqr[pt] >> 1) & rankMask);
         
         return *this;
         }*/
        
        // 減算関数
        void sub(ExtPiece ep){
            const Piece p = toPiece(ep);
            const PieceType pt = toPieceType(ep);
            const Rank r = toRank(ep);
            
            piece -= ep;
            pieceMin -= p;
            
            const uint64_t rankMask = 15ULL << (r << 2);
            if(!piece.contains(p)){ // 同じ牌がなくなった
                pqr[pt] &=  ~rankMask;
                sc[pt]  &=  ~rankMask;
                
                existance.reset(p);
                runExistance = existance & (existance >> 1) & (existance >> 2);
            }else{ // まだある
                const uint64_t opqr = pqr[pt];
                pqr[pt] = (pqr[pt] & ~rankMask) | ((pqr[pt] & rankMask) >> 1);
                sc[pt] -= opqr & rankMask;
            }
            pieces.subtr(pt, 1);
            allPieces -= 1;
        }
        // グループ系 暗槓のとき以外は枚数全部
        void subGroup(Piece p, int n, bool red){
            const PieceType pt = toPieceType(p);
            const Rank r = toRank(p);
            
            piece.sub(p, n, red);
            pieceMin.sub(p, n);
            
            const uint64_t rankMask = 15ULL << (r << 2);
            if(!piece.contains(p)){ // 同じ牌がなくなった
                pqr[pt] &=  ~rankMask;
                sc[pt]  &=  ~rankMask;
                
                existance.reset(p);
                runExistance = existance & (existance >> 1) & (existance >> 2);
            }else{ // まだある
                pqr[pt] = (pqr[pt] & ~rankMask) | ((pqr[pt] & rankMask) >> n);
                sc[pt]  = (sc[pt]  & ~rankMask) | ((sc[pt]  & rankMask) >> n);
            }
            pieces.subtr(pt, n);
            allPieces -= n;
        }
        // 階段系
        void subSeq(Piece p, bool red){
            const PieceType pt = toPieceType(p);
            const Rank r = toRank(p);
            
            piece.subSeq<N_CHOW_PIECES>(p, red);
            pieceMin.subSeq<N_CHOW_PIECES>(p);
            
            const uint64_t rankMask = extractRanks<N_CHOW_PIECES>(15ULL) << (r << 2);
            pqr[pt] = (pqr[pt] & ~rankMask) | ((pqr[pt] >> 1) & rankMask & PQR_123);
            sc[pt]  = (sc[pt]  & ~rankMask) | ((sc[pt]  >> 1) & rankMask & PQR_123);
            
            existance &= ~(0xFFFFULL << toPiece(pt, RANK_MIN));
            existance |= pext(sc[pt], PQR_1) << toPiece(pt, RANK_MIN);
            
            runExistance = existance & (existance >> 1) & (existance >> 2);
            
            pieces.subtr(pt, N_CHOW_PIECES);
            allPieces -= N_CHOW_PIECES;
        }
        void subSeqExcept(Piece p, Piece ex, bool red){
            const PieceType pt = toPieceType(p);
            const Rank r = toRank(p);
            const Rank exr = toRank(ex);
            
            piece.subSeqExcept<N_CHOW_PIECES>(p, ex, red);
            pieceMin.subSeqExcept<N_CHOW_PIECES>(p, ex);
            
            const uint64_t rankMask = (extractRanks<N_CHOW_PIECES>(15ULL) << (r << 2)) - (15ULL << (exr << 2));
            pqr[pt] = (pqr[pt] & ~rankMask) | ((pqr[pt] >> 1) & rankMask & PQR_123);
            sc[pt]  = (sc[pt]  & ~rankMask) | ((sc[pt]  >> 1) & rankMask & PQR_123);
            
            existance &= ~(0xFFFFULL << toPiece(pt, RANK_MIN));
            existance |= pext(sc[pt], PQR_1) << toPiece(pt, RANK_MIN);
            
            runExistance = existance & (existance >> 1) & (existance >> 2);
            
            pieces.subtr(pt, N_CHOW_PIECES - 1);
            allPieces -= N_CHOW_PIECES - 1;
        }
        
        void subAll(ExtPiece ep){
            sub(ep);
            pieceHashKey -= pieceHashTable[ep];
            setStepInfo();
        }
        void subGroupAll(Piece p, int n, bool red){
            subGroup(p, n, red);
            pieceHashKey -= pieceHashTable[p] * (n - 1) + pieceHashTable[toExtPiece(p, red)];
            setStepInfo();
        }
        void subSeqAll(Piece p, bool red){
            subSeq(p, red);
            pieceHashKey -= seqPieceHashTable[toExtPiece(p, red)];
            setStepInfo();
        }
        void subSeqExceptAll(Piece p, Piece ex, bool red){
            subSeqExcept(p, ex, red);
            pieceHashKey -= seqPieceHashTable[toExtPiece(p, red)];
            pieceHashKey += pieceHashTable[toExtPiece(ex, red)];
            setStepInfo();
        }
        // 鳴いた役の更新関数
        // 加算
        void addOpenedSeq(Meld m, ExtPiece picked, const ExtPieceSet& opened){
            openedSeqQr += m.piece();
            openedPiece += opened;
            openedPiece += picked;
            openedSeqs += 1;
            numPicked += 1;
        }
        void addOpenedSeq(Meld m){
            openedSeqQr += m.piece();
            openedPiece.addSeq<N_CHOW_PIECES>(m.piece(), m.red());
            openedSeqs += 1;
            numPicked += 1;
        }
        void addOpenedGroup(Meld m, ExtPiece picked, const ExtPieceSet& opened){
            openedGroup[m.qty() - N_MIN_OPENED_GROUP_PIECES].set(picked);
            openedPiece += opened;
            openedPiece += picked;
            openedGroups += 1;
            numPicked += 1;
        }
        void addOpenedGroup(Meld m){
            openedGroup[m.qty() - N_MIN_OPENED_GROUP_PIECES].set(m.piece());
            openedPiece.add(m.piece(), m.qty(), m.red());
            openedGroups += 1;
            numPicked += 1;
        }
        void expandOpenedGroup(ExtPiece added){
            // 3枚 -> 4枚
            openedGroup[0].flip(toPiece(added));
            openedGroup[1].flip(toPiece(added));
            openedPiece += added;
        }
        void addOpenedGroupNoPick(Meld m, const ExtPieceSet& opened){
            // 全ての牌が一括で与えられる暗槓の場合
            openedGroup[m.qty() - N_MIN_OPENED_GROUP_PIECES].set(m.piece());
            openedPiece += opened;
            openedGroups += 1;
        }
        void addOpenedGroupNoPick(Meld m){
            // 全ての牌が一括で与えられる暗槓の場合
            openedGroup[m.qty() - N_MIN_OPENED_GROUP_PIECES].set(m.piece());
            openedPiece.add(m.piece(), m.qty(), m.red());
            openedGroups += 1;
        }
        // 減算
        void subOpenedSeq(Meld m){
            openedSeqQr -= m.piece();
            openedPiece.subSeq<N_CHOW_PIECES>(m.piece(), m.red());
            openedSeqs -= 1;
            numPicked -= 1;
        }
        void subOpenedGroup(Meld m){
            openedGroup[m.qty() - N_MIN_OPENED_GROUP_PIECES].reset(m.piece());
            openedPiece.sub(m.piece(), m.qty(), m.red());
            openedGroups -= 1;
            numPicked -= 1;
        }
        void subOpenedGroupNoPick(Meld m){
            openedGroup[m.qty() - N_MIN_OPENED_GROUP_PIECES].reset(m.piece());
            openedPiece.sub(m.piece(), m.qty(), m.red());
            openedGroups -= 1;
        }
        void reduceOpenedGroup(ExtPiece subtracted){
            // 4枚 -> 3枚
            openedGroup[0].flip(toPiece(subtracted));
            openedGroup[1].flip(toPiece(subtracted));
            openedPiece -= subtracted;
        }
        
        // 行動による全書き換え操作
        /*void doNoPickGroup(Meld m){
            subGroup(m.piece(), m.qty(), m.red());
            addOpenedGroupNoPick(m);
        }
        void undoNoPickGroup(Meld m){
            addGroup(m.piece(), m.qty(), m.red());
            subOpenedGroupNoPick(m);
        }
        void doAddGroup(Meld m){
            sub(m.extPiece());
            expandOpenedGroup(m.ExtPiece());
        }
        void undoAddGroup(Meld m){
            add(m.extPiece());
            reduceOpenedGroup(m.extPiece());
        }
        void doPickGroup(Meld m){
            subGroup(m.piece(), m.qty() - 1, m.red());
            addOpenedGroup(m);
        }
        void undoPickGroup(Meld m){
            addGroup(m.piece(), m.qty() - 1, m.red());
            subOpenedGroup(m);
        }
        void doPickSeq(Meld m, ExtPiece picked){
            subSeqExcept(m.piece(), toPiece(picked), m.red());
            addOpenedSeq(m);
        }
        void undoPickSeq(Meld m, ExtPiece picked){
            addSeqExcept(m.piece(), toPiece(picked), m.red());
            subOpenedSeq(m);
        }*/

        void clearPieceInfo()noexcept{
            piece.clear();
            pieceMin.clear();
            pqr.clear();
            sc.clear();
            existance.reset();
            runExistance.reset();
            pieceHashKey = PIECE_HASH_NULL;
        }
        void fillPieceInfo()noexcept{
            piece.fill();
            pieceMin.fill();
            pqr.fillPqr();
            sc.fillSc();
            existance = PIECE_EXISTANCE_ALL;
            runExistance = PIECE_EXISTANCE_RUN_ALL;
            pieceHashKey = PIECE_HASH_ALL;
        }
        void clearOpenedInfo()noexcept{
            openedSeqs = openedGroups = numPicked = 0;
            openedPiece.clear();
            openedGroup.fill(PieceExistance(0));
            openedSeqQr.clear();
            openedHashKey = OPENED_HASH_NULL;
        }
        
        void clear()noexcept{
            clearPieceInfo();
            clearOpenedInfo();
        }
        
        void fill()noexcept{
            fillPieceInfo();
            clearOpenedInfo();
        }
        
        PieceSet naive()const noexcept{
            return PieceSet(piece);
        }
        TypePieces naive(PieceType pt)const noexcept{
            return piece.naive(pt);
        }
        
        // validation
        bool examPieces()const{
            if(!piece.exam()){
                cerr << "Hand::examPiecces() : illegal base ext-piese-set" << endl;
                return false;
            }
            return true;
        }
        bool examPqr()const{
            for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
                if(convQR_PQR(piece[pt]) != pqr[pt]){
                    cerr << "Hand::examPqr() : inconsistent base piece-set <-> pqr in pt " << pt << endl;
                    cerr << piece[pt] << " <-> " << pqr[pt] << endl;
                    return false;
                }
            }
            return true;
        }
        bool examSc()const{
            for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
                if(convPQR_SC(pqr[pt]) != sc[pt]){
                    cerr << "Hand::examSc() : inconsistent pqr <-> sc in pt " << pt << endl;
                    cerr << pqr[pt] << " <-> " << sc[pt] << endl;
                    return false;
                }
            }
            return true;
        }
        bool examMin()const{
            for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
                TypePiecesMin tmp;
                tmp.clear().set(piece[pt]);
                if(tmp.data() != pieceMin[pt].data()){
                    cerr << "Hand::examMin() : inconsistent piece <-> pieceMin in pt " << pt << endl;
                    cerr << tmp.data() << " <-> " << pieceMin[pt].data() << endl;
                    return false;
                }
            }
            return true;
        }
        bool examPieceHashkey()const{
            uint64_t tkey = getPieceHashKey(piece);
            if(pieceHashKey != tkey){
                cerr << "Hand::examPieceHashKey() : wrong key " << pieceHashKey << " <-> " << tkey << endl;
                return false;
            }
            return true;
        }
        bool examOpenedSeq()const{
            if(openedSeqQr.sum() != openedSeqs){
                cerr << "Hand::examOpenedSeq() : inconsistent openedSeqs <-> openedSeqQr" << endl;
                cerr << openedSeqs << " <-> " << openedSeqQr << endl;
                return false;
            }
            return true;
        }
        bool examOpenedGroup()const{
            if(!isExclusive(openedGroup[0], openedGroup[1])){
                cerr << "Hand::examOpenedGroup() : group3 and group4 should be exclusive" << endl;
                cerr << "3: " << openedGroup[0] << " 4: " << openedGroup[1] << endl;
                return false;
            }
            if(openedGroupAll().count() != openedGroups){
                cerr << "Hand::examOpenedGrous() : inconsistent openedGroups <-> openedGroupPqr" << endl;
                cerr << openedGroups << " <-> " << "3: " << openedGroup[0] << " 4: " << openedGroup[1] << endl;
                return false;
            }
            return true;
        }
        bool examConcealedInfo()const{
            if(!examPieces()){ return false; }
            if(!examPqr()){ return false; }
            if(!examSc()){ return false; }
            if(!examMin()){ return false; }
            if(!examPieceHashkey()){ return false; }
            return true;
        }
        bool examOpenedInfo()const{
            if(!examOpenedSeq()){ return false; }
            if(!examOpenedGroup()){ return false; }
            return true;
        }
        bool exam()const{
            if(!examConcealedInfo()){ return false; }
            if(!examOpenedInfo()){ return false; }
            return true;
        }
        std::string toString()const{
            std::ostringstream oss;
            oss << piece;
            ExtPieceSet tmp = openedPiece;
            for(std::size_t i = 0; i < openedGroup.size(); ++i){
                iterate(openedGroup[i], [&oss, &tmp, i](int index)->void{
                    Piece p = static_cast<Piece>(index);
                    int q = i + N_MIN_OPENED_GROUP_PIECES;
                    bool r = tmp.red(p);
                    oss << " " << toGroupMeld(p, q, r);
                    tmp.sub(p, q, true); // 赤優先で減算
                });
            }
            iteratePiece(openedSeqQr, [&oss, &tmp](Piece p)->void{
                bool r = tmp.red() & PieceExistance(p, p + 1, p + 2);
                oss << " " << toSeqMeld(p, N_CHOW_PIECES, r);
                tmp.subSeq<N_CHOW_PIECES>(p, true); // 赤優先で減算
            });
            return oss.str();
        }
        std::string toDebugString()const{
            std::ostringstream oss;
            oss << "piece = " << piece << endl;
            oss << "pqr   = " << Pqr(pqr) << endl;
            oss << "sc    = " << Sc(sc) << endl;
            oss << "existance = " << existance << endl;
            oss << "runExistance = " << runExistance << endl;
            return oss.str();
        }
    };
    
    std::ostream& operator <<(std::ostream& ost, const Hand& hand){
        ost << hand.toString();
        return ost;
    }
    
    /**************************手牌を捨てた際に更新される情報**************************/
    
    // 手牌から捨てる牌を1つずつ検討する場合などに、新しく更新される情報のうち
    // 単独で利用出来るものは手牌情報の外に新しい値を記録するだけにとどめておく
    // それによって最終的に行動を決定して試合を進めるときに再計算させずに済む
    
    struct NextHandInfo{
        int minimumSteps;
        BitSet64 acceptable;
        int acceptableNum;
    };
    
    void subAll(Hand *const phand, ExtPiece ep, const NextHandInfo& ni){
        
    }
    
    /**************************手牌情報を巻き戻すための一時情報**************************/
    
    // 手牌から1つの牌を捨てる場合など
    // 手牌から1つ引いて後から1つ追加という操作に無駄がある
    // (抜いた牌が元に戻るだけなのに、色々な情報を再計算してしまう)
    // そのためこちらに一時的に元の情報や差分情報を記録しておく
    // 単独で使える値は NextHandInfo にまとめて、
    // 周囲の情報(元のまま)と合わせて利用する情報のみこちらに記録する
    
    struct DiffHandInfo{
        uint32_t minimumStepsInfo;
        
    };
    
    void unsubAll(Hand *const phand, const DiffHandInfo& di){
        
    }
    
    void subTempAll(Hand *const phand, DiffHandInfo *const pdi, NextHandInfo *const pni){
        
    }
    
    /**************************行動による手牌の変化**************************/
    
    // 行動ごとの手牌の変化を毎回書くのがめんどいのでここでまとめてやる
    
    
    void doTurnAction(Hand *const phand, const TurnAction& action,
                      DiffHandInfo *const pdhi, NextHandInfo *const pnhi){
        
        if(action.discard()){ // 打牌
            phand->subAll(action.discarded());
        }else if(action.drawKong()){ // 暗槓
            phand->subGroupAll(action.piece(), N_KONG_PIECES, action.red());
            phand->addOpenedGroupNoPick(action.toKongMeld());
        }else{ // 小明槓
            ExtPiece ep = action.extPiece();
            phand->subAll(ep);
            phand->expandOpenedGroup(ep);
        }
    }
    void undoTurnAction(Hand *const phand, const TurnAction& action,
                        const DiffHandInfo& dhi){
        
        if(action.discard()){ // 打牌
            phand->addAll(action.discarded());
        }else if(action.drawKong()){ // 暗槓
            phand->addGroupAll(action.piece(), N_KONG_PIECES, action.red());
            phand->subOpenedGroup(action.toKongMeld());
        }else{ // 小明槓
            ExtPiece ep = action.extPiece();
            phand->addAll(ep);
            phand->reduceOpenedGroup(ep);
        }
    }
    void doResponseAction(Hand *const phand, const ResponseAction& action, ExtPiece picked,
                          DiffHandInfo *const pdhi, NextHandInfo *const pnhi){
        // チーとポンは打牌まで一気に行う
        if(action.any()){ // パスでない
            if(action.chow()){ // チー
                phand->subSeqExceptAll(action.piece(), toPiece(picked), action.red() && !isRed(picked));
                phand->addOpenedSeq(action);
                phand->subAll(action.discarded());
            }else{ // ポン, 大明槓
                phand->subGroup(action.piece(), action.qty(), action.red() && !isRed(picked));
                phand->addOpenedGroup(action);
                if(action.pong()){ // ポン
                    phand->subAll(action.discarded());
                }
            }
        }
    }
    void undoResponseAction(Hand *const phand, const ResponseAction& action, ExtPiece picked,
                            const DiffHandInfo& dhi){
        // チーとポンは打牌まで一気に行う
        if(action.any()){ // パスでない
            if(action.chow()){ // チー
                phand->addAll(action.discarded());
                phand->subOpenedSeq(action);
                phand->addSeqExceptAll(action.piece(), toPiece(picked), action.red() && !isRed(picked));
            }else{ // ポン, 大明槓
                if(action.pong()){ // ポン
                    phand->addAll(action.discarded());
                }
                phand->subOpenedGroup(action);
                phand->addGroup(action.piece(), action.qty(), action.red() && !isRed(picked));
            }
        }
    }
    
    /**************************プレーヤー以外の牌の情報**************************/
    
    // 色々情報が必要になったら考える
    using UncertainPieces = ExtPieceSet;
    
    /**************************山**************************/
    
    template<std::size_t N>
    struct ExtPieceSequence : public std::array<ExtPiece, N>{
        using base_t = std::array<ExtPiece, N>;
        void clear()noexcept{
            base_t::fill(EXT_PIECE_NONE);
        }
    };
    
    template<std::size_t N>
    std::ostream& operator <<(std::ostream& ost, const ExtPieceSequence<N>& epseq){
        ost << "[";
        if(N > 0){
            ost << epseq[0];
        }
        for(std::size_t i = 1; i < N; ++i){
            ost << "," << epseq[i];
            if(i % N_PIECES == 0){
                ost << endl;
            }
        }
        ost << "]";
        return ost;
    }
    
    using Wall = ExtPieceSequence<N_WALL_PIECES>;

}

#endif // MAHJONG_STRUCTURE_HAND_HPP_