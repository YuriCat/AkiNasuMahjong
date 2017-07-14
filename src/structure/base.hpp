/*
 base.hpp
 Katsuki Ohto
 */

// 麻雀基本的定義

#ifndef MAHJONG_STRUCTURE_BASE_HPP_
#define MAHJONG_STRUCTURE_BASE_HPP_

#include <unistd.h>
#include <sys/time.h>
#include <immintrin.h>
#include <emmintrin.h>

#include <cstring>
#include <ctime>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <array>
#include <vector>
#include <queue>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <string>
#include <bitset>
#include <numeric>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#endif

// 設定

#if !defined(MINIMUM)
#define USE_ANALYZER
#endif

// 基本的定義、ユーティリティ
// 使いそうなものは全て読み込んでおく

#include "../../../CppCommon/src/util/io.hpp"
#include "../../../CppCommon/src/util/string.hpp"
#include "../../../CppCommon/src/util/xorShift.hpp"
#include "../../../CppCommon/src/util/random.hpp"
#include "../../../CppCommon/src/util/bitOperation.hpp"
#include "../../../CppCommon/src/util/container.hpp"
#include "../../../CppCommon/src/util/arrays.h"
#include "../../../CppCommon/src/util/longBits.hpp"
#include "../../../CppCommon/src/util/bitArray.hpp"
#include "../../../CppCommon/src/util/bitSet.hpp"
#include "../../../CppCommon/src/util/longBitSet.hpp"
#include "../../../CppCommon/src/util/pack.hpp"
#include "../../../CppCommon/src/util/bitPartition.hpp"
#include "../../../CppCommon/src/util/selection.hpp"
#include "../../../CppCommon/src/util/math.hpp"
#include "../../../CppCommon/src/util/pd.hpp"
#include "../../../CppCommon/src/util/search.hpp"
#include "../../../CppCommon/src/hash/HashFunc.hpp"
#include "../../../CppCommon/src/hash/HashBook.hpp"
#include "../../../CppCommon/src/util/multiArmedBandit.hpp"
#include "../../../CppCommon/src/util/softmaxPolicy.hpp"
#include "../../../CppCommon/src/util/lock.hpp"

namespace Mahjong{
    
    /**************************基本的定義**************************/
    
    // 試合タイプ
    enum MatchType{
        SINGLE = 0, // 東風
        DOUBLE, // 半荘
        MATCH_TYPE_NONE = -1
    };
    
    ENABLE_ARITHMETIC_OPERATORS(MatchType)
    
    // プレーヤー
    using Player = int;
    constexpr int N_PLAYERS = 4;
    constexpr int NONE_PLAYER = -1;
    constexpr int N_WON_PLAYERS_DRAW = 3; // この人数より多く同時に上がりを宣言すると途中流局
    
    Player nextPlayer(Player pn)noexcept{
        return unsigned(pn + 1) % N_PLAYERS;
    }
    constexpr bool examPlayerNum(Player p)noexcept{
        return ((unsigned int)p < N_PLAYERS);
    }
    
    // 牌の数
    constexpr int N_ONE_PIECE = 4;
    constexpr int N_WALL_PIECES = 4 * 34;
    constexpr int N_DEALT_PIECES = 13; // 一人に配られる牌
    constexpr int N_DISCARD_PIECES = 24; // 最大の捨て牌の数
    constexpr int N_LEFT_PIECES = 14; // 終了時にこれだけ残す
    
    // 手数
    constexpr int N_TURNS = N_WALL_PIECES - N_DEALT_PIECES * N_PLAYERS - N_LEFT_PIECES;
    constexpr int NONE_TURN = -1;
    
    // カンの回数
    constexpr int N_MAX_KONGS = 4;
    
    // ドラ
    constexpr int N_MAX_DORAS = 1 + N_MAX_KONGS; // 初期1 + カンドラ4
    constexpr int WALL_INDEX_ORIGINAL_DORA_MARKER = N_WALL_PIECES - N_MAX_KONGS - 2; // 嶺上牌を除く
    
    // 通常ツモ牌の範囲
    constexpr int WALL_INDEX_START = N_DEALT_PIECES * N_PLAYERS;
    constexpr int WALL_INDEX_LAST  = N_WALL_PIECES - N_LEFT_PIECES - 1;
    
