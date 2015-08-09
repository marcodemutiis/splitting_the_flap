//////////////////////////////////////////////////////
//stepper connection to Easy Driver 
//red to A left
//blue to A right
//black to B left
//green to B right
//////////////////////////////////////////////////////

#include <AccelStepper.h>

//relay/////////::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::relay
int relayPin = 53;
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//I2C communication setup::::::::::::::::::::::::::::::::::::::::::::::::::::::I2C
int deviceID = 1; // 1 = master
int targetDevice = 2; //2-7
int targetCalibDevice = 2; //2-7
int charCounter = 1;
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//accel stepper and easy driver - motor initialization::::::::::::::::::::::::motor
const int dirPin[] = { 
  9, 12, 4, 14, 38, 44, 50, 29, 23, 26, 32, 35, 41, 47}; //direction pins
const int stepPin[] = { 
  8, 11, 5, 2, 36, 42, 48, 31, 25, 24, 30, 37, 43, 49};//step pins
//init steppers
AccelStepper stepper1(1, stepPin[0], dirPin[0]);
AccelStepper stepper2(1, stepPin[1], dirPin[1]);
AccelStepper stepper3(1, stepPin[2], dirPin[2]);
AccelStepper stepper4(1, stepPin[3], dirPin[3]);
AccelStepper stepper5(1, stepPin[4], dirPin[4]);
AccelStepper stepper6(1, stepPin[5], dirPin[5]);
AccelStepper stepper7(1, stepPin[6], dirPin[6]);
AccelStepper stepper8(1, stepPin[7], dirPin[7]);
AccelStepper stepper9(1, stepPin[8], dirPin[8]);
AccelStepper stepper10(1, stepPin[9], dirPin[9]);
AccelStepper stepper11(1, stepPin[10], dirPin[10]);
AccelStepper stepper12(1, stepPin[11], dirPin[11]);
AccelStepper stepper13(1, stepPin[12], dirPin[12]);
AccelStepper stepper14(1, stepPin[13], dirPin[13]);

AccelStepper allSteppers[] = {
  stepper1, stepper2, stepper3, stepper4, stepper5, stepper6, stepper7, stepper8, stepper9, stepper10, stepper11, stepper12, stepper13, stepper14};

const int totalSteppers = 14;//(sizeof(allSteppers)/sizeof(AccelStepper));
int stepperIndex = 0;
int enblPin[totalSteppers] = {
  7, 10, 6, 3, 34, 40, 46, 33, 27, 22, 28, 39, 45, 51};//enable pins
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::status
boolean calibrating = false;         // bool for calibartion of each motor
boolean writing = false;             // bool for checking if the whole display is done writing and can accept new string
boolean calibrationRequest = false;  // check if arduino local calibration is do
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//positioning and flap speed:::::::::::::::::::::::::::::::::::::::::::::::::::pos&speed
int tot = 40;
int howManySteps[totalSteppers];
int lastPos[totalSteppers];
int mySpeed = 3000; //safe < 10000
int myCalibSpeed = 5000;
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//incoming characters, string handling and splitting:::::::::::::::::::::::::::inBytes
char letters[40] = { 
  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '#', '.', '/', ' '   
}; //allowed chars
char inByte;
//char incomingSentence[totalSteppers];
boolean allMotorsStopped = true;
int motorsStopped = 0;

//string handling
int sentenceLength = totalSteppers;       // 14 chars per row
char incomingSentence[totalSteppers];     // a string to hold incoming data
//int stepperIndex;                       // an int to add chars inside the sentence and check amount of chars once the sentence is complete (sentence is complete when it receives '/n' as last char)
boolean startSentence = false;            // whether we can start storing the chars of our sentence (chars start being stored only if first char is a '*')
boolean stringComplete = false;           // whether the string is complete
String inputString = "";                  // a string to hold incoming data

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//calibration setup::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::calib
const int hallPins[] = { 
  A6, A5, A4, A3, A0, A1, A2, A7, A8, A9, A10, A11, A12, A13};//hall effect pins for calibration
boolean calib[totalSteppers];   //boolean array to store the info on which more motor needs calibration
int calibLast;                  //variable to store the number of the motor that gets calibrated last so we know when calibration is finished
int hallVal[totalSteppers];     //hall effect sensor variables
int hallValPrev[totalSteppers];
int hallThreshold = 500;
//smoothing
const int numReadings = 10;     // how many readings 10
int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//timer variables to turn off motors:::::::::::::::::::::::::::::::::::::::::::timer
unsigned long currentMillis;
boolean sleep[totalSteppers];              // a bool to check if the motor is off or not
long previousMillis[totalSteppers];        // will store last time timer was set
long interval = 100;                       // interval after which we turn off motor
//calibTimer
unsigned long calibCurrentMillis[totalSteppers];
long calibPreviousMillis[totalSteppers];   // will store last time timer was set
long calibInterval = 5000;                 // interval after which we skip calib

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

