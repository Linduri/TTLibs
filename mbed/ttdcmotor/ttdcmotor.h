/**
*     _____ _____ ___     __  __     _           
*    |_   _|_   _|   \ __|  \/  |___| |_ ___ _ _ 
*      | |   | | | |) / _| |\/| / _ \  _/ _ \ '_|
*      |_|   |_| |___/\__|_|  |_\___/\__\___/_|                                                      
*
*
* @file TTEncoder.h
* @brief This file contains the definitions and delcarations associated with TTDcMotor.
*
* @author Ted Tooth
* @date 11 Feb 2021
*
* @copyright Ted Tooth 2021
*
*   TODO
*       - Should stop be mutex protected or just do it? Speed = safety?
*
*/

#ifndef TT_DC_MOTOR_H
#define TT_DC_MOTOR_H

#include "mbed.h"
#include "ttencoder.h"
#include "ttconstants.h"

class TTDcMotor{
    public:
        /*
        * @brief Create an asynchronus interrupt-driven dc motor.
        * @param en PWM pin to drive motor h-bridge.
        * @param A H-bridge A input.
        * @param B H-bridge B input.
        * @param period PWM period to use in seconds.
        * @param inaInbActiveLow (Optional) Are A and B channels active low?
        */
        TTDcMotor(PinName en, PinName A, PinName B, float period, bool inaInbActiveLow = false);

        /*
        * @brief Registers an encoder to use wiht the motor.
        * @param inA Encoder output A pin.
        * @param inB Encoder output B pin.
        * @param inAMode (Optional) Encoder output A pin mode.
        * @param inBMode (Optional) Encoder output B pin mode.
        * @returns TT_SUCCESS or negative error code.
        * @retval TT_MUTEX_TIMEOUT Timed out waiting for mutex lock.
        * @retval TT_OVERWROTE_ENCODER Overwrote existing encoder instance.
        */
        int RegisterEncoder(PinName inA, PinName inB, PinMode inAMode = PullDefault, PinMode inBMode = PullDefault);

        /*
        * @brief Moves the motor perpetually.
        * @param speed Percentage speed, (0 <= speed <= 1).
        * @param direction Speed clockwise or anti-clockwise.
        * @returns TT_SUCCESS or negative error code.
        * @retval TT_MUTEX_TIMEOUT Timed out waiting for mutex lock.
        * @retval TT_FLOORED_SPEED The speed parameter was less than 0, it was floored to 0.
        * @retval TT_CEILINGED_SPEED The speed parameter was greater than 1, it was ceilinged to 1.
        */
        int Spin(float speed, bool direction);

        /*
        * @brief Move the motor x encoder in direction.
        * @param speed 0% (0) to 100% (1) speed to move at.
        * @param x Number of encoder pulses to move. 
        * @param direction Direction to move in.
        * @returns TT_SUCCESS or negative error code.
        * @retval TT_MUTEX_TIMEOUT Timed out waiting for mutex lock.
        * @retval TT_NO_REGISTERED_ENCODER No encoder registered, use RegisterEncoder() to set one.
        * @retval TT_ALREADY_MOVING The motor is currently moving, wait until it has stopped.
        */
        int Move(float speed, int pulses, bool direction);

        /*
        * @brief Is the motor currently moving?
        * @returns True, false or negative error code.
        * @retval TT_MUTEX_TIMEOUT Timed out waiting for mutex lock.
        */  
        int IsMoving(void);

        /*
        * @brief Stops any motor movement.
        * @returns TT_DC_MOTOR_SUCCESS or negative error code.
        * @retval TT_MUTEX_TIMEOUT Timed out waiting for mutex lock.
        */
        int Stop(void);

        /*
        * @brief Register a callback for when motor movement has finished.
        * @param callback Callback to add.
        * @returns TT_SUCCESS or negative error code.
        * @retval TT_MUTEX_TIMEOUT Timed out waiting for mutex lock.
        * @retval TT_OVERWROTE_CALLBACK Overwrote existing callback.
        */
        int SetMoveEndedCallback(function<void()> callback);

        /* @brief Convenient contextual shortcut to TTConstants. */
        enum direction{clockwise = TT_CLOCKWISE, anticlockwise = TT_ANTICLOCKWISE};

    private:
        /* @brief Make this class thread safe by protecting members from simultaneous access. */
        Mutex mtx;

        /* @brief ISR callback for checking if a move is finished on encoder interrupt. */
        void MoveISR(void);

        /*
        * @brief Set the h-bridge A and B channels to choos emotor direction.
        * @param dir Direction to spin. Can be "thisObject".clockwise or "thisObject".anticlockwise.
        * @returns TT_SUCCESS.
        */
        int SetDirection(bool dir);

        /* @brief H-bridge pwm pin. */
        PwmOut pwm;

        /* @brief H-bridge A channel enable pin. */
        DigitalOut A;

        /* @brief H-bridge B channel enable pin. */
        DigitalOut B;

        /* @brief Have a encoder unique top this motor if required. */
        TTEncoder *encoder = 0;

        /* @brief Number of interrupts that indicates a move end location. */
        int endInterrupts = 0;

        /* @brief Store if the motor is currently moving. */
        bool moving = false;

        /* @brief Is the h-bridge enable active low? */
        bool enActiveLow;

        /* @brief Are the h-bridge A and B channels active low? */
        bool inaInbActiveLow;

        /* @brief Store the move ended callback. */
        function<void()> onMoveEndedCallback = 0;
};


#endif