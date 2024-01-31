//=====[Libraries]=============================================================
#include "mbed.h"
#include "arm_book_lib.h"

//=====[Declaration of MACROS]===============
#define AUTO_MODE        0.5
#define DAYLIGHT_MODE    0.75
#define DUSK_MODE        0.65

//=====[Declaration and initialization of public global objects]===============
DigitalIn driverSeatDetector(D2);
DigitalIn ignitionButton(BUTTON1);

DigitalOut engineLED(LED2);
DigitalOut headlight1(D7);
DigitalOut headlight2(D6);

AnalogIn potentiometer(A0);
AnalogIn ldr(A2);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Declaration and initialization of public global variables]=============
float potentiometerReading = 0.0;
float ldrRead = 0.0;

int engineState = 0;
int check = 0;

//=====[Declarations (prototypes) of public functions]=========================
void inputsInit();
void outputsInit();
float ldrSensorRead();
float potentiometerRead();
void lightCheck(float reading);
void ignitionCheck();
void checkDelay();

//=====[Main function, the program entry point after power on or reset]========
int main(){
    //initialize inputs and outputs.
    inputsInit();
    outputsInit();
    //declare local variable that keeps track of the driver.
    int driverState = 0;
    while (true) {
        //if the driver sits down, set driverState to 1.
        if (driverSeatDetector){
            driverState = 1;
            //if the driver gets up after sitting down, set driverState to zero and exit the while loop.
            //if the ignition button is pressed while the driver is seated, start the ignition when the ignition
            //button is released.
            while (driverState == 1) {
                if (!driverSeatDetector){
                    driverState = 0;
                } else if (ignitionButton){
                    engineState = 2;
                    while (engineState == 2){
                        //start the ignition only when the ignition button is released. then set the driverState
                        //to zero so it doesn't matter if the driver leaves the car after the ignition is started.
                        if (!ignitionButton){
                            engineState = 1;
                            driverState = 0;
                        }
                    }
                }
            }
        }
        //while the ignition is running, turn on the engineLED and set the headlights on.
        while (engineState == 1){
            engineLED = ON;
            headlight1 = ON;
            headlight2 = ON;
            //continuously check to see if the ignition button is pressed again to turn off the ignition.
            ignitionCheck();
            //read and print out the potentiometer values to the serial terminal.
            potentiometerReading = potentiometerRead();
            char pStr[100];
            int pStringLength;
            sprintf (pStr, "Potentiometer: %.2f\r\n", potentiometerReading);
            uartUsb.write(pStr, strlen(pStr));
            //if the potentiometer is turned to a value larger than AUTO_MODE, then auto mode is selected.
            while (potentiometerReading > AUTO_MODE && engineState == 1) {
                //continue reading the potentiometer values in case the user unselects auto mode.
                potentiometerReading = potentiometerRead();
                //continuously get the ldr sensor values and check to see if the ignition button is being pressed.
                ignitionCheck();
                ldrRead = ldrSensorRead();
                lightCheck(ldrRead);
                //print the ldr sensor values to the terminal and run the function lightCheck.
                char ldrStr[100];
                int ldrStringLength;
                sprintf(ldrStr, "LDR: %.2f\r\n", ldrRead);
                uartUsb.write(ldrStr, strlen(ldrStr));
            }
        }
    }
}


//=====[Implementations of public functions]===================================

//set the buttons as pulldown.
void inputsInit()
{
    driverSeatDetector.mode(PullDown);
    ignitionButton.mode(PullDown);
}

//turn all the LEDs off.
void outputsInit()
{
    engineLED = OFF;
    headlight1 = OFF;
    headlight2 = OFF;
}

//get and return the ldr sensor value.
float ldrSensorRead()
{
    return ldr.read();
}

//get and return the potentiometer value.
float potentiometerRead(){
    return potentiometer.read();
}

//use the ldr sensor value to see if it is daylight or dusk and turn the LEDs on and off accodindly.
void lightCheck(float reading){
    if (ldrRead > DAYLIGHT_MODE){
        checkDelay();
        headlight1 = OFF;
        headlight2 = OFF;
    } else if (ldrRead < DUSK_MODE){
        checkDelay();
        //if the ignition button is pressed during this option, engineState will be set to zero and will return
        //to this function. Check to see if this has happened. If it hasn't, turn on the headlights.
        if (engineState == 1){
            headlight1 = ON;
            headlight2 = ON;
        }
    }
}

//if the ignitionButton is been pressed, turn off the headlights and the engineLED, set engineState to zero
//and check to 5.
void ignitionCheck(){
    if (ignitionButton){
        headlight1 = OFF;
        headlight2 = OFF;
        engineLED = OFF;
        engineState = 0;
        check = 5;
    }
}

//while the 2 second delay is happening, check to see if the ignitionButton has been pressed every 200 ms.
void checkDelay(){
    check = 0;
    while (check < 4) {
        ignitionCheck();
        delay(200);
        check++;
    }
}