    constexpr int wallIndexDoraMarker(unsigned int n)noexcept{
        // n 番目に生成されるドラの位置(初期ドラを0とする)
        return WALL_INDEX_ORIGINAL_DORA_MARKER - n * 2;
    }
    constexpr int wallIndexTurn(unsigned int t)noexcept{
        // ターン t でのツモの位置
        return WALL_INDEX_START + t;
    }
    constexpr int wallIndexTurnAfterKong(unsigned int k)noexcept{
        // k 回目(0 <= k < N_MAX_KONGS)のツモの位置
        // ※実際には上の牌から取るが、下の牌から取ることにしても問題ないのでそうする
        return N_WALL_PIECES - 1 - k;
    }
    
    constexpr int N_PONG_PIECES = 3;
    constexpr int N_KONG_PIECES = 4;
    constexpr int N_CHOW_PIECES = 3;
    
    constexpr int N_MIN_OPENED_GROUP_PIECES = cmin(N_PONG_PIECES, N_KONG_PIECES);
    
    /**************************プレーヤー状態**************************/
    
    enum PlayerStatus{
        CONCEALED = 0, // 門前状態
        IN_REACH,      // リーチ状態
    };
    
    /**************************試合の状態**************************/
    
    // 試合中と試合後の状態を同じ系列で扱うことで状態遷移を書きやすくする
    
    enum FieldStatus{
        // ゲーム開始状態
        STATUS_MATCH_START,      // 対戦開始
        STATUS_GAME_START,       // 局開始
        
        // ゲーム中状態
        STATUS_TURN_START,       // ターン開始
        STATUS_DRAWN,            // ツモ後
        STATUS_DRAW_WIN,         // ツモ上がり時
        STATUS_TURN_PROCESS,     // 手番行動処理時
        STATUS_ADDED,            // 加槓後
        STATUS_ADDED_PROCESS,    // 槍槓行動処理時
        STATUS_ADDED_WIN,        // 槍槓上がり時
        STATUS_DISCARDED,        // 反応行動選択時
        STATUS_OTHER_RESPONSE,   // 主観的な反応行動の
        STATUS_RESPONSE_PROCESS, // 反応行動処理時
        STATUS_RESPONSE_WIN,     // 打牌に対してのロン上がり時
        
        // 終局状態
        STATUS_DRAW_EXHAUSTED,   // 通常流局
        STATUS_DRAW_9T9P,        // 九種九牌
        
        STATUS_DRAW
        = STATUS_DRAW_EXHAUSTED
        | STATUS_DRAW_9T9P,
        
        STATUS_WIN
        = STATUS_DRAW_WIN
        | STATUS_ADDED_WIN
        | STATUS_RESPONSE_WIN
    };
    
    
    /**************************風**************************/
    
    // プレーヤー番号と自分風が一致するはずなのでプレーヤー番号の意味も兼ねる
    
    enum Wind : unsigned int{
        WIND_E = 0, WIND_S, WIND_W, WIND_N,
        WIND_NONE = (unsigned int)-1,
        WIND_MIN = WIND_E,
        WIND_MAX = WIND_N,
        N_WINDS = WIND_MAX + 1,
    };
    
    ENABLE_ARITHMETIC_OPERATORS(Wind)
    
    constexpr bool examWind(Wind w)noexcept{
        return ((unsigned int)w < N_WINDS);
    }
    
    constexpr Wind toNextWind(Wind w)noexcept{
        return static_cast<Wind>((w + 1) % N_WINDS);
    }
    
    const std::string windChar = "ESWN";
    
    Wind toWind(const std::string& str){
        if(str.size() != 1){ return WIND_NONE; }
        std::string::size_type index = windChar.find_first_of(str);
        if(index == std::string::npos){
            return WIND_NONE;
        }
        return static_cast<Wind>(index + int(WIND_MIN));
    }
    
    char toWindChar(Wind w){
        return examWind(w) ? windChar[w] : '-';
    }
    
    std::ostream& operator<<(std::ostream& ost, const Wind& w){
        ost << toWindChar(w);
        return ost;
    }
    
    /**************************三元牌**************************/
    
    enum Dragon : unsigned int{
        DRAGON_P = 0, DRAGON_F, DRAGON_C,
        DRAGON_NONE = (unsigned int)-1,
        DRAGON_MIN = DRAGON_P,
        DRAGON_MAX = DRAGON_C,
        N_DRAGONS = DRAGON_MAX + 1,
    };
    
    ENABLE_ARITHMETIC_OPERATORS(Dragon)
    
    constexpr bool examDragon(Dragon d)noexcept{
        return ((unsigned int)d < N_DRAGONS);
    }
    
    constexpr Dragon toNextDragon(Dragon d)noexcept{
        return static_cast<Dragon>((d + 1) % N_DRAGONS);
    }
    
