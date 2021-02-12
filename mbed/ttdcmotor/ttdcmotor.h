#ifndef TT_DC_MOTOR_H
#define TT_DC_MOTOR_H

#define TT_DC_MOTOR_SUCCESS 0
#define TT_DC_MOTOR_NO_REGISTERED_ENCODER -1
#define TT_DC_MOTOR_OVERWRITTEN_ENCODER -2
#define TT_DC_MOTOR_ALREADY_MOVING -3
#define TT_DC_MOTOR_FLOORED_SPEED -4
#define TT_DC_MOTOR_CEILINGED_SPEED -5
#define TT_DC_MOTOR_CALLBACK_OVERWRITTEN -6

#include "mbed.h"
#include "ttencoder.h"

class TTDcMotor{
    public:
        TTDcMotor(PinName pwm, PinName hA, PinName hB, PinName hEn = NC);

        /* @breif Useful definitions for direction. */
        enum direction{clockwise, anticlockwise};

        /*
        * @brief Registers an encoder to use wiht the motor.
        * @param inA Encoder output A pin.
        * @param inB Encoder output B pin.
        * @param inAMode (Optional) Encoder output A pin mode.
        * @param inBMode (Optional) Encoder output B pin mode.
        * @returns TT_ENCODER_SUCCESS or negative error code.
        * @retval TT_DC_MOTOR_OVERWRITTEN_ENCODER Over wrote existing encoder instance.
        */
        int RegisterEncoder(PinName inA, PinName inB, PinMode inAMode = PullDefault, PinMode inBMode = PullDefault);

        /*
        * @brief Moves the motor perpetually.
        * @param speed Percentage speed, (0 <= speed <= 1).
        * @param direction Speed clockwise or anti-clockwise.
        * @returns TT_DC_MOTOR_SUCCESS
        */
        int Spin(float speed, bool direction);

        /*
        * @brief Move the motor x encoder in direction.
        * @param speed 0% (0) to 100% (1) speed to move at.
        * @param x Number of encoder pulses to move. 
        * @param direction Direction to move in.
        * @returns TT_ENCODER_SUCCESS or negative error code.
        * @retval TT_DC_MOTOR_NO_REGISTERED_ENCODER No encoder registered, use RegisterEncoder() to set one.
        * @retval TT_DC_MOTOR_ALREADY_MOVING The motor is currently moving, wait until it has stopped.
        */
        int Move(float speed, int pulses, bool direction);

        /*
        * @brief Is the motor currently moving?
        * @returns True or false
        */  
        int IsMoving(void);

        /*
        * @brief Stops any motor movement.
        * @returns TT_DC_MOTOR_SUCCESS
        */
        int Stop(void);

        /*
        * @brief Register a callback for when motor movement has finished.
        * @param callback Callback to add.
        * @returns TT_ENCODER_SUCCESS or negative error code.
        * @retval TT_DC_MOTOR_CALLBACK_OVERWRITTEN Overwrote existing callback.
        */
        int SetMoveEndedCallback(function<void()> callback);

        int SetDirection(bool dir);

    private:

        /*
        * @brief ISR callback for checking if a move is finished.
        */
        void MoveCallback(void);

        AnalogOut pwm;
        DigitalOut hA;
        DigitalOut hB;
        DigitalOut hEn;

        TTEncoder *encoder = 0;

        int endPulses = 0;
        bool moving = false;

        function<void()> onMoveEndedCallback = 0;
};


#endif