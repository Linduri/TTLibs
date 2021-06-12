/**
*        _____ _____ ___ _                          
*       |_   _|_   _/ __| |_ ___ _ __ _ __  ___ _ _ 
*         | |   | | \__ \  _/ -_) '_ \ '_ \/ -_) '_|
*         |_|   |_| |___/\__\___| .__/ .__/\___|_|  
*                               |_|  |_|            
*
*
* @file TTStepper.cpp
* @brief This file contains the functions associated with TTStepper.
*
* @author Ted Tooth
* @date 07 June 2021
*
* @copyright Ted Tooth 2021
*/

#include "ttstepper.h"
#include <algorithm>
#include <chrono>
#include <functional>

TTStepper::TTStepper(PinName en, PinName step, PinName dir, uint32_t stepsPerRev, float posPerRev) : step(step), dir(dir), en(en), stepsPerRev(stepsPerRev), posPerRev(posPerRev){
    Disable();
}

int TTStepper::SetEnable(bool enable){
    TTSTEPPER_ACQUIRE_MUTEX;
    int retval = TTSTEPPER_SUCCESS;

    if(en != NC){
        en = enActiveLow ? !enable : enable;
        retval = 1;
    }
    else{
        retval = 0;
    }

    TTSTEPPER_RELEASE_MUTEX;
    return retval;
}

bool TTStepper::Enable(){
    return SetEnable(true);
}

bool  TTStepper::Disable(){
    return SetEnable(false);
}

bool TTStepper::IsEnabled(){
    return en;
}

int TTStepper::RegisterEndstop(PinName pin, PinMode mode){
    TTSTEPPER_ACQUIRE_MUTEX;
    int retval = TTSTEPPER_SUCCESS;

    //Attatch endstop interrupts.
    if(lowerEndstop == 0){
        lowerEndstop = new InterruptIn(pin, mode);
        lowerEndstop->rise(callback(this, &TTStepper::LowerEndstopRiseISR));
        lowerEndstop->fall(callback(this, &TTStepper::LowerEndstopFallISR));
        retval = TTSTEPPER_LOWER_ENDSTOP;
    }
    else if(upperEndstop == 0){
        upperEndstop = new InterruptIn(pin, mode);
        upperEndstop->rise(callback(this, &TTStepper::UpperEndstopRiseISR));
        upperEndstop->fall(callback(this, &TTStepper::UpperEndstopFallISR));
        retval = TTSTEPPER_UPPER_ENDSTOP;
    }
    else{
        retval = TTSTEPPER_NO_FREE_ENDSTOPS;
    }

    TTSTEPPER_RELEASE_MUTEX;
    return retval;
}

int TTStepper::Home(uint32_t bounceSteps, int endstopId){
    TTSTEPPER_ACQUIRE_MUTEX;

    homing = true;
    debug("Homing\r\n");
    int retVal = SUCCESS;
    InterruptIn *endstop;

    //Check for a registered endstop.
    if(endstopId == TTSTEPPER_LOWER_ENDSTOP){
        if(lowerEndstop == 0){
            TTSTEPPER_RELEASE_MUTEX;
            return TTSTEPPER_ENDSTOP_NOT_REGISTERED;
        } else {
            endstop = lowerEndstop;
        }
    }
    else if(endstopId == TTSTEPPER_UPPER_ENDSTOP){
        if(upperEndstop == 0){
            TTSTEPPER_RELEASE_MUTEX;
            return TTSTEPPER_ENDSTOP_NOT_REGISTERED;
        } else {
            endstop = upperEndstop;
        }
    }
    else{
        TTSTEPPER_RELEASE_MUTEX;
        return TTSTEPPER_INVALID_ID;
    }

    //Make sure motor is stopped.
    Stop();

    debug("Moving to endstop\r\n");

    while(endstop->read() == !invertEndstops){
        retVal = Step(1000000000, TTSTEPPER_ANTI_CLOCKWISE);
        if(retVal == TTSTEPPER_ALREADY_MOVING){ 
            TTSTEPPER_RELEASE_MUTEX;
            return retVal;
        }
        WaitBlocking();
    }

    debug("Bouncing from endstop\r\n");

    //Move out of hit endstop
    while(endstop->read() == invertEndstops){
        retVal = Step(1000000000, TTSTEPPER_CLOCKWISE);
        if(retVal == TTSTEPPER_ALREADY_MOVING){ 
            TTSTEPPER_RELEASE_MUTEX; 
            return retVal;
        }
        WaitBlocking();
    }

    //Move extra to avoid re-triggering endstop.
    retVal = Step(bounceSteps, TTSTEPPER_CLOCKWISE);
    if(retVal != SUCCESS){ 
        TTSTEPPER_RELEASE_MUTEX;
        return retVal;
    }

    WaitBlocking();

    //Reset step.
    currentStep = 0;

    //Clear any latent endstop interrupts.
    ClearEndstopHit();

    homing = false;
    debug("Homed!\r\n");
    TTSTEPPER_RELEASE_MUTEX;
    return SUCCESS;
}

int TTStepper::MoveSteps(long steps){
    TTSTEPPER_ACQUIRE_MUTEX;
    int retval = Step(steps, steps < 0 ? TTSTEPPER_ANTI_CLOCKWISE : TTSTEPPER_CLOCKWISE);
    TTSTEPPER_RELEASE_MUTEX;
    return retval;
}

int TTStepper::MoveDegs(float degrees){
    TTSTEPPER_ACQUIRE_MUTEX;

    bool direction = TTSTEPPER_CLOCKWISE;

    if(degrees < 0){
        degrees *= -1;
        direction = TTSTEPPER_ANTI_CLOCKWISE;
    }

    int retval = Step((degrees / 360) * stepsPerRev, direction);

    TTSTEPPER_RELEASE_MUTEX;
    return retval;
}

