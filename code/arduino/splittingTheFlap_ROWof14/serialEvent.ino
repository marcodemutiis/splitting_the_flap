void serialEvent() {

  if(!writing && !calibrating){
    while (Serial.available()) {
      // get the new byte:


      inByte = Serial.read(); //store incoming char
      if(inByte == '<' && stepperIndex == 0) {
        startSentence = true;
        inputString = "";
      }

      if(startSentence){

        if(inByte == '=') inByte = '.';

        for(int newPos = 0; newPos < 40; newPos++) {  //run through all the available characters 
          if(letters[newPos] == inByte) {              //and if the incoming char is one of the available characters
            if(stepperIndex< sentenceLength){
              incomingSentence[stepperIndex] = inByte;   //add the char to our string
            }
            stepperIndex++;
          }
        }

        // if the incoming character is a newline and we have reach the sentence target length, set a flag
        // so the main loop can do something about it:
        if (inByte == '>' && stepperIndex >= sentenceLength) {
          stringComplete = true;
          startSentence = false;
        } 
        // if the incoming character is a newline but the length is less then we expected we clear the sentence and we go back waiting for incoming data 
        //(this is to avoid chars accumulating from accidentally shorter strings and creating problems):
        else if(inByte == '>' && stepperIndex < sentenceLength){
          stepperIndex = 0;
          startSentence = false;
        }
      }
    }
  }
}






