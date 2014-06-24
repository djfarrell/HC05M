// FullHC05M.ino v1.0 June 2014
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

static uint8_t requested_name = 0xff;

// All HC05 messages are parsed and trigger this (optional) callback.
void fcallback(enum BUF_ARG arg) {

  if(arg == BA_ERROR) {
    Serial.print("ERROR:");
    Serial.println((char *)hc05.bf);
  }
  
  // Inquiry to find bluetooth devices.
  if(arg == BA_INQ) {
    if(hc05.parse_inq())Serial.println("Dev Found");
  }
  
  if(arg == BA_OK) {
    Serial.println("OK");
  }
  if(arg == BA_FAIL) {
    Serial.println("FAIL");
  }
  if(arg == BA_STATE) {
    Serial.print("STATE:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_VERSION) {
    Serial.print("VERSION:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_UART) {
    Serial.print("UART:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_ADDR) {
    Serial.print("ADDR:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_ROLE) {
    Serial.print("ROLE:");
    Serial.println((char *)hc05.bf);
  } 
  if(arg == BA_PSWD) {
    Serial.print("PSWD:");
    Serial.println((char *)hc05.bf);
  } 
  if(arg == BA_CMODE) {
    Serial.print("CMODE:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_NAME) {
    Serial.print("NAME:");
    Serial.println((char *)hc05.bf);
  }
  
  // Reqponse to a previously requested device name, store the response
  // using the index of the requested device.
  if(arg == BA_RNAME) {
    Serial.print("RNAME:");
    Serial.println((char *)hc05.bf);
    
    if(requested_name != 0xff) {
       if(hc05.be[requested_name].nam == NULL) {
         hc05.be[requested_name].nam = strdup((char *)hc05.bf);
       }
       // has a name but was changed, same address.
       else if(strcmp((char *)hc05.bf, hc05.be[requested_name].nam) != 0) {
         free(hc05.be[requested_name].nam);
         hc05.be[requested_name].nam = strdup((char *)hc05.bf);
       }       
       requested_name = 0xff;
    }    
  }
  
  // Whatever class the device responds with is the class I will use from
  // here on.  Be sure to program class prior to this for expected results.
  if(arg == BA_CLASS) {
    Serial.print("CLASS:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_IAC) {
    Serial.print("IAC:");
    Serial.println((char *)hc05.bf);
  } 
  if(arg == BA_INQM) {
    Serial.print("INQM:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_BIND) {
    Serial.print("BIND:");
    Serial.println((char *)hc05.bf);
  } 
  if(arg == BA_POLAR) {
    Serial.print("POLAR:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_MPIO) {
    Serial.print("MPIO:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_IPSCAN) {
    Serial.print("IPSCAN:");
    Serial.println((char *)hc05.bf);
  } 
  if(arg == BA_SENM) {
    Serial.print("SENM:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_ADCN) {
    Serial.print("ADCN:");
    Serial.println((char *)hc05.bf);
  }  
  if(arg == BA_MRAD) {
    Serial.print("MRAD:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_DISC) {
    Serial.print("DISC:");
    Serial.println((char *)hc05.bf);
  }
  if(arg == BA_SNIFF) {
    Serial.print("SNIFF:");
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
      case '2':       
// TODO:sniff is tedious to debug, will have to wait for next rev.       
//      hc05.wrt_cmmd_addr("AT+ENSNIFF?", hc05.be[do_dev].nap, hc05.be[do_dev].uap, hc05.be[do_dev].lap);                     
//      hc05.wrt_cmmd("AT+SNIFF?");
      break;
      case '3':      
       // Manually phone addresses.  If one queries to phones, they will repond with their names.
       // By periodically querying the devices, you can tell if they are in range (or turned on)
       hc05.store_bt_slave( (uint16_t)0x3cd0, (uint8_t)0xf8, (uint32_t)0xb7cbe3, 0, 0x7A020C); // Dav
       hc05.store_bt_slave( (uint16_t)0x3cd0, (uint8_t)0xf8, (uint32_t)0xd34aea, 0, 0x7A020C); // Den
       hc05.store_bt_slave( (uint16_t)0x60c5, (uint8_t)0x47, (uint32_t)0x4a685e, 0, 0x7A020C); // Mik
       hc05.store_bt_slave( (uint16_t)0xa816, (uint8_t)0xb2, (uint32_t)0x04bf94, 0, 0x7A020C); // Mag
       hc05.store_bt_slave( (uint16_t)0xa007, (uint8_t)0x98, (uint32_t)0x155ecd, 0, 0x5A02);   // samsung       
      break;

      case '4':
        hc05.wrt_cmmd("AT+SENM=0,0");
      // sniff is complicated, will have to wait.
//      hc05.wrt_cmmd("AT+SNIFF=0,0,0,0");
//          hc05.wrt_cmmd("AT+SNIFF=60,30,10,10");
      break;
      
      case '5':
        hc05.wrt_cmmd("AT+CLASS=1F00");
      break;
      
      case '@':
        hc05.wrt_cmmd("AT+RESET");        
      break;

      case 'A':
        hc05.wrt_cmmd("AT");
      break;      
      
      case 'B':  // Set user baudrate
        hc05.wrt_cmmd("AT+UART=38400,0,0");       
      break;
      
      case 'C':           
        // link is connect, switches to data mode if successful
        hc05.link(hc05.be[do_dev].nap, hc05.be[do_dev].uap, hc05.be[do_dev].lap);
      break;
            
      case 'D':  // Disconnect
        hc05.wrt_cmmd("AT+DISC");       
      break;
      
      case 'I':  // Start inquiry
        Serial.println("I Start");
        hc05.wrt_cmmd("AT+INQ");
      break;
      
      case 'L':  // Show device list
        for(int i=0;i<hc05.ndev;i++) {
          Serial.print("addr=");
          
          Serial.print(hc05.be[i].nap,HEX);
          Serial.print("-");
          Serial.print(hc05.be[i].uap,HEX);
          Serial.print("-");
          Serial.print(hc05.be[i].lap,HEX);          
          if(hc05.be[i].nam) {
            Serial.print(" nam=");
            Serial.print(hc05.be[i].nam);
          }
          Serial.print(" rssi=");
          Serial.print(hc05.be[i].rssi);
          Serial.print(" class=");
          Serial.println(hc05.be[i].clas,HEX);
        }       
      break;

      case 'P':
        // try to pair for up to 20 seconds, then stop trying.
        // pair does not connect, but is similiar. You must connect then disconnect to clear a pair.
        hc05.wrt_cmmd("AT+PAIR", hc05.be[do_dev].nap, hc05.be[do_dev].uap, hc05.be[do_dev].lap, 20);      
      break;

      case 'Q':
        // Query Rnames of devices in the inq list.
        // Occasionally gets things out of order?
        rv = true;
        Serial.println("Q Start");
        for(int i=0;i<hc05.ndev;i++) {
          if(hc05.be[i].nam == NULL) {
            requested_name = i;  // tell CB_RNAME where to store result
            hc05.wrt_cmmd("AT+RNAME?", hc05.be[i].nap, hc05.be[i].uap, hc05.be[i].lap);          
            rv = hc05.sync();
            if(rv)hc05.noApple((uint8_t *)hc05.be[i].nam);
          }
        } // for
        Serial.println("Q Done");
      break;
      
      // Refresh the Rname of the selected device.
      case 'R':     
        requested_name = do_dev;
        hc05.wrt_cmmd("AT+RNAME?", hc05.be[do_dev].nap, hc05.be[do_dev].uap, hc05.be[do_dev].lap);
        rv = hc05.sync();
        if(rv)hc05.noApple((uint8_t *)hc05.be[do_dev].nam);
      break;

      case 'S':
        if(hc05.state() == true)Serial.println("State_Pin:1");
        else Serial.println("State_Pin:0");
        
        hc05.wrt_cmmd("AT+STATE?");      
      break;

      case 'Z':
        // Zero the inquiry list
        // Make sure no inquiry is in progress when this is run.
        for(int i=0;i<hc05.ndev;i++) {
          if(hc05.be[i].nam != NULL) {
            free(hc05.be[i].nam);  // strdup() is free'd
            hc05.be[i].nam = NULL;
          }  
        }
        hc05.ndev = 0;
        do_dev = 0;
        requested_name = 0xff;
      break;
      
      // Advance to next device
      case '+':
        do_dev++;
        if(do_dev == hc05.ndev)do_dev = 0;
        Serial.print(hc05.be[do_dev].lap,HEX);
        Serial.println(hc05.be[do_dev].nam);
      break;

      // Previous device
      case '-':
        if(do_dev == 0)do_dev = (hc05.ndev-1);
        else do_dev--;
        Serial.print(hc05.be[do_dev].lap,HEX);
        Serial.println(hc05.be[do_dev].nam);
      break;
      
      case '!':  // Seq up (master) for inquiry
        hc05.wrt_cmmd("AT+INIT");
        rv = hc05.sync();
        if(rv) {
          hc05.wrt_cmmd("AT+IAC=9e8b33");
          rv = hc05.sync();
        }
        if(rv) {
          hc05.wrt_cmmd("AT+CLASS=", hc05.cached_class);  // when class is non 0, all INQ will be for specific class.
          rv = hc05.sync();
        }
        if(rv) {
          hc05.wrt_cmmd("AT+INQM=1,16,36");
          rv = hc05.sync();
        }
        if(rv) {
          hc05.wrt_cmmd("AT+INQ");
          rv = hc05.sync();
        }
      break;
      
      case '?':
        rv = true;
        if(rv) {
          hc05.wrt_cmmd("AT+VERSION?");
          rv = hc05.sync();
        }        
        if(rv) {
          hc05.wrt_cmmd("AT+UART?");
          rv = hc05.sync();
        }
        if(rv) {        
          hc05.wrt_cmmd("AT+ADDR?");
          rv = hc05.sync();
        }
        if(rv) {
          hc05.wrt_cmmd("AT+ROLE?");
          rv = hc05.sync();
        }
        if(rv) {        
          hc05.wrt_cmmd("AT+PSWD?");
          rv = hc05.sync();
        }
        if(rv) {
          hc05.wrt_cmmd("AT+CMODE?");
          rv = hc05.sync();
        }
        if(rv) {       
          hc05.wrt_cmmd("AT+NAME?");
          rv = hc05.sync();
        }
        if(rv) {
          hc05.wrt_cmmd("AT+CLASS?");
          rv = hc05.sync();
        }
        if(rv) {        
          hc05.wrt_cmmd("AT+IAC?");
          rv = hc05.sync();
        }
        if(rv) {        
          hc05.wrt_cmmd("AT+INQM?");
          rv = hc05.sync();
        }
        if(rv) {        
          hc05.wrt_cmmd("AT+BIND?");
          rv = hc05.sync();
        }
        if(rv) {        
          hc05.wrt_cmmd("AT+POLAR?");
          rv = hc05.sync();
        }
        if(rv) {         
          hc05.wrt_cmmd("AT+MPIO?");
          rv = hc05.sync();
        }
        if(rv) {        
          hc05.wrt_cmmd("AT+IPSCAN?");
          rv = hc05.sync();
        }
        if(rv) {
          hc05.wrt_cmmd("AT+SENM?");
          rv = hc05.sync();
        }
        if(rv) { 
          hc05.wrt_cmmd("AT+ADCN?");
          rv = hc05.sync();
        }
        if(rv) {   
          hc05.wrt_cmmd("AT+MRAD?");
          rv = hc05.sync();
        }
        if(!rv)Serial.println("Something failed");
      break;
      default:
        Serial.println("0,1,2,3,4,5,@,A,B,C,D,I,L,P,Q,R,S,Z,+,-,!,? are valid.");
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




