#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
  
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
	ofBackground(195, 195, 255);
    //font
    font.loadFont("font/Novecentoslabwide-Normal.otf", 20);
    smallFont.loadFont("font/Novecentoslabwide-Normal.otf", 12);
    targetArduino = 0;
    maxDevices  = ROWS;
    
    arduinoPath.push_back("/dev/tty.usbmodem14141");//24141 henrys mac //14141
    arduinoPath.push_back("/dev/tty.usbmodem14131");//24131 henrys mac //14131
    arduinoPath.push_back("/dev/tty.usbmodem14121");//24121 henrys mac //14121
    arduinoPath.push_back("/dev/tty.usbmodem141141");//241141 henrys mac //141141
    arduinoPath.push_back("/dev/tty.usbmodem141131");//241131 henrys mac //141131
    arduinoPath.push_back("/dev/tty.usbmodem141121");//241121 henrys mac //141121
    arduinoPath.push_back("/dev/tty.usbmodem141111");//241111 henrys mac //141111
    
    for (int i = 0; i < maxDevices; i++) {
        //setup serial comm with each arduino
        serial[i].setup(arduinoPath[i], 9600);
        serial[i].startContinuousRead(false);
        ofAddListener(serial[i].NEW_MESSAGE,this,&testApp::onNewMessage);
        
        //init substrings for each arduino
        splits.push_back("");
        displaySplits.push_back("");
        
    }

    //string management
    message = "";
    logoMessage = "";
    for(int i =0; i<COLUMNS; i++){
        logoMessage+='/';
    }
    extraCalibMessage = "";
    for(int i = 0; i<COLUMNS; i++){
        extraCalibMessage+='x';
    }
    emptyColumn = "";
    for(int i =0; i<COLUMNS; i++){
        emptyColumn+= ' ';
    }
    displayMessage = "";
    for(int i =0; i<MASTERSTRINGSIZE; i++){
        displayMessage+='_';
    }
    
    index           =   0;
    step = 0; //step 0 = standy, step 1 = go to logo, step 2 = calibrate, step 3 = write message
    maxCharAmount   =    MASTERSTRINGSIZE ; //read from XML + 1 for initial * char needed
    canWrite = true;
    remember = true;
    calibration = false;
    extraCalibration = false;
    
    //OSC
    OSC_on = true;
    // listen on the given port
	cout << "listening for osc messages on port " << PORT << "\n";
	receiver.setup(PORT);
    displayNewMessageOSC = "";
    
    terminationSequence = false;
    
}