int TTStepper::MovePos(float units){
    return MoveDegs((units / posPerRev) * 360.f);
}

void TTStepper::GoToRot(float degrees){
    MoveDegs(degrees - GetDegs());
}

void TTStepper::GoToPos(float pos){
    GoToRot((pos / posPerRev) * 360.f);
}

int TTStepper::WaitBlocking(){
    TTSTEPPER_ACQUIRE_MUTEX;

    while(IsMoving()){
        ThisThread::sleep_for(1ms);
    }

    TTSTEPPER_RELEASE_MUTEX;
    return TTSTEPPER_SUCCESS;
}

void TTStepper::Stop(){
    moving = false;
    stepTimout.detach();
}

float TTStepper::GetDegs(){
    return (currentStep / (float)stepsPerRev) * 360.0f;
}

float TTStepper::GetPos(){
    return GetDegs() * (posPerRev / 360.0f);
}

bool TTStepper::IsMoving(){
    return moving;
}

void TTStepper::ClearEndstopHit(void){
    endstopHit = 0;
}

void TTStepper::ClearEndstopReleased(void){
    endstopReleased = 0;
}

int TTStepper::SetMaxSpeed(float speed){
    TTSTEPPER_ACQUIRE_MUTEX;
    maxSpeed = speed;
    TTSTEPPER_RELEASE_MUTEX;
    return TTSTEPPER_SUCCESS;
}

int TTStepper::SetMinSpeed(float speed){
    TTSTEPPER_ACQUIRE_MUTEX;
    minSpeed = speed;
    TTSTEPPER_RELEASE_MUTEX;
    return TTSTEPPER_SUCCESS;
}

int TTStepper::SetHomingSpeed(float speed){
    TTSTEPPER_ACQUIRE_MUTEX;
    homeSpeed = speed;
    TTSTEPPER_RELEASE_MUTEX;
    return TTSTEPPER_SUCCESS;
}

int TTStepper::SetAccelerationMultiplier(float multiplier){
    TTSTEPPER_ACQUIRE_MUTEX;
    speedInterval = TTSTEPPER_BASE_SPEED_INTERVAL * multiplier;
    TTSTEPPER_RELEASE_MUTEX;
    return TTSTEPPER_SUCCESS;
}

int TTStepper::Reverse(bool reverse){
    TTSTEPPER_ACQUIRE_MUTEX;
    this->reverse = reverse;
    TTSTEPPER_RELEASE_MUTEX;
    return TTSTEPPER_SUCCESS;
}

int TTStepper::InvertEndstops(bool invert){
    TTSTEPPER_ACQUIRE_MUTEX;
    invertEndstops = invert;
    TTSTEPPER_RELEASE_MUTEX;
    return TTSTEPPER_SUCCESS;
}

int TTStepper::Step(uint32_t steps, bool direction){
    if(!endstopHit | homing){
        if(!moving){

            moving = true;

            Enable();
            
            dir = !reverse ? direction : !direction;
            
            remainingSteps = steps;

            uint32_t accelerationStopStep = (maxSpeed - minSpeed) / speedInterval;
            if(steps > accelerationStopStep * 2){
                slowStep = accelerationStopStep;
            }
            else{
                slowStep = steps / 2;
            }

            speed = minSpeed;

            StepTimeoutHandler();

            return SUCCESS;
        }
        else{
            return TTSTEPPER_ALREADY_MOVING;
        }
    }
    else{
        return TTSTEPPER_ENDSTOP_HIT;
    }

    return TTSTEPPER_SUCCESS;
}

void TTStepper::StepTimeoutHandler(){
    if(remainingSteps){
        step = stepActiveLow ? false : true;
        step = !step;

        currentStep = dir ? currentStep + 1 : currentStep - 1;
        
        remainingSteps--;

        if(remainingSteps > slowStep){
            if(speed < maxSpeed){
                speed += speedInterval;

                if(speed > maxSpeed){
                    speed = maxSpeed;
                }
            }
        }
        else{
            speed -= speedInterval;

            if(speed < minSpeed){
                speed = minSpeed;
            }
        }

        long period;
        if(homing){
            period = (1000000.0f / (stepsPerRev * homeSpeed));
        } else{
            period = (1000000.0f / (stepsPerRev * speed));
        }

        stepTimout.attach(callback(this, &TTStepper::StepTimeoutHandler), chrono::microseconds(period));
    }
    else{
        Stop();
    }
}

void TTStepper::Endstop(int id, bool rise){
    rise = invertEndstops ? !rise : rise;

    if(rise){
        Stop();
        
        endstopHit = id;
        if(onEndstopHit != 0){
            onEndstopHit(id);
        }
    }
    else{
        endstopReleased = id;
        if(onEndstopReleased != 0){
            onEndstopReleased(id);
        }
    }
}

void TTStepper::LowerEndstopRiseISR(void){
    Endstop(TTSTEPPER_LOWER_ENDSTOP, true);
}

void TTStepper::LowerEndstopFallISR(void){
    Endstop(TTSTEPPER_LOWER_ENDSTOP, false);
}

void TTStepper::UpperEndstopRiseISR(void){
    Endstop(TTSTEPPER_UPPER_ENDSTOP, true);
}

void TTStepper::UpperEndstopFallISR(void){
    Endstop(TTSTEPPER_UPPER_ENDSTOP, false);
}

TTStepper::~TTStepper(){
    Stop();
    Disable();
}