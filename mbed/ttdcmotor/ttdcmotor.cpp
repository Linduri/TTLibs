#include "ttdcmotor.h"

TTDcMotor::TTDcMotor(PinName pwm, PinName hA, PinName hB, PinName hEn) : pwm(pwm), hA(hA, 0), hB(hB, 0),hEn(hEn, 0){}

/*
* @brief Moves the motor perpetually.
* @param speed Percentage speed, (0 <= speed <= 1).
* @param direction Speed clockwise or anti-clockwise.
* @returns TT_DC_MOTOR_SUCCESS
*/
int TTDcMotor::Spin(float speed, bool direction){
    int retVal = TT_DC_MOTOR_SUCCESS;
    
    //Set speeds less tha 0% to 0%.
    if(speed < 0){
        speed = 0;
        retVal = TT_DC_MOTOR_FLOORED_SPEED;
    }
    //Set speeds greater than 100% to 100%;
    else if(speed > 1){
        speed = 1;
        retVal = TT_DC_MOTOR_CEILINGED_SPEED;
    }

    pwm.write(speed);
    SetDirection(direction);
    hEn = 1;
    return TT_DC_MOTOR_SUCCESS;
}

/*
* @brief Stops any motor movement.
* @returns TT_DC_MOTOR_SUCCESS
*/
int TTDcMotor::Stop(void){
    pwm.write(0);
    hEn = 0;
    return TT_DC_MOTOR_SUCCESS;
}

/*
* @brief Registers an encoder to use wiht the motor.
* @param inA Encoder output A pin.
* @param inB Encoder output B pin.
* @param inAMode (Optional) Encoder output A pin mode.
* @param inBMode (Optional) Encoder output B pin mode.
* @returns TT_ENCODER_SUCCESS or negative error code.
* @retval TT_DC_MOTOR_OVERWRITTEN_ENCODER Over wrote existing encoder instance.
*/
int TTDcMotor::RegisterEncoder(PinName inA, PinName inB, PinMode inAMode, PinMode inBMode){
    bool existingEncoder = false;
    if(encoder != 0){
        existingEncoder = true;
    }

    encoder = new TTEncoder(inA, inB, inAMode, inBMode);

    if(existingEncoder){
        return TT_DC_MOTOR_OVERWRITTEN_ENCODER;
    }
    else{
        return TT_ENCODER_SUCCESS;
    }
}

/*
* @brief ISR callback for checking if a move is finished.
*/
void TTDcMotor::MoveCallback(void){
    if(encoder->getChangeCount() != endPulses){
        //DO nothing
    }
    else{
        Stop();
        encoder->SetOnPulseCallback(0);
        moving = false;
    }
}

/*
* @brief Is the motor currently moving?
* @returns True or false
*/
int TTDcMotor::IsMoving(void){
    return moving;
}

/*
* @brief Move the motor x encoder in direction.
* @param speed 0% (0) to 100% (1) speed to move at.
* @param x Number of encoder pulses to move. 
* @param direction Direction to move in.
* @returns TT_ENCODER_SUCCESS or negative error code.
* @retval TT_DC_MOTOR_NO_REGISTERED_ENCODER No encoder registered, use RegisterEncoder() to set one.
* @retval TT_DC_MOTOR_ALREADY_MOVING The motor is currently moving, wait until it has stopped.
*/
int TTDcMotor::Move(float speed, int x, bool direction){
    if(!moving){
        if(encoder != 0){
            moving = true;
            if(direction == encoder->clockwise){
                endPulses = encoder->getChangeCount() + x;
            }
            else{
                endPulses = encoder->getChangeCount() - x;
            }

            encoder->SetOnPulseCallback(callback(this, &TTDcMotor::MoveCallback));

            return Spin(speed, direction);
        } 
        else{
            return TT_DC_MOTOR_NO_REGISTERED_ENCODER;
        }
    }
    else{
        return TT_DC_MOTOR_ALREADY_MOVING;
    }
}

int TTDcMotor::SetMoveEndedCallback(function<void()> callback){
    bool existingCallback = false;
    if(onMoveEndedCallback != 0){
        existingCallback = true;
    }

    onMoveEndedCallback = callback;

    if(existingCallback){
        return TT_DC_MOTOR_CALLBACK_OVERWRITTEN;
    }
    else{
        return TT_ENCODER_SUCCESS;
    }
}

int TTDcMotor::SetDirection(bool dir){
    if(dir){
        hA = 0;
        hB = 1;
    }
    else
    {
        hA = 1;
        hB = 0;
    }

    return TT_DC_MOTOR_SUCCESS;
}