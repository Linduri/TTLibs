/**
*     _____ _____ ___                 _         
*    |_   _|_   _| __|_ _  __ ___  __| |___ _ _ 
*      | |   | | | _|| ' \/ _/ _ \/ _` / -_) '_|
*      |_|   |_| |___|_||_\__\___/\__,_\___|_|  
*                                                     
*
* @file TTEncoder.h
* @brief This file contains the definitions and delcarations associated with TTEncoder.
*
* @author Ted Tooth
* @date 11 Feb 2021
*
* @copyright Ted Tooth 2021
*/

#ifndef TT_ENCODER_H
#define TT_ENCODER_H

#include "mbed.h"
#include "ttconstants.h"

class TTEncoder{
    public:
        /*
        * @brief Create an asynchronus interrupt-driven encoder object to track shaft interrupts.
        * @param inA Encoder A output.
        * @param inB Encoder B output.
        * @param inAMode (Optional) Pin mode for the encoder output A input.
        * @param inBMode (Optional) Pin mode for the encoder output B input.
        */
        TTEncoder(PinName inA, PinName inB, PinMode inAMode = PullDefault, PinMode inBMode = PullDefault);

        /*
        * @brief Get the net number of inA and inB have risen AND fallen. 
        * @returns Net inA and inB count or negative error code.
        * @retval TT_MUTEX_TIMEOUT Timed out waiting for mutex lock.
        */
        int getInterruptCount(void);

        /*
        * @brief Get the number of times inA and inB have risen AND fallen in a specific direction.
        * @param direction Can be "thisObject".clockwise or "thisObject".anticlockwise.
        * @returns Net inA and inB count in a direction or negative error code.
        * @retval TT_MUTEX_TIMEOUT Timed out waiting for mutex lock.
        */
        int getInterruptCount(int direction);
        
        /*
        * @brief Reset all interrupt counts to zero.
        * @returns TT_SUCCESS or negative error code.
        * @retval TT_MUTEX_TIMEOUT Timed out waiting for mutex lock.
        */
        int Reset(void);

        /*
        * @brief Set a single void callback function to be called on inA or inB interrupt.
        * This function will OVERWRITE any exisitng callback.
        * @param callack Function to call. 
        * @returns TT_SUCCESS or negative error code.
        * @retval TT_MUTEX_TIMEOUT Timed out waiting for mutex lock.
        * @retval TT_OVERWROTE_CALLBACK TO set this callback, the existing callback had to be overwritten.
        */
        int SetOnInterruptCallback(function<void()> callback);

        /* @brief Convenient contextual shortcut to TTConstants. */
        enum direction{clockwise = TT_CLOCKWISE, anticlockwise = TT_ANTICLOCKWISE};

    private:
        /* @brief Make this class thread safe by protecting members from simultaneous access. */
        Mutex mtx;

        /* @brief Encoder output A input. */
        InterruptIn *inA = 0;

        /* @brief Encoder output B input. */
        InterruptIn *inB = 0;

        /* @brief Record the number of interrupts in clockwise and anticlockwise directions. */
        volatile uint32_t changeCount[2] = {0};
        
        /* @brief Save the current state in the encoder wave sequence for the next interrupt. */
        int state = 0;

        /* @brief Update the state machine and record interrupt in a direction. */
        void inARiseISR(void);

        /* @brief Update the state machine and record interrupt in a direction. */
        void inAFallISR(void);

        /* @brief Update the state machine and record interrupt in a direction. */
        void inBRiseISR(void);

        /* @brief Update the state machine and record interrupt in a direction. */
        void inBFallISR(void);

        /* @brief Store the interrupt callback. */
        function<void()> onInterruptCallback = 0;
};


#endif