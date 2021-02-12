#include "ttencoder.h"

TTEncoder::TTEncoder(PinName inA, PinName inB, PinMode inAMode, PinMode inBMode){
    this->inA = new InterruptIn(inA, inAMode);
    this->inB = new InterruptIn(inB, inBMode);

    this->inA->rise(callback(this, &TTEncoder::inARiseISR));
    this->inA->fall(callback(this, &TTEncoder::inAFallISR));
    this->inB->rise(callback(this, &TTEncoder::inBRiseISR));
    this->inB->fall(callback(this, &TTEncoder::inBFallISR));
}

int TTEncoder::getChangeCount(void){
    return changeCount[clockwise] - changeCount[anticlockwise];
}

int TTEncoder::getChangeCount(int direction){
    return changeCount[direction];
}

int TTEncoder::Reset(void){
    changeCount[clockwise] = 0;
    changeCount[anticlockwise] = 0;

    return TT_ENCODER_SUCCESS;
}

void TTEncoder::inARiseISR(void){
    switch(state){
        case 2:
            changeCount[anticlockwise]++;
            state = 1;
            break;

        case 3:
            changeCount[clockwise]++;
            state = 0;
            break;

        default:
            //Illegal
            break;
    }
}

void TTEncoder::inAFallISR(void){
    switch(state){
        case 0:
            changeCount[anticlockwise]++;
            state = 3;
            break;

        case 1:
            changeCount[clockwise]++;
            state = 2;
            break;

        default:
            //Illegal
            break;
    }
}

void TTEncoder::inBRiseISR(void){
    switch(state){
        case 0:
            changeCount[clockwise]++;
            state = 1;
            break;

        case 3:
            changeCount[anticlockwise]++;
            state = 2;
            break;

        default:
            //Illegal
            break;
    }
}

void TTEncoder::inBFallISR(void){
    switch(state){
        case 1:
            changeCount[anticlockwise]++;
            state = 0;
            break;
        
        case 2:
            changeCount[clockwise]++;
            state = 3;
            break;

        default:
            //Illegal
            break;
    }
}