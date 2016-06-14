#include <DCF77.h>       //https://github.com/thijse/Arduino-Libraries/downloads
#include <Time.h>        //http://www.arduino.cc/playground/Code/Time
#include "LEDData.h"

#define DCF_PIN 2           // Connection pin to DCF 77 device
#define DCF_INTERRUPT 0     // Interrupt number associated with pin

#define ARRAYSIZE 8

#define dataPin 8        //Define which pins will be used for the Shift Register control
#define latchPin 10
#define clockPin 9
#define pwmPin 11
#define ButtonPin 3

const long debouncing_time = 15; //Debouncing Time in Milliseconds

DCF77 DCF = DCF77(DCF_PIN,DCF_INTERRUPT);
volatile byte buttonState = HIGH;
volatile unsigned long last_micros;
volatile bool blinkState = true;

void setup()
{
    setupPinsForLEDs();           //Setting up pins for LEDs

    turnAllLedsOff();             //Turn all LEDs off
         
    startClockSynchronisation();  //Start time synchronisation and play synchronisation animation
    
    setupPinForButton();          //Seting up pin for Button and attach interrupt

    buttonState = HIGH;           //Reset ButtonState
    
}

void turnAllLedsOff()
{
    byte output[8];
          
    for(int j = 0; j<8;j++)
    {
      output[j] = pgm_read_byte(&(AllLEDsOff[j]));  //Read Data of program memory     
    }

    ShiftByteArrayOut(output);
    ShiftByteArrayOut(output); 
}

void setupPinsForLEDs()
{
    pinMode(dataPin, OUTPUT); //Configure each IO Pin
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(pwmPin, OUTPUT);
}


void startClockSynchronisation()
{
    DCF.Start();                      //Starts time-sync
    
    setSyncInterval(30);              //Sets sync-interval to 30 sec
    setSyncProvider(getDCFTime);      //Defines the methode for time-sync
      
    while(timeStatus()== timeNotSet)  //Waits until time is synced
    { 
      delay(100);
    }
}

unsigned long getDCFTime()
{ 
    time_t DCFtime = DCF.getTime();
    return DCFtime;
}

void PlaySynchronisationAnimation(int Delay)
{
    for(int i = 0; i <120; i++)
    {
        byte output[8];
        
        for(int j = 0; j<8; j++)
        {
          output[j] = pgm_read_byte(&(SynchronisationAnimation[i][j]));     //Read Data of program memory      
        }
        ShiftByteArrayOut(output);
        delay(Delay);
    }
}


void setupPinForButton()
{
    pinMode(ButtonPin, INPUT_PULLUP); //Configure the button pin. Use internal pull-up 
    attachInterrupt(digitalPinToInterrupt(ButtonPin), ButtonInterrupt, CHANGE); //Configure interrupt for button pin. Sets interrupt on CHNAGE
}

void ButtonInterrupt()
{
    //Debouncing
    if((long)(micros() - last_micros) >= debouncing_time * 1000) 
    {
      ChangeState();
      last_micros = micros();
    }
}

void ChangeState()
{
  buttonState = !buttonState; //inverts buttonState
}

void loop()
{
    if(buttonState == LOW)
    {
        ShowTime(blinkState);
        blinkState = !blinkState;
        delay(250);
    }
    else
    {
        blinkState = true;
        IdleMode();
    }
    
}

void ShowTime(bool showMinute)
{
    int Minute = minute();  //get the minute of the current time
    int Hour = hour();      //get the hour of the current time
    
    byte Time[8];
    byte HourByteArray[8];
    byte MinuteByteArray[8];

    ClearByteArray(MinuteByteArray); //reset arrays
    ClearByteArray(HourByteArray);
    ClearByteArray(Time);
    
    for(int i = 0; i<8;i++)
    {
      if(showMinute == true)
      {
        //Read Data of program memory 
        MinuteByteArray[i] = pgm_read_byte(&(SingleLED[pgm_read_byte(&MinuteMap[Minute])][i]));
      }
      //Read Data of program memory 
      HourByteArray[i] = pgm_read_byte(&(SingleLED[pgm_read_byte(&HourMap[Hour])][i]));
    }
    
    Union(HourByteArray, MinuteByteArray, Time); //Union minute and hour bits to time
    ShiftByteArrayOut(Time);
}

void IdleMode()
{
    byte output[8];
    for(int j = 0; j<8;j++)
    {
        output[j] = pgm_read_byte(&(AllLEDsOn[j]));          
    }
    ShiftByteArrayOut(output);
}

//Union two byte array
void Union(const byte* first, const byte* second, byte* output)
{
    int i;
    for(i = 0; i <ARRAYSIZE; i++)
    {
      output[i] = first[i]|second[i];
    }
}

void ShiftByteArrayOut(const byte* byteArray)
{
    digitalWrite(latchPin, HIGH);           //Pull latch HIGH to start sending data             
    for(int i = ARRAYSIZE; i >= 0; i--)
    {
      shiftOut(dataPin, clockPin, LSBFIRST, byteArray[i]);//Send the data
    }
    digitalWrite(latchPin, LOW);            //Pull latch LOW to stop sending data     
}

//Copy the content of byte array "from" to byte array "to"
void CopyByteArray(const byte* from, byte* to)
{
    for(int i = 0; i <ARRAYSIZE; i++)
    {
        to[i] = from[i];
    }
}

//Resets a byte array
void ClearByteArray(byte* array)
{
    for(int i = 0; i < ARRAYSIZE; i++)
    {
        array[i] = 0;
    }
}
