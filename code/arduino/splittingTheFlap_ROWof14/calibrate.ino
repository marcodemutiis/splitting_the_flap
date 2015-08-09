void calibrate(int i){
  calibrating = true;
  allSteppers[i].moveTo(32);//32
  allSteppers[i].setSpeed(myCalibSpeed);
  
  calibCurrentMillis[i] = millis();
  calibPreviousMillis[i] = calibCurrentMillis[i];
  
  while(calibrating){
    //SMOOTH THE READING OF THE HALL EFFECT/////////  

calibCurrentMillis[i] = millis();

    total= total - readings[index]; // subtract the last reading        
    readings[index] = analogRead(hallPins[i]); // read from the sensor  //HALLVAL???-----------------
    total= total + readings[index];  // add the reading to the total    
    index = index + 1;     // advance to the next position in the array                

    if (index >= numReadings) {         // if we're at the end of the array...      
      index = 0;              // ...wrap around to the beginning               
      average = total / numReadings;  // calculate the average   
//        Serial.print("motor # ");
//       Serial.print(i);
//       Serial.print(" hall sensor reading = ");
//       Serial.println(average);   // send it to the computer as ASCII digits  


      resetTimer(i);  //resetTimer
      if(sleep[i]) {
        wakeUp(i);
      } //wakeUp

      if(average > hallThreshold || average <= hallValPrev[i]){
        resetTimer(i);
        if(allSteppers[i].distanceToGo() == 0){
          allSteppers[i].setCurrentPosition(0); 
          allSteppers[i].moveTo(32);//32
          allSteppers[i].setSpeed(myCalibSpeed);
        }
      }

      else if (average > hallValPrev[i]){ 

        allSteppers[i].setCurrentPosition(0);
        lastPos[i] = 39;
        calib[i] = false; 
        calibrating = false;

        if(calibLast == i) {
         // allMotorsStopped = true;
         //powerOFF();
        }

        resetTimer(i); //reset the timer
        goToSleep(i);  //turn off the motor
      }
      
       if( calibCurrentMillis[i] - calibPreviousMillis[i] > calibInterval){ //if it has found the magnet or if it took more than 30 secs and it couldnt find magnet (= mechanical problem)
       calibPreviousMillis[i] = calibCurrentMillis[i]; 
       
        allSteppers[i].setCurrentPosition(0);
        lastPos[i] = 39;
        calib[i] = false; 
        calibrating = false;
        resetTimer(i); //reset the timer
        goToSleep(i);  //turn off the motor
      }
     
      
      
      hallValPrev[i] = average;
    }
    allSteppers[i].runSpeedToPosition();
 
  }
}