//--------------------------------------------------------------
void testApp::update(){

    ////////timer////////////////////////////////////////////////////////////////////
     s = ofGetSeconds();
     m = ofGetMinutes();
     h = ofGetHours();
    
    //turn off OSC reading at 23:59:59 the app turns into a pumpkin at midnight
    if(h == 23 && m == 59 && s == 59){
        if(OSC_on) {
            OSC_on = false;
            terminationSequence = true;
        }
    
    }
    //calibrate and reset display at 00:20
    if(h == 24 && m == 20 && s == 0){
        if(terminationSequence){
            if(canWrite){
            extraCalibration = true;
            terminationSequence = false;
            cout << "received extra calibration message" << endl;
            message = " ";
            if(step == 0){//if we were on standy mode we can start
                step = 1;
                remember = false;
                sendSplits();
            }
            displayNewMessageOSC = "";
            }
        }
    }
    //quit app at 00:40
    if(h == 24 && m == 40 && s == 0){
        exit();
        OF_EXIT_APP(0);
    }
    //comp auto turn off at 1 am
    //turn on at 7
    //login item >> start app
    ////////timer////////////////////////////////////////////////////////////////////
    
    
    //OSC
    if(OSC_on){
        // check for waiting messages
        while(receiver.hasWaitingMessages()){
            // get the next message
            ofxOscMessage m;
            receiver.getNextMessage(&m);
            
            
            // check for new messages for display
            if(m.getAddress() == "/display/newMessage"){
                // the single argument is a string
                displayNewMessageOSC = m.getArgAsString(0);
            }
            else{
                // unrecognized message: display on the bottom of the screen
                string msg_string;
                msg_string = m.getAddress();
                msg_string += ": ";
                for(int i = 0; i < m.getNumArgs(); i++){
                    // get the argument type
                    msg_string += m.getArgTypeName(i);
                    msg_string += ":";
                    // display the argument - make sure we get the right type
                    if(m.getArgType(i) == OFXOSC_TYPE_INT32){
                        msg_string += ofToString(m.getArgAsInt32(i));
                    }
                    else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
                        msg_string += ofToString(m.getArgAsFloat(i));
                    }
                    else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
                        msg_string += m.getArgAsString(i);
                    }
                    else{
                        msg_string += "unknown";
                    }
                }
                // add to the list of strings to display
                msg_strings[current_msg_string] = msg_string;
                timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
                current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;
                // clear the next line
                msg_strings[current_msg_string] = "";
            }
            
        }
        //if the display is writing reset the message
        if (!canWrite){
            
            displayNewMessageOSC = "";
            
        }
        //if the display is stopped and the message is not a calibration message
        else if(canWrite && displayNewMessageOSC != "" && displayNewMessageOSC != "="){
            
            
            //check every letter
            //--
            
            if(displayNewMessageOSC.length() > MASTERSTRINGSIZE){
                
                displayNewMessageOSC = displayNewMessageOSC.substr(0, MASTERSTRINGSIZE);
                cout << "message too long, resized to:" << displayNewMessageOSC << endl;
            }
            
            index = displayNewMessageOSC.length();
            message = displayNewMessageOSC;
            
            //we always send a full array to arduino so we add spaces if the array is less than 24 chars
            if ( index > 0 && index <= maxCharAmount ) {
                cout    <<"index = "<< index<<endl;
                for(int i = index; i < maxCharAmount;i++){
                    message+=' ';
                }
                for(int i = 0; i < maxCharAmount;i++){
                    cout << message[i] << endl;
                }
                //split the message into strings for each arduino
                for(int i = 0; i < maxDevices; i++){
                    splits[i] = message.substr(i*MASTERSTRINGSIZE/ROWS, MASTERSTRINGSIZE/ROWS);
                    cout << "spit string #" << i << " = " << splits[i] << endl;
                }
                if(step == 0){//if we were on standy mode we can start
                    step = 1;
                    remember = false;
                    sendSplits();
                }
                displayNewMessageOSC = "";
                
            }
        }
        //if the display is stopped and the message is a calibration message
        else if(canWrite && displayNewMessageOSC == "="){
            extraCalibration = true;
            cout << "received extra calibration message" << endl;
            //            message = "";
            //            for(int i = 0; i < maxDevices; i++){
            //                splits[i] = "";
            //            }
            //
            //            remember = false;
            message = " ";
            if(step == 0){//if we were on standy mode we can start
                step = 1;
                remember = false;
                sendSplits();
            }
            displayNewMessageOSC = "";
            
            
            
        }
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    ofColor darkGrey = (50,50,70);
    ofColor panna = (220,200,200);
    
    //display title
    ofSetColor(darkGrey);
    font.drawString("blueprint split-flap display", ofGetWidth()/2-(font.stringWidth("blueprint split-flap display"))/2, 100);
    
    for(int i =0; i < ROWS; i++){
        ofSetColor(darkGrey);
        ofRect(50, (i*45)+ 165, ofGetWidth()-600, 30);
        //display string
        ofSetColor(panna);
        font.drawString(displaySplits[i], 90, (i*45)+187);
        
    }
    //ofSetColor(panna);
    //font.drawString(message, 50, 180);
    
    
    //osc message
    string buf;
	buf = "listening for osc messages on port " + ofToString(PORT);
	ofDrawBitmapString(buf, 50, 20);
    
	// draw New Message OSC
    ofDrawBitmapString("new OSC message = " +displayNewMessageOSC, 580, 20);
    
	for(int i = 0; i < NUM_MSG_STRINGS; i++){
        
		ofDrawBitmapString(msg_strings[i], 10, 40 + 15 * i);
	}
    
    
    //line marker/////////////////////////////////////
    ofSetColor(222);
    ofCircle(30, 180 + (45*(index/COLUMNS)), 10);
    /////////////////////////////////////////////////
    //display remining characters
    ofSetColor(darkGrey);
    smallFont.drawString("remaining characters = " + ofToString(maxCharAmount-index), 50, 620);
    
    //commands
    ofSetColor(darkGrey);
    smallFont.drawString("press 'enter' to send the string\npress '=' to calibrate the flaps\npress 'tab' to move cursor to next line\npress '-' for '#'", 50, 670);
    
    if(canWrite) ofSetColor(120, 120, 255);
    else ofSetColor(255, 100, 100);
    ofCircle(ofGetWidth()-200, 93, 10);
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(canWrite){
        
        
        //TAB move to next line////////////////////////
        if(( char )key == OF_KEY_TAB){
            if(index<MASTERSTRINGSIZE){
                int currentRow = (index/COLUMNS) + 1;
                int j = (COLUMNS * currentRow) - index;
                
                for(int i = index; i < index+j; i++){
                    message+=' ';
                    displayMessage[i]= ' ';
                }
                
                index = j+index;
                cout<<"index = " << index << endl;
            }
        }
        ///////////////////////////////////////////////
        
        // - is #
        
        if(( char )key=='-'){
            key = '#';
        }
        
        
        
        key = tolower((char)key);
        
        
        char letters[40] ={
            ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0','a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '.', '#', '/'
        };
        
        
        //insert letter in our message string
        for(int i=0; i<40;i++){
            if(( char )key==letters[i]){
                
                
                //if there is space in our char array add the new char to it
                if (index < maxCharAmount) {
                    
                    message += letters[i];
                    displayMessage[index] = letters[i];
                    
                    cout    << "index = "<< index<<endl;
                    cout    << "char = "<< message <<endl;
                    
                    //                message+=letters[i];
                    index++;
                    break;
                }
                else {//if (index >= maxCharAmount) {
                    
                    cout    <<  "char array full"   <<  endl;
                    cout    << "index = " << index << endl;
                    
                }
            }
        }
        
        if(key==127){ //delete
            if(index>0){
                cout    << "DELETE"<<endl;
                
                message = message.substr(0, message.length()-1);
                
                displayMessage[index-1] = '_';
                
                index--;
                
                for(int i=0; i<message.length();i++){
                    
                    cout<<message[i]<<endl;
                }
                
                cout    << "index = "<< index<<endl;
                cout    << "message = " << message << endl;
            }
        }
        
        
        for(int i = 0; i < maxDevices; i++){
            displaySplits[i] = displayMessage.substr(i*MASTERSTRINGSIZE/ROWS, MASTERSTRINGSIZE/ROWS);
        }
        
        //press enter to send char array to arduino (if the display has finished writing
        if(key==OF_KEY_RETURN){
            cout    << "RETURN"<<endl;
            //we always send a full array to arduino so we add spaces if the array is less than 24 chars
            if ( index > 0 && index <= maxCharAmount ) {
                cout    <<"index = "<< index<<endl;
                //add spaces if message is not full
                for(int i = index; i < maxCharAmount;i++){
                    message+=' ';
                }
                for(int i = 0; i < maxCharAmount;i++){
                    cout << message[i] << endl;
                }
                
                //split the message into strings for each arduino
                for(int i = 0; i < maxDevices; i++){
                    //Serial.print(inputString);
                    splits[i] = message.substr(i*MASTERSTRINGSIZE/ROWS, MASTERSTRINGSIZE/ROWS);
                    cout << "spit string #" << i << " = " << splits[i] << endl;
                }
                
                //start the fireworks
                if(step == 0){//if we were on standy mode we can start
                    step = 1;
                    remember = false;
                    sendSplits();
                }
                
            }
        }
    }
    
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    if(canWrite){
        if(( char )key=='='){
            extraCalibration = true;
            cout << "received extra calibration message" << endl;
            //        message = "";
            //        for(int i = 0; i < maxDevices; i++){
            //            splits[i] = "";
            //        }
            //
            //        remember = false;
            message = " ";
            if(step == 0){//if we were on standy mode we can start
                step = 1;
                remember = false;
                sendSplits();
            }
        }
        //////////////////////////////////////////////////////////
        else if(key == ']'){
            cout << "received logo message" << endl;
            message = "";
            for(int i =0; i<maxCharAmount; i++){
                message+='/';
            }
               cout << message <<  endl;
            
            //split the message into strings for each arduino
            for(int i = 0; i < maxDevices; i++){
                //Serial.print(inputString);
                splits[i] = message.substr(i*MASTERSTRINGSIZE/ROWS, MASTERSTRINGSIZE/ROWS);
                cout << "spit string #" << i << " = " << splits[i] << endl;
            }
            
         
            if(step == 0){//if we were on standy mode we can start
                step = 1;
                remember = false;
                sendSplits();
            }
        }
        /////////////////////////////////////////////////////////
    }
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}
//--------------------------------------------------------------
void testApp::onNewMessage(string & incomingMessage){
	cout << "onNewMessage, message: " << incomingMessage << "\n";
    if(incomingMessage == "all motors have stopped"){
        
        targetArduino++;
        ofSleepMillis(200);
        
        
        if (targetArduino < maxDevices){ //send to next arduino
            remember = false;
            if(calibration == false)     {   sendSplits();}
            else sendSplitCalibs();
        }
        
        if (targetArduino >= maxDevices) { //if you have reached the last arduino
            targetArduino = 0;
            if(step<3){step++;}
            else step = 0;
            
            //step 2: finished logo, go to calib
            if(step == 2){
                
                calibration = true;
                remember = false;
                sendSplitCalibs();
            }
            
            //step 3: finished calib, turn off calib, go to write
            if(step == 3){
                calibration = false;
                remember = false;
                sendSplits();
                
            }
            
            //step 0: finished writing, reset string, string index and splits, turn on canWrite bool > ready to accept new messages
            if(step == 0){
                
                if(extraCalibration){
                    extraCalibration = false;
                }
                if(calibration == false){
                    //reset message and splits
                    message = "";
                    for(int i = 0; i < maxDevices; i++){
                        //Serial.print(inputString);
                        splits[i] = "";
                    }
                    
                }
                else {calibration = false;}
                
                canWrite = true;
                index = 0;
                //ofBackground(0, 200, 0);
                
                //clear display
                for(int i = 0; i < MASTERSTRINGSIZE; i++){
                    displayMessage[i] = '_';
                }
                for(int i = 0; i < maxDevices; i++){
                    displaySplits[i] = displayMessage.substr(i*MASTERSTRINGSIZE/ROWS, MASTERSTRINGSIZE/ROWS);
                }
            }
        }
        
    }
    
    else  {
        //ofBackground(200, 0, 0);
        canWrite = false;
    }
	
}
//--------------------------------------------------------------
void testApp::sendSplits(){
    if(message != "" && remember == false && calibration == false)
    {
        //step 1 LOGO
        if(step==1){
            
            if(!extraCalibration){
                cout << "sending to arduino path" << arduinoPath[targetArduino]<< ", logo message: " << splits[targetArduino] << "\n";
                serial[targetArduino].writeString("<");
                serial[targetArduino].writeString(logoMessage);
                serial[targetArduino].writeString(">");
                //        message = "";
                //        index = 0;
                remember = true;
            }
            else if(extraCalibration == true){
                cout << "sending to arduino path" << arduinoPath[targetArduino]<< ", extra Calib message: " << splits[targetArduino] << "\n";
                serial[targetArduino].writeString("<");
                serial[targetArduino].writeString(extraCalibMessage);
                serial[targetArduino].writeString(">");
                //        message = "";
                //        index = 0;
                remember = true;
                
            }
        }
        
        //step2 CALIB
        if(step==2){
            
            
        }
        //step3 WRITE
        if(step==3){
            if(!extraCalibration){
                cout << "sending to arduino path" << arduinoPath[targetArduino]<< ", message: " << splits[targetArduino] << "\n";
                serial[targetArduino].writeString("<");
                serial[targetArduino].writeString(splits[targetArduino]);
                serial[targetArduino].writeString(">");
                //        message = "";
                //        index = 0;
                remember = true;
            }
            else{
                cout << "sending to arduino path" << arduinoPath[targetArduino]<< ", message: " << splits[targetArduino] << "\n";
                serial[targetArduino].writeString("<");
                serial[targetArduino].writeString(emptyColumn);
                serial[targetArduino].writeString(">");
                //        message = "";
                //        index = 0;
                remember = true;
                
            }
        }
    }
}
//--------------------------------------------------------------
void testApp::sendSplitCalibs(){
    if(remember == false && calibration == true)
    {
        
		cout << "sending to arduino path" << arduinoPath[targetArduino]<< ", message: " << "=" << "\n";
        
        serial[targetArduino].writeString("=");
        serial[targetArduino].writeString(" ");
        
        remember = true;
    }
}
