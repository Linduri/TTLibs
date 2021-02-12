#include "ttencoder.h"

TTEncoder::TTEncoder(PinName inA, PinName inB, PinMode inAMode, PinMode inBMode){
    this->inA = new InterruptIn(inA, inAMode);
    this->inB = new InterruptIn(inB, inBMode);

    this->inA->rise(callback(this, &TTEncoder::inARiseISR));
    this->inA->fall(callback(this, &TTEncoder::inAFallISR));
    this->inB->rise(callback(this, &TTEncoder::inBRiseISR));
    this->inB->fall(callback(this, &TTEncoder::inBFallISR));
}

// void TTEncoder::StateMachine(void){
//     switch(state){
//         case waiting:
//             //Nothing has happened yet, just be chill.
//             break;

//         case aUp:
//             //If 
//             break;

//         case aDown:
            
//             break;

//         case bUp:
            
//             break;

//         case bDown:
            
//             break;

//         default:

//             break;
//     }
// }

int TTEncoder::getChangeCount(void){
    return changeCount[TTENCODER_CLOCKWISE] - changeCount[TTENCODER_ANTICLOCKWISE];
}

void TTEncoder::inARiseISR(void){
    if(!inB->read()){
        changeCount[TTENCODER_CLOCKWISE]++;
    }
    else{
        changeCount[TTENCODER_ANTICLOCKWISE]++;
    }
}

void TTEncoder::inAFallISR(void){
    if(inB->read()){
        changeCount[TTENCODER_CLOCKWISE]++;
    }
    else{
        changeCount[TTENCODER_ANTICLOCKWISE]++;
    }
}

void TTEncoder::inBRiseISR(void){
    if(inA->read()){
        changeCount[TTENCODER_CLOCKWISE]++;
    }
    else{
        changeCount[TTENCODER_ANTICLOCKWISE]++;
    }
}

void TTEncoder::inBFallISR(void){
    if(!inA->read()){
        changeCount[TTENCODER_CLOCKWISE]++;
    }
    else{
        changeCount[TTENCODER_ANTICLOCKWISE]++;
    }
}