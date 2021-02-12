/**
*     _____ _____ ___                 _         
*    |_   _|_   _| __|_ _  __ ___  __| |___ _ _ 
*      | |   | | | _|| ' \/ _/ _ \/ _` / -_) '_|
*      |_|   |_| |___|_||_\__\___/\__,_\___|_|  
*                                                     
*
*
* @file TTEncoder.cpp
* @brief This file contains the functions associated with TTEncoder.
*
* @author Ted Tooth
* @date 11 Feb 2021
*
* @copyright Ted Tooth 2021
*/

#include "ttencoder.h"

TTEncoder::TTEncoder(PinName inA, PinName inB, PinMode inAMode, PinMode inBMode){
    this->inA = new InterruptIn(inA, inAMode);
    this->inB = new InterruptIn(inB, inBMode);

    this->inA->rise(callback(this, &TTEncoder::inARiseISR));
    this->inA->fall(callback(this, &TTEncoder::inAFallISR));
    this->inB->rise(callback(this, &TTEncoder::inBRiseISR));
    this->inB->fall(callback(this, &TTEncoder::inBFallISR));
}

int TTEncoder::getInterruptCount(void){
    if(!mtx.trylock_for(TT_DEFAULT_MUTEX_TIMEOUT)){
        return TT_MUTEX_TIMEOUT;
    }
    else{
        int retval = changeCount[clockwise] - changeCount[anticlockwise];
        mtx.unlock();
        return retval;
    }
}

int TTEncoder::getInterruptCount(int direction){
    if(!mtx.trylock_for(TT_DEFAULT_MUTEX_TIMEOUT)){
        return TT_MUTEX_TIMEOUT;
    }
    else{
        int retval =  changeCount[direction];
        mtx.unlock();
        return retval;
    }
}

int TTEncoder::Reset(void){
    if(!mtx.trylock_for(TT_DEFAULT_MUTEX_TIMEOUT)){
        return TT_MUTEX_TIMEOUT;
    }
    else{
        changeCount[clockwise] = 0;
        changeCount[anticlockwise] = 0;
        mtx.unlock();
        return TT_SUCCESS;
    }
}

int TTEncoder::SetOnInterruptCallback(function<void()> callback){
    if(!mtx.trylock_for(TT_DEFAULT_MUTEX_TIMEOUT)){
        return TT_MUTEX_TIMEOUT;
    }
    else{
        bool existingCallback = false;
        if(onInterruptCallback != 0){
            existingCallback = true;
        }

        onInterruptCallback = callback;

        mtx.unlock();

        if(existingCallback){
            return TT_OVERWROTE_CALLBACK;
        }
        else{
            return TT_SUCCESS;
        }
    }
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

    if(onInterruptCallback != 0){
        onInterruptCallback();
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

    if(onInterruptCallback != 0){
        onInterruptCallback();
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

    if(onInterruptCallback != 0){
        onInterruptCallback();
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

    if(onInterruptCallback != 0){
        onInterruptCallback();
    }
}