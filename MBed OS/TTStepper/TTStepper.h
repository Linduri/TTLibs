/**
*        _____ _____ ___ _                          
*       |_   _|_   _/ __| |_ ___ _ __ _ __  ___ _ _ 
*         | |   | | \__ \  _/ -_) '_ \ '_ \/ -_) '_|
*         |_|   |_| |___/\__\___| .__/ .__/\___|_|  
*                               |_|  |_|            
*
*
* @file ttstepper.h
* @brief This file contains the definitions and delcarations associated with TTStepper.
*
* @author Ted Tooth
* @date 31 Jan 2021
*
* Update log:
*
*   Rev 1.0 - All initial functionality added.
*
*
* To do
*   - Acceleration and velocity needs verification.
*   - Add chirp function.
*   - Fix endstop ISR Queue overflow error (maybe just dodgy wiring?).
*   - Add ability for zero acceleration.
*
* @copyright Ted Tooth 2021
*/

#ifndef TTSTEPPER_H
#define TTSTEPPER_H

#include "mbed.h"
#include <chrono>

using namespace chrono;

#define TTSTEPPER_MUTEX_TIMEOUT 50ms
#define TTSTEPPER_DEFAULT_RPS 1
#define TTSTEPPER_DEFAULT_RPSS 1
#define TTSTEPPER_DEFAULT_MIN_RPS 0.005f

#define TTSTEPPER_DEFAULT_SLIDE_UNITS_PER_REV 0.1f 
#define TTSTEPPER_DEFAULT_SLIDE_UPS 0.1
#define TTSTEPPER_DEFAULT_SLIDE_UPSS 1

#define TTSTEPPER_CLOCKWISE true
#define TTSTEPPER_ANTICLOCKWISE false

#define TTSTEPPER_HOMED_FLAG (0x1 << 0)
#define TTSTEPPER_TRAVEL_ENDED_FLAG (0x1 << 2)
#define TTSTEPPER_DEFAULT_HOMING_TIMEOUT 3s

#define TTSTEPPER_ONE_ENDSTOP 1
#define TTSTEPPER_DUAL_ENDSTOP 2
#define TTSTEPPER_SUCCESS 0
#define TTSTEPPER_ERROR_NO_FREE_ENDSTOPS -1
#define TTSTEPPER_ERROR_MUTEX_TIMEOUT -2
#define TTSTEPPER_ERROR_ALREADY_TRAVELLING - 3
#define TTSTEPPER_ERROR_INVALID_ENDSTOP_ID - 4
#define TTSTEPPER_ERROR_ENDSTOP_HIT - 5
#define TTSTEPPER_ERROR_HOMING_TIMEOUT -6
#define TTSTEPPER_ERROR_SLIDE_UNITS_PER_STEP_NOT_SET -7
#define TTSTEPPER_ERROR_TRAVEL_WAIT_TIMEOUT -8
#define TTSTEPPER_ALREADY_HOMING -9

class TTStepper{
    public:
        /**
        * @brief Creates a stepper motor.
        * @param en Enable pin.
        * @param step Step pin.
        * @param dir Direction pin.
        * @param stepsPerRev The number of steps per revolution.
        * @param metersPerRev (Optional) The distance moved per revolution for linear drives.
        * @param invertLinearDirection (Optional) Flip the direction of movement in linear mode.
        */
        TTStepper(PinName en, PinName step, PinName dir, int stepsPerRev, float slideUnitsPerRev = TTSTEPPER_DEFAULT_SLIDE_UNITS_PER_REV);

        /**
        * @brief Get the current angle of the stepper.
        * @returns The current stepper angle relative to angle at startup OR latest home.
        */
        float GetDegrees(void);

        /**
        * @brief Get the lifetime angle of the stepper.
        * @returns The lifetime stepper angle in degrees relative to angle at startup OR latest home.
        */
        float GetLifetimeDegrees(void);

        /**
        * @brief Move a specified angle using the specified direction.
        * @param degrees Degrees to move.
        * @param direction Direction to rotate TTSTEPPER_CLOCKWISE or TTSTEPPER_ANTICLOCKWISE.
        */
        int Rotate(float degrees, bool direction);

        /**
        * @brief Move to a specified angle in the shortest direction.
        * @param degs Degrees to rotate. TTSTEPPER_CLOCKWISE or TTSTEPPER_ANTICLOCKWISE.
        */
        int RotateTo(float degrees);

