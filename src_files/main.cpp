
#include <iomanip>
#include "uci.h"


#include "Bitboard.h"
#include "Board.h"
#include "Move.h"
#include "MoveOrderer.h"
#include "Verification.h"
#include "Tuning.h"

using namespace std;
using namespace bb;
using namespace move;




void main_tune_features(){
    bb_init();
    Evaluator *evaluator = new Evaluator();
    
    using namespace tuning;
    
    loadPositionFile("resources/quiet-labeled.epd", 1e6);
    auto K = tuning::computeK(evaluator,2.86681, 200, 1e-7);
    
    for(int i = 0; i < 5000; i++){
        
        std::cout << "--------------------------------------------------- ["<<i << "] ----------------------------------------------" << std::endl;
        
        std::cout << std::setprecision(8) << tuning::optimiseGD(evaluator, K, 2e4) << std::endl;
        
        for(int k = 0; k < evaluator->paramCount(); k++){
            std::cout << std::setw(14) << evaluator->getEarlyGameParams()[k]<< ",";
        }
        std::cout << std::endl;
        for(int k = 0; k < evaluator->paramCount(); k++){
            std::cout << std::setw(14) <<evaluator->getLateGameParams()[k] << ",";
        }
        std::cout << std::endl;
        
    }
    
    delete evaluator;
    bb_cleanUp();
}


#ifdef TUNE_PST
void main_tune_pst(){
    bb_init();
    Evaluator *evaluator = new Evaluator();
    
    using namespace tuning;
    
    loadPositionFile("resources/quiet-labeled.epd", 1e6);
    
    //auto K = tuning::computeK(evaluator,2.86681, 200, 1e-7);
    


    for(int i = 0; i < 5000; i++){
        
        std::cout << "--------------------------------------------------- ["<<i << "] ----------------------------------------------" << std::endl;
        
        std::cout << std::setprecision(8) << tuning::optimisePST(evaluator, 2.86681, 1e6) << std::endl;
        
        for(int k = 0; k < 64; k++){
            std::cout << std::setprecision(1) << fixed << std::setw(10) << evaluator->getTunablePST_MG()[k]<< ",";
            if(k % 8 == 7) std::cout << "\n";
        }
        std::cout << std::endl;
        for(int k = 0; k < 64; k++){
            std::cout << std::setprecision(1) << fixed << std::setw(10) << evaluator->getTunablePST_EG()[k]<< ",";
            if(k % 8 == 7) std::cout << "\n";
        }
        std::cout << std::endl;
        
    }
    
    delete evaluator;
    bb_cleanUp();
}
#endif



int main(int argc, char *argv[]) {
    
    if(argc == 1){
        uci_loop(false);
    }else if(argc > 1 && strcmp(argv[1], "bench") == 0){
        uci_loop(true);
    }
    


    
    
    
    
    
    
    
    
    
    
    
    
    
    /**********************************************************************************
     *                                  T U N I N G                                   *
     **********************************************************************************/


    //bb_init();
    //tuning::loadPositionFile("resources/quiet-labeled.epd", 1000000);
    //tuning::evalSpeed();
    //bb_cleanUp();
    
    //main_tune_features();
    //main_tune_pst();
    
    return 0;
    
}