////////////////////////////////////////////////////////////////////////////////////////////setup//setuo//setup//setup//setup
void setup(){
  Serial.begin(9600);                       // init Serial

  for(int i = 0; i < totalSteppers; i++){

    pinMode(enblPin[i], OUTPUT);           //init enable pin as outputs
    digitalWrite(enblPin[i], HIGH);        // set enable pin high to turn off motors
    //init motors
    allSteppers[i].setMaxSpeed(mySpeed);
    allSteppers[i].setCurrentPosition(0);
    allSteppers[i].setSpeed(mySpeed);
    lastPos[i] = 39;

    sleep[i] = true;
    previousMillis[i] = 0;

    //calib timer variables
    calibCurrentMillis[i] = 0;
    calibPreviousMillis[i]=0;

    calib[i] = false;
    hallVal[i] = 0;
  }  
  //smoothing
  for (int thisReading = 0; thisReading < numReadings; thisReading++){ //reset average calibration readings
    readings[thisReading] = 0;   
  }
  //string
  inputString.reserve(sentenceLength);                // reserve 98 bytes for the inputString:

  ///relay
  pinMode(relayPin, OUTPUT); //init enable pin as outputs
  powerOFF();

}
/////////////////////////////////////////////////////////////////////////////////////////////loop//loop/loop//loop//loop//loop
void loop(){
  currentMillis = millis(); //update millis counter for timer function  

  //calib start//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://calib start// 
  if(inByte == '='){   //if the incoming char is not an available letter and it is a '=' trigger the calibration function

    Serial.println("calibration begins"); 
    allMotorsStopped = false;  
    powerON();//turn on the relay
    calibrationRequest = true;

    for(int needCalib = 0; needCalib < totalSteppers; needCalib++){    //run through all motors
     
     /* Serial.print("motor #");
      Serial.print(needCalib);
      Serial.print(" hall effect reading = ");
      Serial.println(analogRead(hallPins[needCalib]));*/

      if(analogRead(hallPins[needCalib]) > hallThreshold){             // if the hall effect sensor detects that it is far from magnet, means we are not at zero position

        hallValPrev[needCalib] = analogRead(hallPins[needCalib]) + 1;  
        calib[needCalib] = true;                                       //turn on the needCalib boolean for the motors which need to be calibrated
        calibrating = true;
        calibLast = needCalib;

      }
      else{
        calib[needCalib] = false;                                      //if the hall effect senses that is close to the magnet we are in 0 position, so we turn off the needCalib boolean for the motors (does not need to be calibrated)
      } 
      stepperIndex=0;                                                  //reset stepperIndex
      
    }
  }
  //calib ends//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://calib ends// 


  //string complete start///::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://string complete starts// 
  if (stringComplete && stepperIndex >= sentenceLength) {
Serial.println("writing begins");
    writing = true;
    powerON();//turn on the relay

    allMotorsStopped = false;                        //we switch this boolean in any case just say we got a new string coming in

    for(int i = 0; i < totalSteppers; i++){          //send each char to each stepper 
      for(int newPos = 0; newPos < tot; newPos++){   //run through all the available characters DUPLICATE filter???---------------------
        if( incomingSentence[i] == letters[newPos]){ //and if the incoming char is one of the available characters---------------------
          if(lastPos[i] != newPos){                  //if new letter is different from current letter
            resetTimer(i);                           //reset timer
            if(sleep[i]) {
              wakeUp(i);                             //turn on motor if it was off
            }  
            
            //calculate how many steps to new position////////////
            if (lastPos[i] < newPos) howManySteps[i] = newPos - lastPos[i]; 
            else howManySteps[i] = tot - (lastPos[i] - newPos);
            /////////////////////////////////////////////////////
            allSteppers[i].moveTo(160*howManySteps[i]);  //160. = 1/10 of the motor gear = 1/40 of the flap gear
            allSteppers[i].setSpeed(mySpeed);
            /////////////////////////////////////////////////////
            lastPos[i] = newPos;
          }
        }              
      }
    }
    stepperIndex = 0; //reset stepperIndex
  }
  //string complete ends//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://string complete ends// 


  for(int i = 0; i < totalSteppers; i++){

    if(calib[i] == true) {
      calibrate(i);
    }
  }

  //////////////////////////////////////

  for(int i =0; i < totalSteppers; i++){

    if(allSteppers[i].distanceToGo() == 0){            //if this motor has reached destination

      if(!sleep[i]){
        if(currentMillis - previousMillis[i] > interval){ //if the motor hasnt moved in more than 100 ms
          resetTimer(i); //reset the timer
          goToSleep(i);  //turn off the motor
        }
      }

      allSteppers[i].setCurrentPosition(0);            //set the position to 0

        if(allMotorsStopped == false){
        motorsStopped++;
      }                                 //add the motor to the count of motors that have reached destionation and have been turned off
    }

    else{                                              //if this motor has NOT reached destination
      resetTimer(i);                                   //reset the timer if motor is still running (so it doesn't trigger the timer that turns it off)
      motorsStopped = 0;                                //reset the count of motors that have stopped
      allMotorsStopped = false; 

      allSteppers[i].runSpeedToPosition();             //tell the motor to RUN to position (unless it has reached pos 0)
    }

    if(motorsStopped == totalSteppers){
      allMotorsStopped = true;
      charCounter=0;
      Serial.println("all motors have stopped");

      motorsStopped = totalSteppers+1;
      powerOFF();
      writing = false;
    }
  }
}












