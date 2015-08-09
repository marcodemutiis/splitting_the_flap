#pragma once

#include "ofMain.h"
#include "ofxSimpleSerial.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

#define ROWS 7	//3 //will be 7 horizontal rows
#define COLUMNS 14 //will be 14 units per row
#define MASTERSTRINGSIZE COLUMNS*ROWS //will be 98 total chars

// listen on port 12345
#define PORT 12345
#define NUM_MSG_STRINGS 1

class testApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void sendSplits();
    void sendSplitCalibs();
    
    
    //font
    ofTrueTypeFont  font, smallFont;
    
    //ofxSimpleSerial
    ofxSimpleSerial	serial[7];
    vector<string> arduinoPath;
    int targetArduino, maxDevices;
    
    string		message;
    string logoMessage;
    string extraCalibMessage;
    string emptyColumn;
    vector<string> splits;
    
    string displayMessage;
    vector<string> displaySplits;
    bool		remember;
    void		onNewMessage(string & incomingMessage);
    
    //string management
    int index;
    int maxCharAmount;
    
    bool canWrite;
    bool calibration;
    bool extraCalibration;
    bool terminationSequence;
    
    int step;
    
    
    //OSC
    ofxOscReceiver receiver;
    bool OSC_on;
    string displayNewMessageOSC;
    int current_msg_string;
    string msg_strings[NUM_MSG_STRINGS];
    float timers[NUM_MSG_STRINGS];
    
    int s, m, h;
};
