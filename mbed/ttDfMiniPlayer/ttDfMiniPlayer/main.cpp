#include "mbed.h"
#include "DFRobotDFPlayerMini.h"


BufferedSerial dfMiniPlayer(PA_0, PinName::NC); //TX RX
DFRobotDFPlayerMini myDFPlayer;

DigitalIn userButton(USER_BUTTON);

// main() runs in its own thread in the OS
int main()
{
    debug("Booted!\r\n");

    // Set desired properties (9600-8-N-1).
    dfMiniPlayer.set_baud(9600);

    myDFPlayer.begin(dfMiniPlayer);

    myDFPlayer.volume(25);
    // myDFPlayer.play(1);  //Play the first mp3

    while(1){
        if(userButton){
            myDFPlayer.next();
        }

        ThisThread::sleep_for(50ms);
    };


}

