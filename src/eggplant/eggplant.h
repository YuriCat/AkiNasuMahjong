/*
 eggplant.h
 Katsuki Ohto
 */

#ifndef MAHJONG_EGGPLANT_EGGPLANT_H_
#define MAHJONG_EGGPLANT_EGGPLANT_H_

// 思考部本体でだけでなくテストコード等でも読み込むコードとデータのヘッダ

#include "../mahjong.hpp"

namespace Mahjong{
    namespace Eggplant{
        namespace Settings{
            // パラメータ等の可変設定
            // 本番用(match)ビルドでは全て定数として埋め込む
            // こちらに値を直書きしてもいいが途中でデフォルトに戻す操作を可能にするため分けている
            
            // 行動評価点による事前分布設定
            MATCH_CONST double rootTurnPriorCoef = 4;
            MATCH_CONST double rootTurnPriorExponent = 0.6;
            
            MATCH_CONST double rootResponsePriorCoef = 4;
            MATCH_CONST double rootResponsePriorExponent = 0.6;
            
            // 手札推定設定
            MATCH_CONST double estimationTemperatureTurn = SIMULATION_TEMPERATURE_TURN;
            MATCH_CONST double estimationTemperatureResponse = SIMULATION_TEMPERATURE_RESPONSE;
            
            // シミュレーション設定
            MATCH_CONST double simulationTemperatureTurn = SIMULATION_TEMPERATURE_TURN;
            MATCH_CONST double simulationTemperatureResponse = SIMULATION_TEMPERATURE_RESPONSE;
            
            MATCH_CONST double simulationAmplifyCoef = SIMULATION_AMPLIFY_COEF;
            MATCH_CONST double simulationAmplifyExponent = SIMULATION_AMPLIFY_EXPONENT;
            
            MATCH_CONST double temperatureTurn = TEMPERATURE_TURN;
            MATCH_CONST double temperatureResponse = TEMPERATURE_RESPONSE;
            
            MATCH_CONST int NThreads = N_MAX_THREADS;
            
            MATCH_CONST Selector simulationSelector = SIMULATION_SELECTOR;
            
            MATCH_CONST DealType monteCarloDealType = MONTECARLO_DEAL_TYPE;
            
            // シミュレーション中の相手モデル利用設定
#ifdef MODELING_PLAY
            MATCH_CONST bool simulationTurnModel = true;
#else
            MATCH_CONST bool simulationTurnModel = false;
#endif
            // 相手モデル化のバイアス設定
            MATCH_CONST double playerBiasAttenuateRate = 0.9997;
            MATCH_CONST double playerBiasPriorSize = 1000;
            MATCH_CONST double playerBiasCoef = 0.65;
        }
    }
}

#endif // MAHJONG_EGGPLANT_EGGPLANT_H_
