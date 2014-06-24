// MinHC05M.ino v1.0 June 2014
// Bluetooth Master control library.
// Copyright (C) 2014 David Farrell @ Protoparts.
// See license.txt for terms.
// Tested on the Arduino Micro.
// Your HC05 module MUST have STATE and KEY lines. The modules
// with EN will not work properly.

#include <hc05m.h>

// general required IO (in addition to Serialx)
const uint8_t HC05_KEY = 6;
const uint8_t HC05_STATE = 7;
HC05M hc05(HC05_KEY, HC05_STATE);

// All HC05 messages are parsed and trigger this (optional) callback.
void fcallback(enum BUF_ARG arg) {
  if(arg == BA_ERROR) {
    Serial.print("ERROR:");
    Serial.println((char *)hc05.bf);
  }  
  if(arg == BA_OK) {
    Serial.println("OK");
  }
  if(arg == BA_FAIL) {
    Serial.println("FAIL");
  }
  if(arg == BA_DISC) {
    Serial.print("DISC:");
    Serial.println((char *)hc05.bf);
  }
}

void setup() {
  hc05.callback = fcallback;    // optional for some uses, set before begin()!
  hc05.begin(&Serial1, 38400);  // serial port HC05, best baud.
  Serial.begin(9600);           // UI terminal
}

static uint8_t do_dev = 0;
        
void loop() {
char c;
bool rv;
 
  if(Serial.available()) {
    c = Serial.read();
    switch(c) {
      case '0':  // switch to command mode (connected or not)
        hc05.setmode(HC05M::MODE_CMMD);
      break;
      case '1':  // switch to data mode (assumes connection)
        hc05.setmode(HC05M::MODE_DATA);   
      break;
      case 'A':
        hc05.wrt_cmmd("AT");
      break;      
      case 'C':           
        // link is connect, switches to data mode if successful
        hc05.link(0x98D3,0x31,0x22006F9);
      break;            
      case 'D':  // Disconnect
        hc05.wrt_cmmd("AT+DISC");       
      break;
      case 'T':
        hc05.println("test message");
      break;
      default:
        Serial.println("0,1,A,C,D,T are valid.");
    } // switch   
  } // if Serial.available()
  
  hc05.process();  // process anything rx from HC05
  
  // Data interface (for connected slave) uses the stream class.
  if(hc05.available()) {
    char c;
    c = hc05.read();
    Serial.print(c);    
  }  
}




