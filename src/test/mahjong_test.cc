/*
 mahjong_test.cc
 Katsuki Ohto
 */

// 麻雀基本演算のテスト

#include "../mahjong.hpp"
#include "../structure/world.hpp"
#include "../structure/field.hpp"

using namespace Mahjong;

Clock cl;

/*int testRankCards(){
    for(int r = RANK_U; r <= RANK_O; ++r){
        Cards test = rankCards(r);
        Cards ans = rankCardsTable[r];
        if(test != ans){
            cerr << "inconsistent Rank -> Cards conversion!" << endl;
            cerr << OutRank(r) << " : " << OutCards(test) << " <-> " << OutCards(ans) << endl;
            return -1;
        }
    }
    for(int r = RANK_U; r <= RANK_O; ++r){
        Cards ans = CARDS_NULL;
        for(int rr = r; rr <= RANK_O; ++rr){
            Cards test = rankCards(r, rr);
            ans |= rankCardsTable[rr];
            if(test != ans){
                cerr << "inconsistent [Rank, Rank] -> Cards conversion!" << endl;
                cerr << "[" << OutRank(r) << ", " << OutRank(rr) << "]";
                cerr << " : " << OutCards(test) << " <-> " << OutCards(ans) << endl;
                return -1;
            }
        }
    }
    return 0;
}

int testRank4xCards(){
    for(int r4x = RANKX4_U; r4x <= RANKX4_O; r4x += 4){
        Cards test = rankx4Cards(r4x);
        Cards ans = rankCardsTable[r4x / 4];
        if(test != ans){
            cerr << "inconsistent Rank4x -> Cards conversion!" << endl;
            cerr << OutRank(r4x / 4) << " : " << OutCards(test) << " <-> " << OutCards(ans) << endl;
            return -1;
        }
    }
    return 0;
}

int testSuitCards(){
    // スート->カード集合変換テスト
    uint64_t time[4] = {0};
    for(uint32_t s = 0; s < 16; ++s){
        cl.start();
        Cards test = suitCards(s);
        time[0] += cl.restart();
        Cards ans = suitCardsTable[s];
        time[1] += cl.stop();
        if(test != ans){
            cerr << "inconsistent Suits -> Cards conversion!" << endl;
            cerr << OutSuits(s) << " : " << OutCards(test) << " <-> " << OutCards(ans) << endl;
            return -1;
        }
    }
    for(uint32_t s = 0; s < 16; ++s){
        cl.start();
        Cards test = unSuitCards(s);
        time[2] += cl.restart();
        Cards ans = CARDS_CDHS & (~suitCardsTable[s]);
        time[3] += cl.stop();
        if(test != ans){
            cerr << "inconsistent unSuits -> Cards conversion!" << endl;
            cerr << OutSuits(s) << " : " << OutCards(test) << " <-> " << OutCards(ans) << endl;
            return -1;
        }
    }
    cerr << "s  -> c test : " << time[0] << " clock" << endl;
    cerr << "s  -> c ans  : " << time[1] << " clock" << endl;
    cerr << "us -> c test : " << time[2] << " clock" << endl;
    cerr << "us -> c ans  : " << time[3] << " clock" << endl;
    return 0;
}

int testQR(const std::vector<Cards>& sample){
    // QR（枚数型）のテスト
    uint64_t time[2] = {0};
    for(Cards c : sample){
        cl.start();
        BitArray64<4> test = convCards_QR(c);
        time[0] += cl.restart();
        BitArray64<4> ans = convCards_QR_slow(c);
        time[1] += cl.stop();
        if(test != ans){
            cerr << "inconsistent Cards -> QR conversion!" << endl;
            cerr << OutCards(c) << " : " << test << " <-> " << ans << endl;
            return -1;
        }
    }
    cerr << "c -> qr test : " << time[0] << " clock" << endl;
    cerr << "c -> qr ans  : " << time[1] << " clock" << endl;
    return 0;
}*/

int testContaining(const std::vector<PieceSet>& sample){
    for(const PieceSet& ps : sample){
        for(PieceIndex pi = PIECE_INDEX_MIN; pi <= PIECE_INDEX_MAX; ++pi){
            Piece p = toPiece(pi);
            bool test = ps.contains(p);
            bool ans = ps[p] > 0;
            if(test != ans){
                return -1;
            }
        }
    }
    return 0;
}

