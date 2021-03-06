
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

#include "Tuning.h"

#include "Bitboard.h"

#include <iomanip>
#include <thread>

using namespace std;
using namespace tuning;

std::vector<TrainingEntry> tuning::training_entries {};

void tuning::loadPositionFile(const std::string& path, int count, int start) {

    fstream newfile;
    newfile.open(path, ios::in);
    if (newfile.is_open()) {
        string tp;
        int    lineCount = 0;
        int    posCount  = 0;
        while (getline(newfile, tp)) {

            if (lineCount < start) {
                lineCount++;
                continue;
            }

            // finding the first "c" to check where the fen ended
            auto firstC = tp.find_first_of('c');
            auto lastC  = tp.find_last_of('c');
            if (firstC == string::npos || lastC == string::npos) {
                continue;
            }

            // extracting the fen and result and removing bad characters.
            string fen = tp.substr(0, firstC);
            string res = tp.substr(lastC + 2, string::npos);

            fen = trim(fen);
            res = findAndReplaceAll(res, "\"", "");
            res = findAndReplaceAll(res, ";", "");
            res = trim(res);

            TrainingEntry new_entry {Board {fen}, 0};

            // parsing the result to a usable value:
            // assuming that the result is given as : a-b
            if (res.find('-') != string::npos) {
                if (res == "1/2-1/2") {
                    new_entry.target = 0.5;
                } else if (res == "1-0") {
                    new_entry.target = 1;
                } else if (res == "0-1") {
                    new_entry.target = 0;
                } else {
                    continue;
                }
            }
            // trying to read the result as a decimal
            else {
                try {
                    double actualResult = stod(res);
                    new_entry.target    = actualResult;
                } catch (std::invalid_argument& e) { continue; }
            }

            training_entries.push_back(new_entry);

            lineCount++;
            posCount++;

            if (posCount % 10000 == 0) {

                std::cout << "\r" << loadingBar(posCount, count, "Loading data") << std::flush;
            }

            if (posCount >= count)
                break;
        }

        std::cout << std::endl;
        newfile.close();
    }
}

double tuning::optimiseBlackBox(double K, float* params, int paramCount, float lr, int threads) {

    for (int p = 0; p < paramCount; p++) {

        std::cout << "\r  param: " << p << std::flush;

        double er = computeError(K, threads);

        params[p] += lr;
        double erUpper = computeError(K, threads);

        if (erUpper < er)
            continue;

        params[p] -= 2 * lr;
        double erLower = computeError(K, threads);

        if (erLower < er)
            continue;

        params[p] += lr;
    }
    std::cout << std::endl;

    return computeError(K, threads);
}

double tuning::optimisePSTBlackBox(double K, EvalScore* evalScore, int count, int lr, int threads) {
    double er;

    for (int p = 0; p < count; p++) {

        std::cout << "\r  param: " << p << std::flush;

        er = computeError(K, threads);
        //        std::cout << er << std::endl;
        evalScore[p] += M(+lr, 0);
        eval_init();
        //        showScore(M(+lr,0));

        double upper = computeError(K, threads);
        //        std::cout << upper << std::endl;
        if (upper >= er) {
            evalScore[p] += M(-2 * lr, 0);
            eval_init();
            //            showScore(evalScore[p]);

            double lower = computeError(K, threads);

            if (lower >= er) {
                evalScore[p] += M(+lr, 0);
                //                showScore(evalScore[p]);
                eval_init();
            }
        }

        er = computeError(K, threads);
        evalScore[p] += M(0, +lr);
        eval_init();

        upper = computeError(K, threads);
        if (upper >= er) {
            evalScore[p] += M(0, -2 * lr);
            eval_init();

            double lower = computeError(K, threads);

            if (lower >= er) {
                evalScore[p] += M(0, +lr);
                eval_init();
            }
        }
    }
    std::cout << std::endl;
    return er;
}

double tuning::optimisePSTBlackBox(double K, EvalScore** evalScore, int count, int lr, int threads) {
    double er;

    for (int p = 0; p < count; p++) {

        std::cout << "\r  param: " << p << std::flush;

        er = computeError(K, threads);
        *evalScore[p] += M(+lr, 0);
        eval_init();

        double upper = computeError(K, threads);
        if (upper >= er) {
            *evalScore[p] += M(-2 * lr, 0);
            eval_init();

            double lower = computeError(K, threads);

            if (lower >= er) {
                *evalScore[p] += M(+lr, 0);
                eval_init();
            }
        }

        er = computeError(K, threads);
        *evalScore[p] += M(0, +lr);
        eval_init();

        upper = computeError(K, threads);
        if (upper >= er) {
            *evalScore[p] += M(0, -2 * lr);
            eval_init();

            double lower = computeError(K, threads);

            if (lower >= er) {
                *evalScore[p] += M(0, +lr);
                eval_init();
            }
        }
    }
    std::cout << std::endl;
    return er;
}

double tuning::computeError(double K, int threads) {
    double score = 0;

    auto eval_part = [](int threadId, int threads, double K, double* resultTarget) {
        int start = training_entries.size() * threadId / threads;
        int end   = training_entries.size() * (threadId + 1) / threads;

        double    score;
        Evaluator evaluator {};
        for (int i = start; i < end; i++) {

            Score  q_i      = evaluator.evaluate(&training_entries[i].board);
            double expected = training_entries[i].target;

            double sig = sigmoid(q_i, K);

            score += (expected - sig) * (expected - sig);
        }
        resultTarget[threadId] = score;
    };

    double                    resultTargets[threads];
    double*                   resultPointer = &resultTargets[0];
    std::vector<std::thread*> runningThreads {};
    for (int i = 0; i < threads; i++) {
        std::thread* t1 = new std::thread(eval_part, i, threads, K, resultPointer);
        runningThreads.push_back(t1);
    }
    for (int i = 0; i < threads; i++) {
        if (runningThreads[i]->joinable()) {
            runningThreads[i]->join();
        }
    }
    for (int i = 0; i < threads; i++) {
        score += resultPointer[i];
    }

    return score / training_entries.size();
}

double tuning::computeK(double initK, double rate, double deviation, int threads) {

    double K    = initK;
    double dK   = 0.01;
    double dEdK = 1;

    while (abs(dEdK) > deviation) {

        double Epdk = computeError(K + dK, threads);
        double Emdk = computeError(K - dK, threads);

        dEdK = (Epdk - Emdk) / (2 * dK);

        std::cout << "K:" << K << " Error: " << (Epdk + Emdk) / 2 << " dev: " << abs(dEdK) << std::endl;

        K -= dEdK * rate;
    }

    return K;
}

void tuning::evalSpeed() {

    Evaluator evaluator {};

    startMeasure();
    U64 sum = 0;
    for (int i = 0; i < training_entries.size(); i++) {
        sum += evaluator.evaluate(&training_entries[i].board);
    }

    int ms = stopMeasure();
    std::cout << ms << "ms for " << training_entries.size()
              << " positions = " << (1000 * training_entries.size() / ms) / 1e6 << "Mnps"
              << " Checksum = " << sum << std::endl;
}
