#ifndef TT_ENCODER_H
#define TT_ENCODER_H

#define TT_ENCODER_SUCCESS 0

#include "mbed.h"

class TTEncoder{
    public:
        TTEncoder(PinName inA, PinName inB, PinMode inAMode = PullDefault, PinMode inBMode = PullDefault);

        int getChangeCount(void);
        int getChangeCount(int direction);

        int Reset(void);

        enum direction{clockwise, anticlockwise};
        enum risefall{rise, fall};
        enum in{A, B};

    private:
        InterruptIn *inA = 0;
        InterruptIn *inB = 0;

        volatile uint32_t pulseCount[2] = {0};
        volatile uint32_t changeCount[2] = {0};
        
        int state = 0;
        enum states{waiting, aUp, aDown, bUp, bDown};

        void StateISR(int in, int riseFall);

        void inARiseISR(void);
        void inAFallISR(void);

        void inBRiseISR(void);
        void inBFallISR(void);
};


#endif