        /**
        * @brief Move to a specified angle in the specified direction.
        * @param degrees Degrees to move.
        * @param direction Direction to rotate. TTSTEPPER_CLOCKWISE or TTSTEPPER_ANTICLOCKWISE.
        */
        int RotateTo(float degrees, bool direction);

        /**
        * @brief Move to a specified absolute angle in the shortest direction.
        * @param degrees Degrees to move.
        */
        int SetRotation(float degrees);
       
        /**
        * @brief Get the current linear position along the slide.
        * @returns Units along the slide or a negative error code.
        */
        float GetSlidePositon(void);

        /**
        * @brief Get the current linear position along the slide.
        * @returns Success or a negative error code.
        * @retval TTSTEPPER_SUCCESS Successfully set slide invert.
        * @retval TTSTEPPER_ERROR_MUTEX_TIMEOUT Failed to acquire stepperLock.
        */
        int InvertSlide(bool invert);

        /**
        * @brief Move a specified distance using the specified direction.
        * @param distance Distance to move.
        * @param direction Direction to rotate. TTSTEPPER_CLOCKWISE or TTSTEPPER_ANTICLOCKWISE.
        */
        int Slide(float distance, bool direction);

        /**
        * @brief Move to a specified distance in the shortest direction.
        * @param position Position along the slide to move to.
        */
        int SlideTo(float position);

        /**
        * @brief Register an endstop. Registers lower endstop first.
        * @param pin Desired interupt pin.
        * @param mode Desired pin mode.
        * @returns Id of the newly registered interupt.
        * @retval 0 Lower interrupt added.
        * @retval 1 Upper interrupt added.
        * @retval TTSTEPPER_ERROR_NO_FREE_ENDSTOPS No free endstops spaces.
        */
        int RegisterEndstop(PinName pin, PinMode mode = PullDefault);
 
        /**
        * @brief Clears any endstop hit flags.
        * @returns Success.
        */
        int ClearEndstopHit(void);

        /**
        * @brief Flip the direction of stepper rotation.
        * @param invert True or false.
        */
        int Invert(bool invert);

        /**
        * @brief Flip the direction 
        * @param invert True (active high) or false (active low).
        */
        int InvertEndstops(bool invert);

        //Homes to lower, always moves anti-clockwise.
        /**
        * @brief Home stepper to the lower endstop.
        * @param timeout Length to try home for in chrono.
        * @param direction Direction to home, default is anti-clockwis, try to use this orientation to avoid unexpected behaviour.
        * @returns Success or negative error code.
        * @retval TTSTEPPER_SUCCESS Successully homed.
        * @retval TTSTEPPER_HOMING_TIMEOUT Failed to home in specified time.
        */
        int Home(milliseconds timeout = duration_cast<milliseconds>(TTSTEPPER_DEFAULT_HOMING_TIMEOUT), bool direction = TTSTEPPER_ANTICLOCKWISE);

        /**
        * @brief Set the maximum angular velocity.
        * @param rpsMax Desired maximum revolutions per second.
        * @param rpsMin (Optional) Desired minimum revolutions per second.
        * @retval TTSTEPPER_SUCCESS Successfuly set maximum angular velocity. 
        */
        int SetMaxRPS(float rpsMax, float rpsMin = TTSTEPPER_DEFAULT_MIN_RPS);

        /**
        * @brief Set the maximum angular acceleration.
        * @param rps Desired maximum angular acceleration in revolutions per second^2.
        * @retval TTSTEPPER_SUCCESS Successfuly set maximum angular acceleration. 
        */
        int SetRPSS(float rpss);  

        /**
        * @brief Set the slide units per stepper step.
        * @param unitsPerStep Desired POSITIVE units per step.
        * @retval TTSTEPPER_SUCCESS Successfuly set slide unit per step. 
        * @retval TTSTEPPER_MUTEX_TIMEOUT Failed to acquire stepperLock
        */
        int SetSlideUnitsPerStep(float unitsPerStep);

        /**
        * @brief Set the slide linear velocity.
        * @param upsMax Desired POSITIVE maximum linear velocity in units per second.
        * @param upsMin (Optional) Desired POSITIVE minimum linear velocity in units per second.
        * @retval TTSTEPPER_SUCCESS Successfuly set linear velocity. 
        * @retval TTSTEPPER_MUTEX_TIMEOUT
        */
        int SetSlideUPS(float upsMax, float upsMin = -1);

