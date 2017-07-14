/*
 logic_test.cc
 Katsuki Ohto
 */

// 麻雀応用演算のテスト

#include "../mahjong.hpp"

using namespace Mahjong;

Clock cl;

template<class dice_t>
int testDealNaive1P(const std::vector<PieceSet>& samples, dice_t *const pdice){
    uint64_t clSum[4] = {0};
    PieceSet dummy;
    dummy.clear();
    // full から 1つ
    for(int i = 0; i < (int)samples.size(); ++i){
        cl.start();
        Piece p = dealPiece(PIECE_SET_ALL, TYPE_SUM_ALL, N_ALL_PIECES, pdice);
        clSum[0] += cl.stop();
        dummy += p;
    }
    // random から1つ
    for(const PieceSet& ps : samples){
        cl.start();
        Piece p = dealPiece(ps, pdice);
        clSum[1] += cl.stop();
        dummy += p;
    }
    cerr << dummy << endl;
    cerr << "deal-1p full test : " << clSum[0] / samples.size() << " clock" << endl;
    cerr << "deal-1p rdno test : " << clSum[1] / samples.size() << " clock" << endl;
    return 0;
}
template<class dice_t>
int testDealExt1P(const std::vector<ExtPieceSet>& samples, dice_t *const pdice){
    uint64_t clSum[4] = {0};
    ExtPieceSet dummy;
    dummy.clear();
    // full から 1つ
    for(int i = 0; i < (int)samples.size(); ++i){
        cl.start();
        ExtPiece ep = dealExtPiece(EXT_PIECE_SET_ALL, TYPE_SUM_ALL, N_ALL_PIECES, pdice);
        clSum[0] += cl.stop();
        dummy += ep;
    }
    // random から1つ
    for(const ExtPieceSet& eps : samples){
        cl.start();
        ExtPiece ep = dealExtPiece(eps, pdice);
        clSum[1] += cl.stop();
        dummy += ep;
    }
    cerr << (dummy & PQR256_1) << dummy.red() << endl;
    cerr << "deal-1ep full test : " << clSum[0] / samples.size() << " clock" << endl;
    cerr << "deal-1ep rdno test : " << clSum[1] / samples.size() << " clock" << endl;
    return 0;
}

int testMinimumSteps(const std::vector<Hand>& samples){
    // シャンテン数と受け入れ牌の計算
    uint64_t clSum[2] = {0};
    std::vector<uint64_t> clv[2];
    for(auto& v : clv){
        v.reserve(samples.size());
    }
    std::vector<uint64_t> clv1; clv1.reserve(samples.size());
    int msSum[2] = {0};
    uint64_t acSum[2] = {0};
    for(const Hand& hand : samples){
        PieceExistance acceptable;
        cl.start();
        int ms = calcMinimumSteps(hand, &acceptable);
        uint64_t tm = cl.stop();
        clv[0].push_back(tm);
        clSum[0] += tm;
        msSum[0] += ms;
        acSum[0] += acceptable;
        
        /*cerr << acceptable << " ";
        PieceExistance acceptable1(0);
        calcAcceptableBits(hand.piece, 0, &acceptable1);
        cerr << acceptable1 << endl;
        
        if(acceptable != acceptable1){
            getchar();
        }*/
    }
    for(const Hand& hand : samples){
        PieceExistance acceptable;
        cl.start();
        int ms = calcAcceptableBits(hand.piece, 0, &acceptable);
        uint64_t tm = cl.stop();
        clv[1].push_back(tm);
        clSum[1] += tm;
        msSum[1] += ms;
        acSum[1] += acceptable;
    }
    for(auto& v : clv){
        std::sort(v.begin(), v.end());
    }
    cerr << "minumum steps test : " << clSum[0] / samples.size() << " clock (mid " << clv[0][samples.size() / 2] << ")" << endl;
    cerr << "minumum steps ans  : " << clSum[1] / samples.size() << " clock (mid " << clv[1][samples.size() / 2] << ")" << endl;
    if(msSum[0] != msSum[1]){
        cerr << "failed to calculate minimum steps." << endl;
        return -1;
    }
    if(acSum[0] != acSum[1]){
        cerr << "failed to calculate acceptable pieces." << endl;
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]){
    
    std::vector<PieceSet4> randomPs;
    std::vector<ExtPieceSet4> randomEps;
    std::vector<Hand> randomHand;
    XorShift64 dice;
    dice.srand((unsigned int)time(NULL));
    
    randomPs.reserve(50000);
    randomEps.reserve(50000);
    randomHand.reserve(50000);
    for(int i = 0; i < 50000; ++i){
        ExtPieceSet filled, eps;
        eps.clear();
        filled.fill();
        BitArray32<8, N_PIECE_TYPES> typeSum = TYPE_SUM_ALL;
        int sum = N_ALL_PIECES;
        for(int j = 0; j < N_DEALT_PIECES; ++j){
            ASSERT(typeSum.sum() == sum, cerr << typeSum << " " << sum << endl;);
            ExtPiece ep = dealExtPiece(filled, typeSum, sum, &dice);
            eps += ep;
            filled -= ep;
            typeSum.subtr(toPieceType(ep), 1);
            sum -= 1;
        }
        //cerr << i << " " << eps << endl;
        randomPs.push_back(eps);
        randomEps.push_back(eps);
        Hand hand;
        hand.set(eps);
        randomHand.push_back(hand);
    }
    
    testMinimumSteps(randomHand);
    testDealNaive1P(randomPs, &dice);
    testDealExt1P(randomEps, &dice);
    
    return 0;
}
