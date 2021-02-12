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
    changeCount[anticlockwise] =0;

    return TT_ENCODER_SUCCESS
}

void TTEncoder::inARiseISR(void){
    if(!inB->read()){
        changeCount[clockwise]++;
    }
    else{
        changeCount[anticlockwise]++;
    }
}

void TTEncoder::inAFallISR(void){
    if(inB->read()){
        changeCount[clockwise]++;
    }
    else{
        changeCount[anticlockwise]++;
    }
}

void TTEncoder::inBRiseISR(void){
    if(inA->read()){
        changeCount[clockwise]++;
    }
    else{
        changeCount[anticlockwise]++;
    }
}

void TTEncoder::inBFallISR(void){
    if(!inA->read()){
        changeCount[clockwise]++;
    }
    else{
        changeCount[anticlockwise]++;
    }
}