        /**
        * @brief Set the slide linear acceleration.
        * @param upss Desired POSITIVE maximum linear acceleration in units per second^2.
        * @retval TTSTEPPER_SUCCESS Successfuly set linear acceleration. 
        * @retval TTSTEPPER_MUTEX_TIMEOUT
        */
        int SetSlideUPSS(float upss);

        /**
        * @brief Wait for the current stepper travel to end.
        * @param timeoutMillis (Optional) Wait for this many milliseconds before timing out.
        * @retval TTSTEPPER_SUCCESS Stepper finished travelling.
        * @retval TTSTEPPER_ERROR_TRAVEL_WAIT_TIMEOUT Stepper did not home in the specified time.
        */
        int WaitForTravelEnd(int timeoutMillis = -1);

        /** 
        * @brief Enable or disable the stepper motor. Sets the motor enable pin.
        * @param enable True or false to enable the stepper motor.
        * @returns TTSTEPPER_SUCCESS
        */
        int Enable(bool enable);

        /** 
        * @brief Is the stepper enable pin active? 
        * @returns True or false.
        * @retval true Stepper enable pin is active.
        * @retval false Stepper enable pin is not active.
        */
        int Enabled(void);

        /** 
        * @brief Enable the stepper motor. Sets the motor enable pin active. 
        * @returns TTSTEPPER_SUCCESS
        */
        int Enable(void);

        /** 
        * @brief Disable the stepper motor. Sets the motor enable pin not active. 
        * @returns TTSTEPPER_SUCCESS
        */
        int Disable(void);

        /** 
        * @brief Get the id of the last endstop hit.
        * @returns Last hit endstop id.
        * @retval 0 No endstop hit yet.
        * @retval 1 Lower.
        * @retval 2 Upper.
        */
        int LastEndstopHit(void);

        /** 
        * @brief Get the id of the last endstop released.
        * @returns Last released endstop id.
        * @retval 0 No endstop released yet.
        * @retval 1 Lower endstop.
        * @retval 2 Upper endstop.
        */
        int LastEndstopReleased(void);

        /**
        * @brief Gets if the stepper is currently homing. 
        * @returns If the stepper is homing or a negative error code.
        * @retval 0 Stepper NOT homing.
        * @retval 1 Stepper homing.
        * @retval TTSTEPPER_ERROR_MUTEX_TIMEOUT Failed to acquire stepperLock mutex.
        */
        int GetHoming(void);

        /**
        * @brief Enable or disable active stepper braking.
        * @returns Success or a negative error code.
        * @retval TTSTEPPER_SUCCESS Successfully set active braking.
        * @retval TTSTEPPER_ERROR_MUTEX_TIMEOUT Failed to acquire stepperLock mutex.
        */
        int SetActiveBraking(bool enable);

        /**
        * @brief Sets the onEndstopHit callback function.
        * @warning Callback will run in ISR context so defer to a thread.
        * @warning Overwrites the existing callback.
        * @returns Success or a negative error code.
        * @retval TTSTEPPER_SUCCESS Successfully set callback.
        * @retval TTSTEPPER_ERROR_MUTEX_TIMEOUT Failed to acquire stepperLock mutex.
        */
        int SetEndstopHitCallback(function<void(int)> callback);

        /**
        * @brief Sets the onEndstopreleased callback function. 
        * @warning Callback will run in ISR context so defer to a thread.
        * @warning Overwrites the existing callback.
        * @returns Success or a negative error code.
        * @retval TTSTEPPER_SUCCESS Successfully set callback.
        * @retval TTSTEPPER_ERROR_MUTEX_TIMEOUT Failed to acquire stepperLock mutex.
        */
        int SetEndstopReleasedCallback(function<void(int)> callback);

        /**
        * @brief Offsets the slide zero point by X units.
        * @param offset Number of units to offset by. Can be positive or negative.
        * @returns Success or a negative error code.
        * @retval TTSTEPPER_SUCCESS Successfully set callback.
        * @retval TTSTEPPER_ERROR_MUTEX_TIMEOUT Failed to acquire stepperLock mutex.
        */
        int SetSlideOffset(float offset);

    private:

    //================================================================================= GENERAL
        /** @brief When braking, should the motor be powered? */
        bool activeBraking = true;

