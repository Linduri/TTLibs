/**
*        _____ _____ ___ _                          
*       |_   _|_   _/ __| |_ ___ _ __ _ __  ___ _ _ 
*         | |   | | \__ \  _/ -_) '_ \ '_ \/ -_) '_|
*         |_|   |_| |___/\__\___| .__/ .__/\___|_|  
*                               |_|  |_|            
*
*
* @file TTStepper.h
* @brief This file contains the definitions associated with TTStepper.
*
* @author Ted Tooth
* @date 07 June 2021
*
* @copyright Ted Tooth 2021
*/

#ifndef TT_STEPPER_H
#define TT_STEPPER_H

#define TTSTEPPER_CLOCKWISE true
#define TTSTEPPER_ANTI_CLOCKWISE false

#define TTSTEPPER_ACCELERATION_CURVE_LENGTH 100

#define TTSTEPPER_BASE_SPEED_INTERVAL 0.001f

#define TTSTEPPER_LOWER_ENDSTOP 1
#define TTSTEPPER_UPPER_ENDSTOP 2

#define TTSTEPPER_MUTEX_TIMEOUT 50ms
#define TTSTEPPER_ACQUIRE_MUTEX if(!mutex.trylock_for(TTSTEPPER_MUTEX_TIMEOUT)){return TTSTEPPER_MUTEX_TIMEDOUT;}
#define TTSTEPPER_RELEASE_MUTEX mutex.unlock()

#define TTSTEPPER_SUCCESS 0
#define TTSTEPPER_MUTEX_TIMEDOUT -1
#define TTSTEPPER_INVALID_ID -2
#define TTSTEPPER_INVALID_ENDSTOP -3
#define TTSTEPPER_ENDSTOP_NOT_REGISTERED -4
#define TTSTEPPER_ENDSTOP_HIT -5
#define TTSTEPPER_NO_FREE_ENDSTOPS -6
#define TTSTEPPER_ALREADY_MOVING -7

#include "mbed.h"
#include <cstdint>

class TTStepper{

    public:
        TTStepper(PinName en, PinName step, PinName dir, uint32_t stepsPerRev, float posPerRev = 1.0f);

        /** 
        * @brief Set the stepper enable pin logical high (independent of active low).
        * @returns Enable pin state
        * @retval true The enable pin was set high.
        * @retval false The enable pin was set low.
        */
        bool Enable();

        /** 
        * @brief Set the stepper enable pin logical low (independent of active low).
        * @returns Enable pin set state
        * @retval true The enable pin was set high.
        * @retval false The enable pin was set low.
        */
        bool Disable();
        
        /** 
        * @brief Get the stepper enable pin logical state (independent of active low).
        * @returns Enable pin state
        * @retval true The enable pin was set high.
        * @retval false The enable pin was set low.
        */
        bool IsEnabled();

        /**
        * @brief Adds an endstop to this stepper. Can take two, the first added is the lower. The second added is the higher.
        * @returns Positive endstop ID or a negative TTSTEPPER error code.
        */
        int RegisterEndstop(PinName pin, PinMode mode);

        /**
        * @brief Adds an endstop to this stepper. Can take two, the first added is the lower. The second added is the higher.
        * @param bounceSteps How many steps to "bounce" after triggering the endstop.
        * @param endstopId Endstop to home to (TTSTEPPER_LOWER_ENDSTOP or TTSTEPPER_UPPER_ENDSTOP).
        * @returns Success or a negative TTSTEPPER error code.
        */
        int Home(uint32_t bounceSteps = 100, int endstopId = TTSTEPPER_LOWER_ENDSTOP);

        /**
        * @brief Move the motor a specified number of steps.
        * @param steps How many steps to take. Positive = clockwise, negative = anti-clockwise.
        * @returns Success or a negative TTSTEPPER error code.
        */
        int MoveSteps(long steps);

