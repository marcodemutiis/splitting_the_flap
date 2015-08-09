void powerON(){
   digitalWrite(relayPin, HIGH); //turn the power on
   delay(200);
}

void powerOFF(){
  delay(200);
   digitalWrite(relayPin, LOW); //turn the power on
}
