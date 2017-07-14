/*
 table_converter.cc
 Katsuki Ohto
 */

// 水上さん作のシャンテン数、受け入れ数計算用テーブルを自分が使いやすいように変形して保存

#include "../mahjong.hpp"

using namespace Mahjong;

std::mt19937 mt;

uint32_t myMinimumStepsInfoTable[ipow(N_ONE_PIECE + 1, N_RANKS)];
uint64_t myAcceptableInfoTable[ipow(N_ONE_PIECE + 1, N_RANKS) * 2];

std::array<int, N_RANKS * N_ONE_PIECE + 1> u;
std::array<uint32_t, N_RANKS> hash;

const int keySize = 18;
std::array<uint16_t, (1 << keySize)> hashCount;
int doubling;
uint64_t maxA1 = 0;

uint64_t reverseAcceptableBits(uint64_t v){
    BitArray64<9> t = v;
    BitArray64<9> a = 0;
    for(int i = 0; i < 7; ++i){
        a.set(i, reverseBits64(t[i]) >> (64 - 9));
    }
    return a;
}

void calcValue(int r, int cnt, int mizu3b, int mizu5x, int my5x, int myHash){
    if(r == (int)N_RANKS){
        myMinimumStepsInfoTable[my5x] = shantenTable[mizu3b];
        //myAcceptableInfoTable[my5x * 2 + 0] = acceptableTable0[mizu5x];
        //myAcceptableInfoTable[my5x * 2 + 1] = acceptableTable1[mizu5x];
        myAcceptableInfoTable[my5x * 2 + 0] = reverseAcceptableBits(acceptableTable0[mizu5x]);
        myAcceptableInfoTable[my5x * 2 + 1] = reverseAcceptableBits(acceptableTable1[mizu5x]);
        maxA1 = max(maxA1, acceptableTable1[mizu5x]);
        
        for(int i = cnt; i < u.size(); ++i){
            u[i] += 1;
        }
        if(cnt <= N_DEALT_PIECES){
            uint32_t key = myHash & ((1 << keySize) - 1);
            if(hashCount[key] > 0){
                doubling += 1;
            }
            hashCount[key] += 1;
        }
    }else{
        if(1 || r != RANK_RED){
            for(int i = 0; i <= N_ONE_PIECE; ++i){
                calcValue(r + 1, cnt + i,
                          mizu3b | (i << (3 * (N_RANKS - 1 - r))),
                          mizu5x + i * ipow5Table[N_RANKS - 1 - r],
                          my5x   + i * ipow5Table[r],
                          myHash + i * hash[r]);
            }
        }else{
            for(int i = 0; i <= N_ONE_PIECE - 1; ++i){
                calcValue(r + 1, cnt + i,
                          mizu3b | (i << (3 * (N_RANKS - 1 - r))),
                          mizu5x + i * ipow5Table[N_RANKS - 1 - r],
                          my5x   + i * ipow5Table[r],
                          myHash + i * hash[r]);
            }
            // 赤
            for(int i = 0; i <= 1; ++i){
                calcValue(r + 1, cnt + i,
                          mizu3b | (i << (3 * (N_RANKS - 1 - r))),
                          mizu5x + i * ipow5Table[N_RANKS - 1 - r],
                          my5x   + i * ipow5Table[r],
                          myHash + i * hash[r]);
            }
        }
    }
}

void countPatterns(){
    doubling = 0;
    u.fill(0);
    hashCount.fill(0);
    
    for(int i = 0; i < N_RANKS; ++i){
        hash[i] = mt();
    }
    
    calcValue(0, 0, 0, 0, 0, 0);
    
    for(int i = 0; i < u.size(); ++i){
        cerr << i << " " << u[i] << endl;
    }
    cerr << "d = " << doubling << endl;
    cerr << "a1 = " << maxA1 << endl;
}

int main(int argc, char *argv[]){
    
    memset(myMinimumStepsInfoTable, 0, sizeof(myMinimumStepsInfoTable));
    memset(myAcceptableInfoTable, 0, sizeof(myAcceptableInfoTable));
    
    mt.seed((unsigned int)time(NULL));
    
    countPatterns();
    
    const char *opath0 = "./data/minimumStepsInfoTable.bin";
    const char *opath1 = "./data/acceptableInfoTable.bin";
    
    /*FILE *pf;
    pf = fopen(opath0, "wb");
    fwrite(myMinimumStepsInfoTable, sizeof(myMinimumStepsInfoTable), 1, pf);
    fclose(pf);
    pf = fopen(opath1, "wb");
    fwrite(myAcceptableInfoTable, sizeof(myAcceptableInfoTable), 1, pf);
    fclose(pf);*/
    
    std::string op0 = "./data/minimumStepsInfoTable.dat";
    std::string op1 = "./data/acceptableInfoTable.dat";
    
    std::ofstream ofs0(op0);
    for(int i = 0; i < ipow(5, N_RANKS); ++i){
        if(i != 0){
            ofs0 << endl;
        }
        ofs0 << myMinimumStepsInfoTable[i];
    }
    std::ofstream ofs1(op1);
    for(int i = 0; i < ipow(5, N_RANKS) * 2; ++i){
        if(i != 0){
            ofs1 << endl;
        }
        ofs1 << myAcceptableInfoTable[i];
    }
    
    return 0;
}