        /**
        * @brief Move the motor a specified number of degrees.
        * @param degrees How many degrees to move. Positive = clockwise, negative = anti-clockwise.
        * @returns Success or a negative TTSTEPPER error code.
        */
        int MoveDegs(float degrees);

        /**
        * @brief Move the motor a specified number of units.
        * @param units How many units to move. Positive = clockwise, negative = anti-clockwise.
        * @returns Success or a negative TTSTEPPER error code.
        */
        int MovePos(float units);

        /**
        * @brief Go to a net rotation.
        * @param Target rotation. Can be >360 degrees. Positive = clockwise, negative = anti-clockwise.
        * @returns Success or a negative TTSTEPPER error code.
        */
        void GoToRot(float degrees);

        /**
        * @brief Go to a net position.
        * @param Target position. Positive = clockwise, negative = anti-clockwise.
        * @returns Success or a negative TTSTEPPER error code.
        */
        void GoToPos(float pos);

        /** 
        * @brief Wait for the motor to stop moving.
        * @warning This function blocks the calling thread.
        * @returns Success or a negative TTSTEPPER error code.
        */
        int WaitBlocking();

        /**
        * @brief Stops the motor.
        */
        void Stop();

        /**
        * @brief Get the net rotation of the stepper in degrees.
        * @returns The net rotation rotation of the stepper in degrees. Positive = clockwise, negative = anti-clockwise.
        */
        float GetDegs();
        
        /**
        * @brief Get the net position of the stepper in units.
        * @returns The net position of the stepper in units. Positive = clockwise, negative = anti-clockwise.
        */
        float GetPos();

        /**
        * @brief Gets if the stepper is currently moving.
        * @returns Is the motor moving?
        * @retval true The motor is moving.
        * @retval false The motor is stationary.
        */        
        bool IsMoving();

        /** @brief Reset the endstop hit flag. This will allow the motor to move after an endstop is triggered. */
        void ClearEndstopHit();

        /** @brief Reset the endstop released flag. This flag is purely informative. */
        void ClearEndstopReleased();

        /** 
        * @brief Scale maximum motor speed (units are abstract). Sets the home speed to 10% max speed. Sets min speed to 1% max speed.
        * @param speed The desired speed.
        * @returns Success or a negative TTSTEPPER error code.
        */
        int SetMaxSpeed(float speed);

        /** 
        * @brief Scale minimum motor speed (units are abstract). Sets the home speed to 10% max speed. Sets min speed to 1% max speed.
        * @param speed The desired speed.
        * @returns Success or a negative TTSTEPPER error code.
        */
        int SetMinSpeed(float speed);

        /** 
        * @brief Scale motor homing speed (units are abstract). Sets the home speed to 10% max speed. Sets min speed to 1% max speed.
        * @param speed The desired speed.
        * @returns Success or a negative TTSTEPPER error code.
        */
        int SetHomingSpeed(float speed);

        /** 
        * @brief Scale stepper acceleration.
        * @param multiplier The desired acceleration multiplier.
        * @returns Success or a negative TTSTEPPER error code.
        */
        int SetAccelerationMultiplier(float multiplier);

        /** 
        * @brief Set the stepper to run in the opposite direction. Clockwise becomes anti-clockwise and vice versa.
        * @param reverse Should the stepper output be reversed?
        * @param true Clockwise = anti-clockwise and vice versa.
        * @param false Clockwise = clockwise
        * @returns Success or a negative TTSTEPPER error code.
        */
        int Reverse(bool reverse);
        
        /** 
        * @brief Set if endstops should invert logical high.
        * @param invert Should the endstops input be inversed?
        * @param true Endstop input is opposite to pull-up / pull-down config.
        * @param false Endstop input is as specified by pull-up / pull-down config.
        * @returns Success or a negative TTSTEPPER error code.
        */
        int InvertEndstops(bool invert);

        ~TTStepper();      

    private:

