//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: RESET TIMER
void resetTimer(int i){
   previousMillis[i] = currentMillis;  
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: WAKE UP
void wakeUp(int i){

   digitalWrite(enblPin[i], LOW);                  // wake up motor
   sleep[i] = false;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: GO TO SLEEP
void goToSleep(int i){
 
   digitalWrite(enblPin[i], HIGH);                 //disable pins and shut down motor
   sleep[i] = true;
}