/*int testHolding(const std::vector<PieceSet>& sample){
    for(const PieceSet& ps : sample){
        const PieceSet& opp = sample[dice.rand() % sample.size()];
        bool test = ps.holds(opp);
        bool ans = true;
        for(PieceIndex pi = PIECE_INDEX_MIN; pi <= PIECE_INDEX_MAX; ++pi){
            Piece p = toPiece(pi);
            if(ps[p] >= opp[p]){
                ans = false; break;
            }
        }
        if(test != ans){
            return -1;
        }
    }
    return 0;
}*/

int testCounting(const std::vector<PieceSet>& sample){
    // count each type
    for(const PieceSet& ps : sample){
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            uint64_t test = ps[pt].sum();
            uint64_t ans = 0;
            for(Rank r = RANK_MIN; r <= RANK_MAX; ++r){
                ans += ps[pt][r];
            }
            if(test != ans){
                return -1;
            }
        }
    }
    // whole count
    for(const PieceSet& ps : sample){
        uint64_t test = ps.sum();
        uint64_t ans = 0;
        for(PieceIndex pi = PIECE_INDEX_MIN; pi <= PIECE_INDEX_MAX; ++pi){
            Piece p = toPiece(pi);
            ans += ps[p];
        }
        if(test != ans){
            return -1;
        }
    }
    return 0;
}

uint64_t convQR_PQR_slow(uint64_t aqr)noexcept{
    BitArray64<4> qr = aqr;
    BitArray64<4> ret = 0;
    for(int r = 0; r < 16; ++r){
        if(qr[r]){
            ret.set(r, 1 << (qr[r] - 1));
        }
    }
    return ret.data();
}
uint64_t convQR_SC_slow(uint64_t aqr)noexcept{
    BitArray64<4> qr = aqr;
    BitArray64<4> ret = 0;
    for(int r = 0; r < 16; ++r){
        ret.set(r, (1 << qr[r]) - 1);
    }
    return ret.data();
}
uint64_t convPQR_SC_slow(uint64_t apqr)noexcept{
    BitArray64<4> pqr = apqr;
    BitArray64<4> ret = 0;
    for(int r = 0; r < 16; ++r){
        if(pqr[r]){
            uint32_t q = bsf(pqr[r]) + 1;
            ret.set(r, (1 << q) - 1);
        }
    }
    return ret.data();
}

int testPQR(const std::vector<PieceSet4>& sample){
    // PQR（枚数位置型）のテスト
    uint64_t time[4] = {0};
    uint64_t dummyTypes;
    PieceSet4 dummy;
    dummy.clear();
    // 一致テスト
    // タイプ別
    for(const PieceSet4& ps : sample){
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            const TypePieces4 tp = ps[pt];
            BitArray64<4> test = (uint64_t)convQR_PQR(tp);
            BitArray64<4> ans = convQR_PQR_slow(tp);
            if(test != ans){
                cerr << "inconsistent QR -> PQR conversion!" << endl;
                cerr << tp << " : " << test << " <-> " << ans << endl;
                return -1;
            }
        }
    }
    // 一括
    for(const PieceSet4& ps : sample){
        PieceSet4 all = convQR_PQR(ps);
        PieceSet4 each = convQR_PQR_each(ps);
        if(all != each){
            cerr << "inconsistent QR -> PQR conversion!" << endl;
            cerr << ps << " : " << all << " <-> " << each << endl;
            return -1;
        }
    }
    // 時間計測
    // タイプ別
    for(const PieceSet4& ps : sample){
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            const TypePieces4 tp = ps[pt];
            cl.start();
            BitArray64<4> test = (uint64_t)convQR_PQR(tp);
            time[0] += cl.stop();
            dummyTypes += test;
        }
    }
    for(const PieceSet4& ps : sample){
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            const TypePieces4 tp = ps[pt];
            cl.start();
            BitArray64<4> ans = convQR_PQR_slow(tp);
            time[1] += cl.stop();
            dummyTypes += ans;
        }
    }
    // 一括
    for(const PieceSet4& ps : sample){
        cl.start();
        PieceSet4 all = convQR_PQR(ps);
        time[2] += cl.stop();
        dummy += all;
    }
    for(const PieceSet4& ps : sample){
        cl.start();
        PieceSet4 each = convQR_PQR_each(ps);
        time[3] += cl.stop();
        dummy += each;
    }
    
    cerr << "qr -> pqr (type) test : " << time[0] / (sample.size() * (int)N_PIECE_TYPES) << " clock" << endl;
    cerr << "qr -> pqr (type) ans  : " << time[1] / (sample.size() * (int)N_PIECE_TYPES) << " clock" << endl;
    cerr << "qr -> pqr (full) all  : " << time[2] / sample.size() << " clock" << endl;
    cerr << "qr -> pqr (full) each : " << time[3] / sample.size() << " clock" << endl;
    return 0;
}

