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
* @date 31 Jan 2021
*
* @copyright Ted Tooth 2021
*/

#include "ttstepper.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ratio>

TTStepper::TTStepper(PinName en, PinName step, PinName dir, int stepsPerRev, float slideUnitsPerRev) : 
                    degsPerStep(360.0f / stepsPerRev),
                    stepsPerRevolution(stepsPerRev),
                    en(en), 
                    step(step), 
                    dir(dir),
                    slideUnitsPerStep(slideUnitsPerRev / stepsPerRev){
    
    //Set default speed parameters.
    SetMaxRPS(TTSTEPPER_DEFAULT_RPS);
    SetRPSS(TTSTEPPER_DEFAULT_RPSS);

    //Enable the stepper.
    this->en = 0;
}

float TTStepper::GetDegrees(void){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::GetDegrees() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }    
    else{
        float degrees = fmod(GetRevs(), 1) * 360.0f;

        stepperLock.unlock();
        return degrees;
    }
}

float TTStepper::GetLifetimeDegrees(void){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::GetLifetimeDegrees() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }    
    else{
        float degrees = GetRevs() * 360.0f;

        stepperLock.unlock();
        return degrees;
    }
}

int TTStepper::Rotate(float degrees, bool direction){
    return Step(degrees / degsPerStep, direction);
}

int TTStepper::RotateTo(float degrees){
    float currentDegrees = GetDegrees();
    float deltaDegrees;
    bool direction;

    if(degrees > currentDegrees){
        deltaDegrees = degrees - currentDegrees;
        direction = TTSTEPPER_CLOCKWISE;
    }
    else if(degrees < currentDegrees){
        deltaDegrees = currentDegrees - degrees;
        direction = TTSTEPPER_ANTICLOCKWISE;
    }
    else{
        //It's already there, return success.
        return TTSTEPPER_SUCCESS;
    }

    //Choose the shortest path.
    if(deltaDegrees > 180.0f){
        direction = !direction;
        deltaDegrees -= 180.0f;
    }

    return RotateTo(degrees, direction);
}

int TTStepper::RotateTo(float degrees, bool direction){
    int targetStep = degrees / degsPerStep;

    uint32_t deltaSteps;    
    
    if(targetStep > currentStep){
        if(direction == TTSTEPPER_CLOCKWISE){
            deltaSteps = targetStep - currentStep;
        }
        else {
            deltaSteps = stepsPerRevolution - (targetStep - currentStep);
        }
        
    }
    else if(targetStep < currentStep){
        if(direction == TTSTEPPER_CLOCKWISE){
            deltaSteps = stepsPerRevolution - (currentStep - targetStep);
        }
        else {
            deltaSteps = currentStep - targetStep;
        }
    }
    else {
        //It's already there, return success.
        return TTSTEPPER_SUCCESS;
    }

    return Step(deltaSteps, direction);
}

int TTStepper::SetRotation(float degrees){
    int targetSteps = degrees / degsPerStep;
    bool direction;
    int deltaSteps;

    if(currentStep <= targetSteps){
        direction = TTSTEPPER_CLOCKWISE;
        deltaSteps = targetSteps - currentStep;
    }
    else{
        direction = TTSTEPPER_ANTICLOCKWISE;
        deltaSteps = currentStep - targetSteps;
    }

    return Step(deltaSteps, direction);
}

float TTStepper::GetSlidePositon(void){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::GetSlidePositon() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }    
    else{
        float slidePosition = currentStep * slideUnitsPerStep;
        stepperLock.unlock();
        return slidePosition;
    }
}

int TTStepper::InvertSlide(bool invert){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::InvertSlide() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }    
    else{
        invertSlide = invert;
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::SetSlideOffset(float offset){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::SetSlideOffset() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }    
    else{
        slideOffset = offset;
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::Slide(float distance, bool direction){
    return Step(distance / slideUnitsPerStep, direction);
}

int TTStepper::SlideTo(float position){
    float slidePosition = GetSlidePositon();

    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::InvertSlide() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }    
    else{
        bool direction;
        float deltaDistance;        
        position += slideOffset;

        if(slidePosition <= position){
            deltaDistance = position - slidePosition;
            direction = !invertSlide ? TTSTEPPER_CLOCKWISE : TTSTEPPER_ANTICLOCKWISE;
        }
        else{
            deltaDistance = slidePosition - position;
            direction = !invertSlide ? TTSTEPPER_ANTICLOCKWISE : TTSTEPPER_CLOCKWISE;
        }

        stepperLock.unlock();
        return Step(deltaDistance * stepsPerRevolution, direction);        
    }        
}

int TTStepper::RegisterEndstop(PinName pin, PinMode mode){
    //Attatch endstop interrupts.
    if(lowerEndstop == 0){
        lowerEndstop = new InterruptIn(pin, mode);
        lowerEndstop->rise(callback(this, &TTStepper::LowerEndstopRiseISR));
        lowerEndstop->fall(callback(this, &TTStepper::LowerEndstopFallISR));
        return 0;
    }
    else if(upperEndstop == 0){
        upperEndstop = new InterruptIn(pin, mode);
        upperEndstop->rise(callback(this, &TTStepper::UpperEndstopRiseISR));
        upperEndstop->fall(callback(this, &TTStepper::UpperEndstopFallISR));
        return 1;
    }
    else{
        return TTSTEPPER_ERROR_NO_FREE_ENDSTOPS;
    }
}

int TTStepper::ClearEndstopHit(void){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::ClearEndstopHit() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        endstopHit = 0;

        stepperLock.unlock();
        return TTSTEPPER_SUCCESS;
    }    
}

