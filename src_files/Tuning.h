
/****************************************************************************************************
 *                                                                                                  *
 *                                     Koivisto UCI Chess engine                                    *
 *                           by. Kim Kahre, Finn Eggers and Eugenio Bruno                           *
 *                                                                                                  *
 *                 Koivisto is free software: you can redistribute it and/or modify                 *
 *               it under the terms of the GNU General Public License as published by               *
 *                 the Free Software Foundation, either version 3 of the License, or                *
 *                                (at your option) any later version.                               *
 *                    Koivisto is distributed in the hope that it will be useful,                   *
 *                  but WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 *                   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                  *
 *                           GNU General Public License for more details.                           *
 *                 You should have received a copy of the GNU General Public License                *
 *                 along with Koivisto.  If not, see <http://www.gnu.org/licenses/>.                *
 *                                                                                                  *
 ****************************************************************************************************/

#ifndef KOIVISTO_TUNING_H
#define KOIVISTO_TUNING_H

#include "Bitboard.h"
#include "Util.h"
#include "eval.h"

#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

namespace tuning {

struct TrainingEntry {
    Board  board;
    double target;

    const Board& getBoard() const { return board; }
    void         setBoard(const Board& board) { TrainingEntry::board = board; }
    double       getTarget() const { return target; }
    void         setTarget(double target) { TrainingEntry::target = target; }
};

extern std::vector<TrainingEntry> training_entries;

inline double sigmoid(double s, double K) { return (double) 1 / (1 + exp(-K * s / 400)); }

inline double sigmoidPrime(double s, double K) {
    double ex = exp(-s * K / 400);
    return (K * ex) / (400 * (ex + 1) * (ex + 1));
}

void loadPositionFile(const std::string& path, int count, int start = 0);

/**
 * does blackbox tuning on the given data. This is usually inefficient.
 * @param evaluator
 * @param K
 * @return
 */
double optimiseBlackBox(double K, float* params, int paramCount, float lr, int threads = 1);

double optimisePSTBlackBox(double K, EvalScore* evalScore, int count, int lr, int threads = 1);

double optimisePSTBlackBox(double K, EvalScore** evalScore, int count, int lr, int threads = 1);

/**
 * computes the error of the evaluator on the given set
 */
double computeError(double K, int threads = 1);

/**
 * computes the K value
 * @param evaluator
 * @param initK
 * @param rate
 * @param deviation
 * @return
 */
double computeK(double initK, double rate, double deviation, int threads = 1);

/**
 * computes the average time for evaluation
 */
void evalSpeed();

}    // namespace tuning

#endif    // KOIVISTO_TUNING_H