int testSC(const std::vector<PieceSet4>& sample){
    // SC（スート圧縮型）のテスト
    uint64_t time[4] = {0};
    // タイプ別
    for(const PieceSet4& ps : sample){
        for(PieceType pt = PIECE_TYPE_MIN; pt <= PIECE_TYPE_MAX; ++pt){
            const TypePieces4 tp = ps[pt];
            const BitArray64<4> pqr = (uint64_t)convQR_PQR(tp);
            cl.start();
            BitArray64<4> test = (uint64_t)convPQR_SC(pqr);
            time[0] += cl.restart();
            BitArray64<4> ans = convPQR_SC_slow(pqr);
            time[1] += cl.stop();
            if(test != ans){
                cerr << "inconsistent PQR -> SC conversion!" << endl;
                cerr << tp << " : " << test << " <-> " << ans << endl;
                return -1;
            }
        }
    }
    // 一括
    for(const PieceSet4& ps : sample){
        const PieceSet4 pqr = convQR_PQR(ps);
        cl.start();
        PieceSet4 all = convPQR_SC(pqr);
        time[2] += cl.restart();
        PieceSet4 each = convPQR_SC_each(pqr);
        time[3] += cl.stop();
        if(all != each){
            cerr << "inconsistent PQR -> SC conversion!" << endl;
            cerr << ps << " : " << all << " <-> " << each << endl;
            return -1;
        }
    }
    
    cerr << "pqr -> sc (type) test : " << time[0] / (sample.size() * (int)N_PIECE_TYPES) << " clock" << endl;
    cerr << "pqr -> sc (type) ans  : " << time[1] / (sample.size() * (int)N_PIECE_TYPES) << " clock" << endl;
    cerr << "pqr -> sc (full) all  : " << time[2] / sample.size() << " clock" << endl;
    cerr << "pqr -> sc (full) each : " << time[3] / sample.size() << " clock" << endl;
    return 0;
}

