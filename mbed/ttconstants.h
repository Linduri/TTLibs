/**
*     _____ _____ ___             _            _      
*    |_   _|_   _/ __|___ _ _  __| |_ __ _ _ _| |_ ___
*      | |   | || (__/ _ \ ' \(_-<  _/ _` | ' \  _(_-<
*      |_|   |_| \___\___/_||_/__/\__\__,_|_||_\__/__/                                          
*                                                     
*
* @file TTEncoder.h
* @brief This file contains the definitions of all error codes used throught TTLibs.
*
* @author Ted Tooth
* @date 12 Feb 2021
*
* @copyright Ted Tooth 2021
*/

#ifndef TT_ERROR_CODES_H
#define TT_ERROR_CODES_H

#include "mbed.h"

#define TT_DEFAULT_MUTEX_TIMEOUT 50ms

enum ttDirection{TT_CLOCKWISE, TT_ANTICLOCKWISE};

enum ttErrors{

    TT_OVERWROTE_CALLBACK = INT_MIN,

    //Encoders
    TT_NO_REGISTERED_ENCODER,
    TT_OVERWROTE_ENCODER,

    //Motors
    TT_ALREADY_MOVING,
    TT_FLOORED_SPEED,
    TT_CEILINGED_SPEED,

    //Threads
    TT_MUTEX_TIMEOUT,
    TT_SUCCESS = 0                         //Should always be zero element.
};  

#endif