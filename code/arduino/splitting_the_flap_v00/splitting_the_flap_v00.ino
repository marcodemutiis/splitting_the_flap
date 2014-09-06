/*

  SPLITTING THE FLAP - version 00

  created by Marco De Mutiis 
  2012 - 2014
  more info: www.marcodemutiis.com

*/

//////////////////////////////////////////////////////
//stepper connection to Easy Driver 
//red to A left
//blue to A right
//black to B left
//green to B right
//////////////////////////////////////////////////////

//connections to the Easy Driver
#define DIR_PIN 2      //direction pin 
#define STEP_PIN 3     //step pin
//#define SLEEP_PIN 9
#define ENABLE_PIN 12  //enable pin


char inByte;

//timer variables to turn off the motor if it doesn't move for more than 30 seconds
boolean sleep = true;
unsigned long currentMillis;
long previousMillis = 0;        // will store last time LED was updated
long interval = 30000;           // interval at which to blink (milliseconds)


int tot = 40;
int howManySteps;
int lastPos = 39;

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: SETUP
void setup() { 
  pinMode(DIR_PIN, OUTPUT); 
  pinMode(STEP_PIN, OUTPUT);
  //pinMode(SLEEP_PIN, OUTPUT); 
  pinMode(ENABLE_PIN, OUTPUT); 

  //digitalWrite(SLEEP_PIN, LOW);
  digitalWrite(ENABLE_PIN, HIGH); //disable all pins of A397 chip
  
  Serial.begin(9600);
} 

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: LOOP
void loop(){ 

  currentMillis = millis();

  if(Serial.available() > 0){

    inByte = Serial.read(); //store incoming char


    char letters[40] = { 
      '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '.', '-', '/', ' '   
    };

    for(int newPos =0; newPos<tot; newPos++){
      if(inByte == letters[newPos]){

        //if new letter is different from current letter
        if(lastPos != newPos){ 

          //-----------------------------resetTimer
          resetTimer();
          //-----------------------------wakeUp
          if(sleep) wakeUp();

          //calulate how many steps to new position////////////
          if (lastPos < newPos) howManySteps = newPos - lastPos; 
          else howManySteps = tot - (lastPos - newPos);
          /////////////////////////////////////////////////////


          //print info to monitor///////////////////////////////
          Serial.print("NewPos = ");
          Serial.println(newPos);

          Serial.print("lastPos = ");
          Serial.println(lastPos);

          Serial.print("steps = ");
          Serial.println(howManySteps);
          /////////////////////////////////////////////////////


          //motor spins for howManySteps///////////////////////
          for(int stepNext = 0; stepNext < howManySteps; stepNext++){
            rotate(160, 1.);// 160 = 1/10 of a full revolution of the 1st gear = 1/40 of the last gear = 1 split flap character step //1. = MAX SPEED (slower = stronger)
          } 
          /////////////////////////////////////////////////////

          lastPos = newPos; 
        }     
      }

    }

  }


  if(!sleep){
    // digitalWrite(SLEEP_PIN, LOW);
    if(currentMillis - previousMillis > interval) {   //if the motor hasnt moved in more than 30 secs
      //-----------------------------resetTimer
      resetTimer(); 
      //-----------------------------goToSleep
      goToSleep();
    }
  }
  else(Serial.println("zzzz"));
}



//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: ROTATE
void rotate(int steps, float speed){ 
  //rotate a specific number of microsteps (8 microsteps per step) - (negitive for reverse movement)
  //speed is any number from .01 -> 1 with 1 being fastest - Slower is stronger
  int dir = (steps > 0)? HIGH:LOW;
  steps = abs(steps);

  digitalWrite(DIR_PIN,dir); 

  float usDelay = (1/speed) * 70;

  for(int i=0; i < steps; i++){ 
    digitalWrite(STEP_PIN, HIGH); 
    delayMicroseconds(usDelay); 

    digitalWrite(STEP_PIN, LOW); 
    delayMicroseconds(usDelay); 
  } 
} 
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: RESET TIMER
void resetTimer(){
  previousMillis = currentMillis;  
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: WAKE UP
void wakeUp(){
  //digitalWrite(SLEEP_PIN, HIGH);
  digitalWrite(ENABLE_PIN, LOW);                  // wake up motor
  sleep = false;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: GO TO SLEEP
void goToSleep(){
  digitalWrite(ENABLE_PIN, HIGH);                 //disable pins and shut down motor
  sleep = true;
}