/*int testENR(const std::vector<Cards>& sample){
    // ENR(ランクにN枚以上存在)のテスト N = 1 ~ 3
    uint64_t time[6] = {0};
    for(Cards c : sample){
        cl.start();
        BitArray64<4> test1 = convCards_ER(c);
        time[0] += cl.restart();
        BitArray64<4> ans1 = convCards_ENR_slow(c, 1);
        time[1] += cl.stop();
        
        if(test1 != ans1){
            cerr << "inconsistent Cards -> E1R conversion!" << endl;
            cerr << OutCards(c) << " : " << test1 << " <-> " << ans1 << endl;
            return -1;
        }
        
        cl.start();
        BitArray64<4> test3 = convCards_3R(c);
        time[6] += cl.restart();
        BitArray64<4> ans3 = convCards_NR_slow(c, 3);
        time[7] += cl.stop();
        
        if(test0 != ans0){
            cerr << "inconsistent Cards -> E3R conversion!" << endl;
            cerr << OutCards(c) << " : " << test3 << " <-> " << ans3 << endl;
            return -1;
        }
    }
    for(int n = 1; n < 4; ++n){
        cerr << "c -> e" << n << "r test : " << time[(n - 1) * 2] << " clock" << endl;
        cerr << "c -> e" << n << "r ans  : " << time[(n - 1) * 2 + 1] << " clock" << endl;
    }
    return 0;
}

int testNR(const std::vector<Cards>& sample){
    // NR(ランクにN枚存在)のテスト N = 0 ~ 4
    uint64_t time[10] = {0};
    for(Cards c : sample){
        cl.start();
        BitArray64<4> test0 = convCards_0R(c);
        time[0] += cl.restart();
        BitArray64<4> ans0 = convCards_NR_slow(c, 0);
        time[1] += cl.stop();
        
        if(test0 != ans0){
            cerr << "inconsistent Cards -> 0R conversion!" << endl;
            cerr << OutCards(c) << " : " << test0 << " <-> " << ans0 << endl;
            return -1;
        }
        
        cl.start();
        BitArray64<4> test1 = convCards_1R(c);
        time[2] += cl.restart();
        BitArray64<4> ans1 = convCards_NR_slow(c, 1);
        time[3] += cl.stop();
        
        if(test1 != ans1){
            cerr << "inconsistent Cards -> 1R conversion!" << endl;
            cerr << OutCards(c) << " : " << test1 << " <-> " << ans1 << endl;
            return -1;
        }
        
        cl.start();
        BitArray64<4> test2 = convCards_2R(c);
        time[4] += cl.restart();
        BitArray64<4> ans2 = convCards_NR_slow(c, 2);
        time[5] += cl.stop();
        
        if(test2 != ans2){
            cerr << "inconsistent Cards -> 2R conversion!" << endl;
            cerr << OutCards(c) << " : " << test2 << " <-> " << ans2 << endl;
            return -1;
        }
        
        cl.start();
        BitArray64<4> test3 = convCards_3R(c);
        time[6] += cl.restart();
        BitArray64<4> ans3 = convCards_NR_slow(c, 3);
        time[7] += cl.stop();
        
        if(test3 != ans3){
            cerr << "inconsistent Cards -> 3R conversion!" << endl;
            cerr << OutCards(c) << " : " << test3 << " <-> " << ans3 << endl;
            return -1;
        }
        
        cl.start();
        BitArray64<4> test4 = convCards_FR(c);
        time[8] += cl.restart();
        BitArray64<4> ans4 = convCards_NR_slow(c, 4);
        time[9] += cl.stop();
        
        if(test4 != ans4){
            cerr << "inconsistent Cards -> 4R conversion!" << endl;
            cerr << OutCards(c) << " : " << test4 << " <-> " << ans4 << endl;
            return -1;
        }
    }
    for(int n = 0; n <= 4; ++n){
        cerr << "c -> " << n << "r test : " << time[n * 2] << " clock" << endl;
        cerr << "c -> " << n << "r ans  : " << time[n * 2 + 1] << " clock" << endl;
    }
    return 0;
}
*/
int outputStructureSize(){
    cerr << "sizeof Player           = " << sizeof(Player) << endl;
    cerr << "sizeof MatchType        = " << sizeof(MatchType) << endl;
    cerr << "sizeof PieceType        = " << sizeof(PieceType) << endl;
    cerr << "sizeof Wind             = " << sizeof(Wind) << endl;
    cerr << "sizeof Dragon           = " << sizeof(Dragon) << endl;
    cerr << "sizeof Rank             = " << sizeof(Rank) << endl;
    cerr << "sizeof Piece            = " << sizeof(Piece) << endl;
    cerr << "sizeof ExtPiece         = " << sizeof(ExtPiece) << endl;
    cerr << "sizeof PieceIndex       = " << sizeof(PieceIndex) << endl;
    cerr << "sizeof Score            = " << sizeof(Score) << endl;
    cerr << "sizeof Meld             = " << sizeof(Meld) << endl;
    cerr << "sizeof TurnAction       = " << sizeof(TurnAction) << endl;
    cerr << "sizeof ResponseAction   = " << sizeof(ResponseAction) << endl;
    cerr << "sizeof TypePiecesMin    = " << sizeof(TypePiecesMin) << endl;
    cerr << "sizeof TypePieces3      = " << sizeof(TypePieces3) << endl;
    cerr << "sizeof TypePieces4      = " << sizeof(TypePieces4) << endl;
    cerr << "sizeof PieceExistance   = " << sizeof(PieceExistance) << endl;
    cerr << "sizeof PieceSetMin      = " << sizeof(PieceSetMin) << endl;
    cerr << "sizeof PieceSet3        = " << sizeof(PieceSet3) << endl;
    cerr << "sizeof PieceSet4        = " << sizeof(PieceSet4) << endl;
    cerr << "sizeof ExtPieceSet4     = " << sizeof(ExtPieceSet4) << endl;
    cerr << "sizeof Hand             = " << sizeof(Hand) << endl;
    cerr << "sizeof Wall             = " << sizeof(Wall) << endl;
    cerr << "sizeof World            = " << sizeof(World) << endl;
    cerr << "sizeof Field            = " << sizeof(Field) << endl;
    cerr << "sizeof TurnRecord       = " << sizeof(TurnRecord) << endl;
    cerr << "sizeof GameRecord       = " << sizeof(GameRecord) << endl;
    cerr << "sizeof MatchRecord      = " << sizeof(MatchRecord) << endl;
    return 0;
}

