/*
 value.hpp
 Katsuki Ohto
 */

// 試合状況からの予測勝率計算

#ifndef MAHJONG_EGGPLANT_VALUE_HPP_
#define MAHJONG_EGGPLANT_VALUE_HPP_

#include "eggplant.h"

namespace Mahjong{
    namespace Eggplant{
        
        /*template<class field_t>
        double calcExpectedWP(const field_t& field){
            // TODO: 現在適当
            return sigmoid(field.myScore() / 10000.0);
        }*/
        
        template<class field_t, class distribution_t>
        void calcExpectedDistribution(const field_t& field, const Player pn, distribution_t *const pdist){
            // TODO: 現在適当
            pdist->fill(0);
            (*pdist)[0] = sigmoid((double)field.score[pn] / 10000.0);
            (*pdist)[N_PLAYERS - 1] = 1 - (*pdist)[0];
        }
        
        template<class distribution_t>
        double distributionToReward(const distribution_t& dist){
            double sum = 0;
            for(int i = 0; i < N_PLAYERS - 1; ++i){ // 最下位は0
                sum += dist[i] * (N_PLAYERS - 1 - i) / (N_PLAYERS - 1);
            }
            return sum;
        }
    }
}

#endif // MAHJONG_EGGPLANT_VALUE_HPP_