    const std::string dragonChar = "PFC";
    
    Dragon toDragon(const std::string& str){
        if(str.size() != 1){ return DRAGON_NONE; }
        std::string::size_type index = dragonChar.find_first_of(str);
        if(index == std::string::npos){
            return DRAGON_NONE;
        }
        return static_cast<Dragon>(index + int(DRAGON_MIN));
    }
    
    char toDragonChar(Dragon d){
        if((unsigned int)d >= dragonChar.size()){
            return '-';
        }
        return dragonChar[d];
    }
    
    /**************************牌の種類**************************/
    
    enum PieceType : unsigned int{
        // 牌の種類
        CHARACTER = 0, CIRCLE, BAMBOO, HONOR,
        PIECE_TYPE_NONE = (unsigned int)-1,
        PIECE_TYPE_MIN = CHARACTER,
        PIECE_TYPE_MAX = HONOR,
        PIECE_TYPE_NUMBERS_MIN = CHARACTER,
        PIECE_TYPE_NUMBERS_MAX = BAMBOO,
        N_PIECE_TYPES = PIECE_TYPE_MAX + 1,
        N_NUMBER_PIECE_TYPES = PIECE_TYPE_NUMBERS_MAX - PIECE_TYPE_NUMBERS_MIN + 1,
        N_HONOR_PIECE_TYPES = 1,
    };
    
    ENABLE_ARITHMETIC_OPERATORS(PieceType)
    
    constexpr bool examPieceType(PieceType pt)noexcept{
        return ((unsigned int)pt < N_PIECE_TYPES);
    }
    constexpr bool isNumberType(PieceType pt)noexcept{
        return ((unsigned int)pt <= PIECE_TYPE_NUMBERS_MAX);
    }
    constexpr bool isRedPieceType(PieceType pt)noexcept{
        return isNumberType(pt);
    }
    
    /**************************数牌のランク**************************/
    
    constexpr int N_HONORS = (int)N_WINDS + (int)N_DRAGONS;
    
    enum Rank : unsigned int{
        // 牌の数字
        RANK_1 = 0, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_9,
        RANK_MIN = RANK_1,
        RANK_MAX = RANK_9,
        RANK_RED = RANK_5, // 赤ドラは5だけということにする
        RANK_HONOR_MIN = RANK_MIN,
        RANK_HONOR_MAX = N_HONORS - 1,
        N_RANKS = RANK_MAX + 1,
    };
    
    ENABLE_ARITHMETIC_OPERATORS(Rank)
    
    constexpr bool examRank(Rank r)noexcept{
        return ((unsigned int)r < N_RANKS);
    }
    constexpr bool examHonorRank(Rank r)noexcept{
        return ((unsigned int)r < (unsigned int)N_HONORS);
    }
    
    constexpr Rank toRank(const char rc){
        return static_cast<Rank>(rc - '1' + RANK_1);
    }
    constexpr char toRankChar(Rank r)noexcept{
        return '1' + static_cast<char>(r - RANK_1);
    }
    
    constexpr Rank toNextRank(Rank r)noexcept{
        return static_cast<Rank>((r + 1) % N_RANKS);
    }
    /**************************牌の種類+赤情報**************************/
    
    enum ExtPieceType : unsigned int{
        EXT_PIECE_TYPE_FLAG_RED = 1 << 2,
        EXT_PIECE_TYPE_MASK_NAIVE = (1 << 2) - 1,
        EXT_PIECE_TYPE_NONE = (unsigned int)-1,
    };
    
    ENABLE_ARITHMETIC_OPERATORS(ExtPieceType)
    
    constexpr ExtPieceType toExtPieceType(PieceType pt, bool isRed)noexcept{
        return isRed ? static_cast<ExtPieceType>((static_cast<ExtPieceType>(pt) | EXT_PIECE_TYPE_FLAG_RED))
        : static_cast<ExtPieceType>(pt);
    }
    constexpr PieceType toPieceType(ExtPieceType ept)noexcept{
        return static_cast<PieceType>(ept & EXT_PIECE_TYPE_MASK_NAIVE);
    }
    constexpr bool examExtPieceType(ExtPieceType ept)noexcept{
        return isRedPieceType(toPieceType(ept)) ?
        examPieceType(static_cast<PieceType>(ept & ~EXT_PIECE_TYPE_FLAG_RED)) : examPieceType(static_cast<PieceType>(ept));
    }
}

#endif // MAHJONG_STRUCTURE_BASE_HPP_