int outputMask(){
    cerr << "NaivePieceSet4" << endl;
    cerr << "all" << endl;
    cerr << PIECE_SET_ALL << endl;
    cerr << "all pqr" << endl;
    cerr << SET_PQR_ALL << endl;
    cerr << "all sc" << endl;
    cerr << SET_SC_ALL << endl;
    cerr << "mask" << endl;
    cerr << B4_MASK << endl;
    cerr << B8_MASK << endl;
    cerr << B16_MASK << endl;
    cerr << B32_MASK << endl;
    cerr << B64_MASK << endl;
    cerr << B128_MASK << endl;
    return 0;
}

int main(int argc, char* argv[]){
    
    outputStructureSize();
    outputMask();
    
    XorShift64 dice;
    std::vector<PieceSet> sample;
    dice.srand((unsigned int)time(NULL));
    
    cerr << dice.rand() << endl;
    
    sample.reserve(50000);
    /*for(int i = 0; i < 50000; ++i){
        PieceSet ps;
        ps.clear();
        while(ps.sum() < N_DEALT_PIECES){
            PieceIndex pi = PieceIndex(dice.rand() % N_PIECES);
            Piece p = toPiece(pi);
            if(dice.rand() % N_ONE_PIECE >= ps[p]){
                ps += p;
                //cerr << ps << " ";
                //cerr << ps.sum() << endl;
            }
        }
        cerr << i << " " << ps << endl;
        sample.emplace_back(ps);
    }*/
    for(int i = 0; i < 50000; ++i){
        PieceSet filled, ps;
        ps.clear();
        filled.fill();
        BitArray32<8, N_PIECE_TYPES> typeSum = TYPE_SUM_ALL;
        int sum = N_ALL_PIECES;
        for(int j = 0; j < N_DEALT_PIECES; ++j){
            ASSERT(typeSum.sum() == sum, cerr << typeSum << " " << sum << endl;);
            Piece p = dealPiece(filled, typeSum, sum, &dice);
            ps += p;
            filled -= p;
            typeSum.subtr(toPieceType(p), 1);
            sum -= 1;
        }
        sample.push_back(ps);
    }
    
    if(testContaining(sample)){
        cerr << "failed containing test." << endl;
        return -1;
    }
    cerr << "passed containing test." << endl << endl;
    
    if(testCounting(sample)){
        cerr << "failed counting test." << endl;
        return -1;
    }
    cerr << "passed counting test." << endl << endl;
    
    /*if(testSuitSuits()){
        cerr << "failed (Suit, Suits) test." << endl;
        return -1;
    }
    cerr << "passed (Suit, Suits) test." << endl;
    
    if(test2Suits()){
        cerr << "failed (Suits Suits) test." << endl;
        return -1;
    }
    cerr << "passed (Suits Suits) test." << endl;
    
    if(testSuitsSuits()){
        cerr << "failed (Suits, Suits) test." << endl;
        return -1;
    }
    cerr << "passed (Suits, Suits) test." << endl << endl;
    
    if(testRankCards()){
        cerr << "failed Rank -> Cards test." << endl;
        return -1;
    }
    cerr << "passed Rank -> Cards test." << endl << endl;
    
    if(testRank4xCards()){
        cerr << "failed Rank4x -> Cards test." << endl;
        return -1;
    }
    cerr << "passed Rank4x -> Cards test." << endl << endl;
    
    if(testSuitCards()){
        cerr << "failed Suits -> Cards test." << endl;
        return -1;
    }
    cerr << "passed Suits -> Cards test." << endl << endl;
    
    if(testQR(sample)){
        cerr << "failed QR test." << endl;
        return -1;
    }
    cerr << "passed QR test." << endl << endl;
    */
    
    if(testPQR(sample)){
        cerr << "failed PQR test." << endl;
        return -1;
    }
    cerr << "passed PQR test." << endl << endl;
    
    if(testSC(sample)){
        cerr << "failed SC test." << endl;
        return -1;
    }
    cerr << "passed SC test." << endl << endl;
    /*
    if(testNR(sample)){
        cerr << "failed NR test." << endl;
        return -1;
    }
    cerr << "passed NR test." << endl << endl;
    
    if(testENR(sample)){
        cerr << "failed ENR test." << endl;
        return -1;
    }
    cerr << "passed ENR test." << endl << endl;
    */
    return 0;
}