        /** 
        * @brief Asynchronusly step in a given direction
        * @param steps Number of steps to take.
        * @param direction Direction to step.
        * @returns Success or negative error code.
        * @retval TTSTEPPER_SUCCESS Successfully begun asynchronus stepping.
        * @retval TTSTEPPER_ERROR_ALREADY_TRAVELLING Stepper already stepping.
        * @retval TTSTEPPER_ERROR_ENDSTOP_HIT Endstop has been hit, needs to be cleared first.
        */
        int Step(uint32_t steps, bool direciton);

        /** @brief ISR handler for stepping. */
        void StepISR(void);

    //================================================================================== TIMING
        /** @brief Recursive trigger for asynchronus interrupt driven stepping. */
        Timeout timeout;

        /** @brief Make variable access thread safe. */
        Mutex stepperLock;

        /** @brief Allow thread safe waiting for the stepper. */
        EventFlags eventFlags;

    //==================================================================================== GPIO
        /**  @brief Instance enable, step and direction pins for controlling the stepper. */
        DigitalOut en, step, dir;

        /** @brief Is the stepper driver enable pin active low? */
        bool enActiveLow = true;

    //================================================================================ ENDSTOPS  
        /** @brief Instance lower and upper endstops. */
        InterruptIn *lowerEndstop, *upperEndstop = 0;

        /** @breif Should endstop inputs be inverted? */
        bool invertEndstops = false;

        /** @brief Report if currently homing, used to override step remaining count. */
        volatile bool homing = false;

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

        /** 
        * @brief Logic for handling endstop interupts. 
        * @param id Endstop to report. lower (0) or upper (1).
        * @param riseOrFall Is this interrupt a rise or fall event?
        */
        void Endstop(int id, bool riseOrFall);

        /** @brief ISR handler for lower endstop rise. */
        void LowerEndstopRiseISR(void);

        /** @brief ISR handler for lower endstop fall. */
        void LowerEndstopFallISR(void);

        /** @brief ISR handler for upper endstop rise. */
        void UpperEndstopRiseISR(void);

        /** @brief ISR handler for upper endstop fall. */
        void UpperEndstopFallISR(void);

    //======================================================================== GENERAL MOVEMENT
        /** @brief The net count of steps taken (positive is clockwise, negative anti-clockwise). */
        volatile int currentStep = 0;
        
        /** @brief Remaining steps to take in this step sequence. */
        volatile int remainingSteps = 0;
       
        /** @brief Number of steps it takes to reach maximum rotary velocity. */
        uint32_t accelerationSteps = 0;
        
        /** @brief The remainingStepes count to begin deceleration at. */
        uint32_t decelerationStep = 0;

        /** @brief Is the stepper currently moving? */
        bool travelling = false; 

        /** @brief Should stepper rotation be reversed? */
        bool invertRotation = false;

    //========================================================================= ROTARY MOVEMENT        
        /** @brief Number of steps per complete revolution of the stepper. */
        int stepsPerRevolution;

        /** @brief Number of degrees rotated per step. */
        float degsPerStep;

        /** @brief The current rotary velocity in revolutions per second. */
        volatile float rps = minRps;

        /** @brief Maximum rotary velocity in revolutions per second. */
        float rpsMax = 1.0f;

        /** @brief Rotary acceleration in revolutions per second^2. */
        float rpss = 1.0f;

        /** @brief Rotary acceleration adjusted for calculations in revolutions per second^2. */
        float rpssAdjusted = 1.0f;

        /** @brief  Acceleration time in seconds. */
        float rpssT = 1.0f;
        
        /** @brief The slowest rps to ever move the motor. Used when acceleratring from stationary. */
        float minRps = TTSTEPPER_DEFAULT_MIN_RPS;

        /** @brief Pre-calculated acceleration increment. */
        float rpsInterval = 0.0f;

        /**
        * @brief Gets the number of revolutions the motor has done.
        * @returns Sum of total clockwise rotations - total anticlockwise rotations.
        */
        float GetRevs(void);

    //========================================================================= LINEAR MOVEMENT
        /** @brief Maximum linear velocity in units per second. */
        float upsMax = TTSTEPPER_DEFAULT_SLIDE_UPS;

        /** @brief Maximum linear acceleartion in units per second^2. */
        float upssMax = TTSTEPPER_DEFAULT_SLIDE_UPSS;
        
        /** @brief Number of linear units moved per step. */
        float slideUnitsPerStep;

        /** @brief Invert the direction of slide movement. */
        bool invertSlide = false;

        /** @brief Offset the slide zero point. */
        float slideOffset = 0.0f;
};


#endif