    //================================================================================= GENERAL
        /** 
        * @brief Set the stepper enable pin (independent of active low).
        * @returns Set pin state or a negative TTSTEPPER error code.
        */
        int SetEnable(bool enable);

        /**
        * @brief Take a number of steps in a specified direction.
        * @param steps Number of steps to take.
        * @param direction Direction to move in.  TTSTEPPER_CLOCKWISE or TTSTEPPER_ANTI_CLOCKWISE.
        * @returns Success or a negative TTSTEPPER error code.
        */
        int Step(uint32_t steps, bool direction); 

        /** @brief Protect variable from modification while in use. */
        Mutex mutex;
        
    //==================================================================================== GPIO
        DigitalOut step, dir, en;

        /** @brief Is the stepper enable pin active low? */
        bool enActiveLow = true;

        /** @brief Is the stepper step pin active low? */
        bool stepActiveLow = true;

    //================================================================================ ENDSTOPS
        /** @brief Should endstop inputs be inverted? */
        bool invertEndstops = false;

        /** @brief Is the stepper currently homing? */
        volatile bool homing = false;

        /** @brief Instance lower and upper endstops. */
        InterruptIn *lowerEndstop, *upperEndstop = 0;

        /**
        * @brief Record an endstop event.
        * @param id ID of triggered endstop.
        * @param rise Is this event a rising (true) or falling (false) event?
        */
        void Endstop(int id, bool rise);

        /** @brief Trigger a lower endstop rise endstop event. */
        void LowerEndstopRiseISR();
        /** @brief Trigger a lower endstop fall endstop event. */
        void LowerEndstopFallISR();
        /** @brief Trigger an upper endstop rise endstop event. */
        void UpperEndstopRiseISR();
        /** @brief Trigger an upper endstop fall endstop event. */
        void UpperEndstopFallISR();

        /** @brief The id of the last endstop released. 0 = none, 1 = lower * 2 = upper. */
        int endstopHit = 0;

        /** @brief The id of the last endstop released. 0 = none, 1 = lower * 2 = upper. */
        int endstopReleased = 0;

        /** 
        * @brief Function to call on endstop hit. 
        * @param endstopHot The id of the endstop hit. 0 = lower, 1 = upper.
        */
        function<void(int endstopHit)> onEndstopHit;

        /** 
        * @brief Function to call on endstop release. 
        * @param endstopHot The id of the endstop hit. 0 = lower, 1 = upper.
        */
        function<void(int endstopReleased)> onEndstopReleased;

    //=================================================================================== SPEED
        /** @brief Maximum motor speed (abstract units). */
        float maxSpeed = 1.0f;

        /** @brief Minimum motor speed (abstract units). */
        float minSpeed = 0.1f;

        /** @brief Speed to use while homing (abstract units). */
        float homeSpeed = 0.25f;

        /** @brief Motor acceleration interval (abstract units). */
        float speedInterval = 0.001f;

        /** @brief Current motor speed (abstract units). */
        float speed = minSpeed;

    //======================================================================== GENERAL MOVEMENT
        /** @brief The number of stepper steps per output revolution. */
        uint32_t stepsPerRev;

        /** @brief How many units are moved with each output revolution. */
        float posPerRev;

        /** @brief The net stepper step.*/
        volatile long currentStep = 0;

        /** @brief How many steps are left in the current movement.*/
        volatile uint32_t remainingSteps = 0;

        /** @brief At what remaining step count to begin decelerating.*/
        volatile uint32_t slowStep = 0;

        /** @brief Is the stepper currently moving? */
        bool moving = false;

        /** @brief Should the motor output be reversed? */
        bool reverse = false;

    //================================================================================== TIMING
        /** @brief Recursive trigger for asynchronus interrupt driven stepping. */
        Timeout stepTimout;

    //===================================================================================== ISR
        void StepTimeoutHandler();
        void SpeedTimeoutHandler();
};


#endif