int TTStepper::Invert(bool invert){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::Invert() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        invertRotation = invert;

        stepperLock.unlock();
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::InvertEndstops(bool invert){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::InvertEndstops() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        invertEndstops = true;

        stepperLock.unlock();
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::Home(milliseconds timeout, bool direction){
    if(homing){
        return TTSTEPPER_ALREADY_HOMING;
    }
    else{
        homing = true;
        //Start the home.
        Step(1, direction);
        int waitResult = eventFlags.wait_any(TTSTEPPER_HOMED_FLAG, timeout.count());
        //If no flag triggered, report timeout.
        if(waitResult < 0){
            homing = false;
            return TTSTEPPER_ERROR_HOMING_TIMEOUT;
        }

        //If waited and not returned, endstop must've been hit.
        currentStep = 0;
        //This was a planned endstop hit, clear the flag.
        ClearEndstopHit();
        homing = false;

        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::SetMaxRPS(float rpsMax, float rpsMin){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::SetMaxRPS() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        this->rpsMax = rpsMax;
        this->minRps = rpsMin;
        
        stepperLock.unlock();
        SetRPSS(rpss);

        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::SetRPSS(float rpss){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::SetRPSS() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        //Assume any movement starts from stationary.
        this->rpss = rpss;
        rpssAdjusted = 1.0f / pow(rpss, 2);
        rpssT = sqrt(pow(rpss, 2) + pow(rpsMax, 2));
        rpsInterval = (minRps/rpssT);
        accelerationSteps = rpsMax / rpsInterval;

        stepperLock.unlock();
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::SetSlideUnitsPerStep(float unitsPerStep){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::SetSlideUnitsPerStep() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        slideUnitsPerStep = unitsPerStep;

        stepperLock.unlock();

        SetSlideUPS(upsMax);
        SetSlideUPSS(upssMax);
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::SetSlideUPS(float upsMax, float upsMin){
    if(slideUnitsPerStep != 0){
        this->upsMax = upsMax;
        float equivolentMaxRps = upsMax / (slideUnitsPerStep * stepsPerRevolution);
        
        if(upsMin != -1){
            float equivolentMinRps = upsMin / (slideUnitsPerStep * stepsPerRevolution);
            SetMaxRPS(equivolentMaxRps, equivolentMinRps);
        }
        else{
            SetMaxRPS(equivolentMaxRps);
        }
                
        SetRPSS(rpss);
        return TTSTEPPER_SUCCESS;
    }
    else{
        return TTSTEPPER_ERROR_SLIDE_UNITS_PER_STEP_NOT_SET;
    }
}

int TTStepper::SetSlideUPSS(float upss){
    if(slideUnitsPerStep != 0){
        float equivolentRpss = upss / (slideUnitsPerStep * stepsPerRevolution);
        SetRPSS(equivolentRpss);
        return TTSTEPPER_SUCCESS;
    }
    else{
        return TTSTEPPER_ERROR_SLIDE_UNITS_PER_STEP_NOT_SET;
    }
}

int TTStepper::WaitForTravelEnd(int timeoutMillis){
    int waitResult;
    if(timeoutMillis > 0){
        waitResult = eventFlags.wait_any(TTSTEPPER_TRAVEL_ENDED_FLAG, timeoutMillis);
    }
    else{
        waitResult = eventFlags.wait_any(TTSTEPPER_TRAVEL_ENDED_FLAG);
    }

    if(waitResult <= 0){
        return TTSTEPPER_ERROR_TRAVEL_WAIT_TIMEOUT;
    }
    else{
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::Enabled(void){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::Enabled() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        bool enabled = en;

        stepperLock.unlock();
        return enabled;
    }    
}

int TTStepper::Enable(bool enable){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::Enable() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        en = enable ? !enActiveLow : enActiveLow;

        stepperLock.unlock();
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::Enable(void){
    return Enable(true);
}

int TTStepper::Disable(void){
    return Enable(false);
}

int TTStepper::LastEndstopHit(void){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::LastEndstopHit() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        bool result = endstopHit;

        stepperLock.unlock();
        return result;
    }
}

int TTStepper::LastEndstopReleased(void){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::LastEndstopReleased() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        bool result = endstopReleased;

        stepperLock.unlock();
        return result;
    }
}

int TTStepper::GetHoming(void){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::GetHoming() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        bool result = homing;

        stepperLock.unlock();
        return result;
    }
}

int TTStepper::SetActiveBraking(bool enable){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::SetActiveBraking() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        activeBraking = enable;
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::SetEndstopHitCallback(function<void(int)> callback){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::SetEndstopHitCallback() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        onEndstopHit = callback;

        stepperLock.unlock();
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::SetEndstopReleasedCallback(function<void(int)> callback){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::SetEndstopReleasedCallback() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{
        onEndstopReleased = callback;

        stepperLock.unlock();
        return TTSTEPPER_SUCCESS;
    }
}

int TTStepper::Step(uint32_t steps, bool direciton){
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        debug("TTStepper::Step() - Failed to acquire stepperLock!\r\n");
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }
    else{          
        //If an endstop hit is not cleared, don't move!
        if(!endstopHit){  
            if(!travelling){
                //If active braking, re-enable the stepper.
                if(activeBraking){
                    en = !enActiveLow;
                }

                travelling = true;
                remainingSteps = steps;
                dir = invertRotation ? !direciton : direciton;
                
                int midSteps = steps / 2;
                if(accelerationSteps > midSteps){
                    decelerationStep = midSteps;
                }
                else{
                    decelerationStep = accelerationSteps;
                }
                //Reset velocity in case it didn't reset.
                rps = minRps;

                StepISR();
                stepperLock.unlock();
                return TTSTEPPER_SUCCESS;
            }
            else{
                stepperLock.unlock();
                return TTSTEPPER_ERROR_ALREADY_TRAVELLING;
            }
        }
        else{
            stepperLock.unlock();
            return TTSTEPPER_ERROR_ENDSTOP_HIT;
        }
    }
}

void TTStepper::StepISR(void){
    if(remainingSteps != 0){
        step = 1;
        step = 0;

        currentStep = dir ? currentStep + 1 : currentStep - 1;

        //If homing, don't decrement steps.
        if(!homing){
            remainingSteps--;
        }
        
        if(remainingSteps > decelerationStep){
            //Calculate current step velocity. 
            if(rps < rpsMax){
                rps += rpsInterval;
                if(rps >= rpsMax){
                    rps = rpsMax;
                }
            }
            else {
                //Do nothing.
            }
        }
        else{
            if(rps > minRps){
                rps -= rpsInterval;
                if(rps < 0.0f){
                    rps = minRps;
                }
            }
            else {
                //Do nothing.
            }
        }

        //Calculate the current velocity interval.
        long period = (1000000.0f / (stepsPerRevolution * rps));
        timeout.attach(callback(this, &TTStepper::StepISR), microseconds(period));
    }
    else{
        travelling = false;
        eventFlags.set(TTSTEPPER_TRAVEL_ENDED_FLAG);

        if(!activeBraking){
            en = enActiveLow;
        }
    }
}

float TTStepper::GetRevs(void){
    //Acquire lock for non-volatile stepsPerRevolution variable.
    if(!stepperLock.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){
        return TTSTEPPER_ERROR_MUTEX_TIMEOUT;
    }        
    else{
        //Cast currentStep to float to make sure this divide is floating point.
        float revs = (float)currentStep / stepsPerRevolution;
        stepperLock.unlock();
        return revs;
    }    
}

void TTStepper::Endstop(int id, bool rise){
    rise = invertEndstops ? !rise : rise;

    if(rise){
        //If non-active braking, disable the stepper.
        en = activeBraking ? !enActiveLow : enActiveLow;     
        //Stop any step sequence.
        timeout.detach();

        if(homing){
            homing = false;
            eventFlags.set(TTSTEPPER_HOMED_FLAG);            
        }

        if(travelling){
            travelling = false;
            eventFlags.set(TTSTEPPER_TRAVEL_ENDED_FLAG);
        }
        
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
    Endstop(0, true);
}

void TTStepper::LowerEndstopFallISR(void){
    Endstop(0, false);
}

void TTStepper::UpperEndstopRiseISR(void){
    Endstop(1, true);
}

void TTStepper::UpperEndstopFallISR(void){
    Endstop(1, false);
}
