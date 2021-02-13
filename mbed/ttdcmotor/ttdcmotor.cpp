/**
*     _____ _____ ___     __  __     _           
*    |_   _|_   _|   \ __|  \/  |___| |_ ___ _ _ 
*      | |   | | | |) / _| |\/| / _ \  _/ _ \ '_|
*      |_|   |_| |___/\__|_|  |_\___/\__\___/_|                                                      
*
*
* @file TTEncoder.h
* @brief This file contains the functions with TTDcMotor.
*
* @author Ted Tooth
* @date 11 Feb 2021
*
* @copyright Ted Tooth 2021
*/

#include "ttdcmotor.h"

TTDcMotor::TTDcMotor(PinName en, PinName A, PinName B, float period, bool inaInbActiveLow) 
    : pwm(en), A(A), B(B), inaInbActiveLow(inaInbActiveLow){
        pwm.period(period);
}

int TTDcMotor::Spin(float speed, bool direction){
    if(!mtx.trylock_for(TT_DEFAULT_MUTEX_TIMEOUT)){
        return TT_MUTEX_TIMEOUT;
    }
    else{
        int retVal = TT_SUCCESS;
        
        //Set speeds less tha 0% to 0%.
        if(speed < 0){
            speed = 0;
            retVal = TT_FLOORED_SPEED;
        }
        //Set speeds greater than 100% to 100%;
        else if(speed > 1){
            speed = 1;
            retVal = TT_CEILINGED_SPEED;
        }

        pwm.write(speed);
        SetDirection(direction);

        mtx.unlock();
        return retVal;
    }
}

int TTDcMotor::Stop(void){
    if(!mtx.trylock_for(TT_DEFAULT_MUTEX_TIMEOUT)){
        return TT_MUTEX_TIMEOUT;
    }
    else{
        pwm.write(0);
        A = inaInbActiveLow;
        B = inaInbActiveLow;

        mtx.unlock();
        return TT_SUCCESS;
    }
}

int TTDcMotor::RegisterEncoder(PinName inA, PinName inB, PinMode inAMode, PinMode inBMode){
    if(!mtx.trylock_for(TT_DEFAULT_MUTEX_TIMEOUT)){
        return TT_MUTEX_TIMEOUT;
    }
    else{
        bool existingEncoder = false;
        if(encoder != 0){
            existingEncoder = true;
        }

        encoder = new TTEncoder(inA, inB, inAMode, inBMode);

        mtx.unlock();

        if(existingEncoder){
            return TT_OVERWROTE_ENCODER;
        }
        else{
            return TT_SUCCESS;
        }
    }
}

int TTDcMotor::IsMoving(void){
    if(!mtx.trylock_for(TT_DEFAULT_MUTEX_TIMEOUT)){
        return TT_MUTEX_TIMEOUT;
    }
    else{
        bool retval = moving;
        mtx.unlock();
        return retval;
    }
}

int TTDcMotor::Move(float speed, int x, bool direction){
    if(!mtx.trylock_for(TT_DEFAULT_MUTEX_TIMEOUT)){
        return TT_MUTEX_TIMEOUT;
    }
    else{
        if(!moving){
            if(encoder != 0){
                moving = true;
                if(direction == encoder->clockwise){
                    endInterrupts = encoder->getInterruptCount() + x;
                }
                else{
                    endInterrupts = encoder->getInterruptCount() - x;
                }

                encoder->SetOnInterruptCallback(callback(this, &TTDcMotor::MoveISR));

                mtx.unlock();
                return Spin(speed, direction);
            } 
            else{
                mtx.unlock();
                return TT_NO_REGISTERED_ENCODER;
            }
        }
        else{
            mtx.unlock();
            return TT_ALREADY_MOVING;
        }
    }
}

int TTDcMotor::SetMoveEndedCallback(function<void()> callback){
    if(!mtx.trylock_for(TT_DEFAULT_MUTEX_TIMEOUT)){
        return TT_MUTEX_TIMEOUT;
    }
    else{
        bool existingCallback = false;
        if(onMoveEndedCallback != 0){
            existingCallback = true;
        }

        onMoveEndedCallback = callback;

        mtx.unlock();

        if(existingCallback){
            return TT_OVERWROTE_CALLBACK;
        }
        else{
            return TT_SUCCESS;
        }
    }
}

int TTDcMotor::SetDirection(bool dir){
    if(dir){
        A = inaInbActiveLow;
        B = !inaInbActiveLow;
    }
    else
    {
        A = !inaInbActiveLow;
        B = inaInbActiveLow;
    }

    return TT_SUCCESS;
}

void TTDcMotor::MoveISR(void){
    if(encoder->getInterruptCount() != endInterrupts){
        //DO nothing
    }
    else{
        Stop();
        encoder->SetOnInterruptCallback(0);
        moving = false;
    }
}
