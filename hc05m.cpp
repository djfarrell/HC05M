// hc05m.cpp v1.0 June 2014
// Bluetooth Master control library.
// Copyright (C) 2014 David Farrell @ Protoparts.
// See license.txt for terms.
// Tested on the Arduino Micro.
// Your HC05 module MUST have STATE and KEY lines. The modules
// with EN will not work properly.

#include <arduino.h>
#include "hc05m.h"

HC05M::HC05M(uint8_t _key, uint8_t _state) {
  key_pin = _key;
  state_pin = _state;
  operating_mode = MODE_UNDEF;
  u = NULL;

  timeout = 1000;
  is_master = false;
  previous_state = false;  
  ndev = 0;

  parse_state = 0;

  buffer_contains = BA_NONE;

  callback = NULL;
  post_callback = false;
  cached_class = 0x1f00;
}

void HC05M::begin(void) {

  link_attempt = false;

  digitalWrite(key_pin, LOW);
  pinMode(key_pin, OUTPUT);  
  pinMode(state_pin, INPUT);

  delay(5);  // small settling
  
  // Check to see if already connected, since there is no reliable way to determine
  // who I am connected to, so just disconnect.
  // Begin should always leave us in command mode.
  
  if(state() == HIGH) {
    setmode(MODE_CMMD);  
    wrt_cmmd("AT");
    delay(250);
    
    wrt_cmmd("AT+DISC");
    sync();
  }
  else {
    setmode(MODE_CMMD);
    wrt_cmmd("AT");
    delay(250);
  }
}

void HC05M::begin(class HardwareSerial *_u, uint32_t baud) {
  u = _u;
  u->begin(baud);
  u->setTimeout(timeout);
  begin();
}

bool HC05M::state(void) {
  return digitalRead(state_pin);
}

void HC05M::setmode(enum MODE mode) {
  operating_mode = mode;  
  if(mode == MODE_DATA)digitalWrite(key_pin, LOW);
  if(mode == MODE_CMMD)digitalWrite(key_pin, HIGH); 
}

// Write command, single reponse is "OK"
void HC05M::wrt_cmmd(char *cmd) {
//  cmmd_timestamp = millis();  // stamp the time the command was sent.
  u->print(cmd);
  u->print("\x0d\x0a");  
}

void HC05M::wrt_cmmd(char *cmd, uint16_t _class) {
//  cmmd_timestamp = millis();
  u->print(cmd);
  u->print(_class, HEX);
  u->print("\x0d\x0a"); 
}

uint8_t  HC05M::hex2asc(uint8_t d, char *p) {
  uint8_t t;
  uint8_t cnt = 2;   // needed if lead zero suppressed

  t = (d >> 4);
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;

  t = d & 0x0f;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;
  return cnt;
}

uint8_t HC05M::hex2asc(uint16_t d, char *p) {
  uint8_t t;
  uint8_t cnt = 4;  

  t = (d >> 12) & 0xf;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;

  t = (d >> 8) & 0xf;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;

  t = (d >> 4) & 0xf;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;

  t = d & 0x0f;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;
  return cnt;
}

uint8_t HC05M::hex2asc(uint24_t d, char *p) {
  uint8_t t;
  uint8_t cnt = 6;

  t = (d >> 20) & 0xf;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;

  t = (d >> 16) & 0xf;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;

  t = (d >> 12) & 0xf;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;

  t = (d >> 8) & 0xf;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;

  t = (d >> 4) & 0xf;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;

  t = d & 0x0f;
  if(t > 9 )t += ('A' - 10);
  else t += '0';
  *p++ = t;
//  cnt++;
  return cnt;
}

void HC05M::wrt_cmmd(char *_cmd, uint16_t nap, uint8_t uap, uint24_t lap) {  
  uint8_t n;  
//  cmmd_timestamp = millis();
  post_callback = false;
  
  u->print(_cmd);
//  Serial.print(_cmd);
  
  n = hex2asc(nap, (char *)(bf+n));
  *(bf+n)=',';
  n++;
  
  n += hex2asc(uap, (char *)(bf+n));
  *(bf+n)=',';
  n++;
  
  n += hex2asc(lap, (char *)(bf+n));
  *(bf+n)=0x0d;
  n++;
  *(bf+n)=0x0a;
  n++;

  *(bf+n) = 0; 
  u->print((char *)bf);
//  Serial.print((char *)bf);

}


void HC05M::wrt_cmmd(char *_cmd, uint16_t nap, uint8_t uap, uint24_t lap, uint16_t sec) {
  
  uint8_t n;
//  cmmd_timestamp = millis();
  post_callback = false;
  
  u->print(_cmd);
//  Serial.print(_cmd);
  
  *bf = '=';
  
  n = 1;
  n += hex2asc(nap, (char *)(bf+n));
  *(bf+n)=',';
  n++;
  
  n += hex2asc(uap, (char *)(bf+n));
  *(bf+n)=',';
  n++;
  
  n += hex2asc(lap, (char *)(bf+n));  
  *(bf+n) = ',';
  n++;
  *(bf+n) = 0;
  
  u->print((char *)bf);
//  Serial.print((char *)bf);
  u->println(sec);  
}

void HC05M::link(uint16_t nap, uint8_t uap, uint24_t lap) {  
  uint8_t n;  
//  cmmd_timestamp = millis();
  post_callback = false;
  link_attempt = true;
  
  u->print("AT+LINK=");
  
//  Serial.print("AT+LINK=");
  
  n = 0;
  n += hex2asc(nap, (char *)(bf+n));
  *(bf+n)=',';
  n++;
  
  n += hex2asc(uap, (char *)(bf+n));
  *(bf+n)=',';
  n++;
  
  n += hex2asc(lap, (char *)(bf+n));
  *(bf+n)=0x0d;
  n++;
  *(bf+n)=0x0a;
  n++;

  *(bf+n) = 0; 
//  Serial.println((char *)bf);
  u->print((char *)bf);    
}

// borrowed from serial stream code
RingBuffer::RingBuffer( void ) {
  memset(_aucBuffer, 0, STRM_BUFF_SIZE);
  _iHead = 0;
  _iTail = 0;
}

void RingBuffer::store_char( uint8_t c ) {
  int i = (uint32_t)(_iHead + 1) % STRM_BUFF_SIZE ;
  if ( i != _iTail ) {
    _aucBuffer[_iHead] = c ;
    _iHead = i ;
  }
}

// Stream functions
int HC05M::available( void ) {
  return (uint32_t)(STRM_BUFF_SIZE + Rx_buffer._iHead - Rx_buffer._iTail) % STRM_BUFF_SIZE;
}
int HC05M::peek( void ) {
  if(Rx_buffer._iHead == Rx_buffer._iTail )return -1 ;
  return Rx_buffer._aucBuffer[Rx_buffer._iTail] ;
}
int HC05M::read( void ) {
  // if the head isn't ahead of the tail, we don't have any characters
  if (Rx_buffer._iHead == Rx_buffer._iTail )return -1 ;

  uint32_t uc = Rx_buffer._aucBuffer[Rx_buffer._iTail] ;
  Rx_buffer._iTail = (uint32_t)(Rx_buffer._iTail + 1) % STRM_BUFF_SIZE ;
  return uc ;
}
void HC05M::flush( void ) {
  Rx_buffer._iHead = 0;
  Rx_buffer._iTail = 0;
}
size_t HC05M::write( const uint8_t uc_data ) {
  u->print((char)uc_data); 
  return 1;
}
size_t HC05M::write( const char uc_data ) {
  u->print(uc_data); 
  return 1;
}

// sync() is a timeout based wait for response to commands.
bool HC05M::sync(uint8_t sync_time) {  
  volatile static uint8_t timeout;
  timeout = sync_time;   // 12sec max timeout
  while(timeout) {
    timeout--;
    
    // Note how process() functions. Process() reads one char per call,
    // during delays I may need to check for many chars. Call process()
    // frequently.
    for(int i=0;i<20;i++) {
      delay(5);
      process();  
    }    
    if(post_callback)break;  // after (any) callback interval.
  }
  post_callback = false;
  return(timeout > 0);
}

void HC05M::setMaster(void) {
  bool brv;
  
  // Don't constantly write (paramter) flash of the HC05M if not needed, flash
  // writes generally have limited life.
  if((operating_mode == MODE_CMMD) && (is_master == false)) {
    wrt_cmmd("AT+INIT");  // may respond with error, but still continue.
    brv = sync();
    if(brv) {
      wrt_cmmd("AT+IAC=9e8b33");
      brv = sync();
    }
    if(brv) {
      wrt_cmmd("AT+PSWD=1234");
      brv = sync();
    }
    if(brv) {
      wrt_cmmd("AT+CMODE=0");
      brv = sync();
    }
    if(brv) {
      wrt_cmmd("AT+ROLE=1");
      brv = sync();
    }
    if(brv) {    
      wrt_cmmd("AT+CLASS=1F00");
      brv = sync();
    }
    if(brv) {    
      wrt_cmmd("AT+INQM=1,16,36");
      brv = sync();
    }
    if(brv) {    
      wrt_cmmd("AT+INQ");
      brv = sync();
    }
    if(brv) {    
      wrt_cmmd("AT+NAME=\"MSTxx\"");
      brv = sync();
    }
    if(brv) {    
      wrt_cmmd("AT+RESET");
      brv = sync();
    }
    is_master = true;    
  } 
}

void HC05M::setSlave(void) {
  bool brv;
  
  // Don't constantly write flash of HC05 if not needed.
  if((operating_mode == MODE_CMMD) && is_master) {
    wrt_cmmd("AT+ROLE=0");
    brv = sync();
    if(brv) {
      wrt_cmmd("AT+NAME=\"SLVxx\"");
      brv = sync();
    }
    if(brv) {    
      wrt_cmmd("AT+CLASS=1F00");
      brv = sync();
    }    
    if(brv) {
      wrt_cmmd("AT+PSWD=1234");
      brv = sync();
    }
    if(brv) {
      wrt_cmmd("AT+RESET");
      brv = sync();
    }
    is_master = false;
  }
}

void HC05M::noApple(uint8_t *s) {
  uint8_t *r;
  r = s;
  while(*r) {
    if(*r == 0xe2) {
      *s = '\'';
      r++;
      s++;
    }
    else if(*r > 0x7F)r++;    
    else {
      *s = *r;    
      s++;
      r++;
    }
  }
  *s = 0;
}

bool HC05M::parse_inq(void) {
  bool rv = false;
  uint16_t _nap;  // IEEE assigned
  uint16_t _uap;  // also IEEE but part lap
  uint24_t _lap;  // sent with packets    
  uint32_t _class; 
  uint16_t _rssi;    
  
  int i;
  i = hextouint((char *)bf, 4, &_nap);
  i++; // past ':'
    
  i += hextouint((char *)(bf+i), 2, &_uap);
  i++; // past ':'
    
  i += hextouint((char *)(bf+i), 6, &_lap);
  i++; // past ','
    
  i += hextouint((char *)(bf+i), 6, &_class);
  i++; // past ','
    
  i = hextouint((char *)(bf+i), 4, &_rssi);
  
  // Only save the class I care about, note any new (of my type)
  if(_class == cached_class) {
    if(rv = store_bt_slave(_nap, _uap, _lap, _rssi, _class)) {
//      Serial.print("NEW INQ:");
//      Serial.println((char *)bf);  
    }
//    else {
//      Serial.print("INQ:");
//      Serial.println((char *)bf);
//    }
  }
  return rv; 
}

void HC05M::process(void) {
  char c;
  bool new_state;
  
  // Detect non-command disconnect, maybe slave turned off?
  new_state = state();    
  if(previous_state && !new_state) {
    setmode(MODE_CMMD);
  }  
  previous_state = new_state;
  
  if(u->available()) {
    c = u->read();
    
//      if(c > 0x1f)Serial.write(c);
//      Serial.print("(0x");
//      Serial.print(c,HEX);
//      Serial.print(") ");
      
    if(operating_mode == MODE_CMMD) {
      
#ifdef SDBG      
      if(c > 0x1f)Serial.write(c);
      Serial.print("(0x");
      Serial.print(c,HEX);
      Serial.print(") ");
      Serial.print(" St=");
      Serial.println(parse_state);
#endif
    
      switch(parse_state) {
        
        case 0:
          if(c == '+')parse_state = 1;   // +xxxx:
          if(c == 'O')parse_state = 10;  // OK
          if(c == 'E')parse_state = 20;  // ERROR(x)
          if(c == 'F')parse_state = 30;  // FAIL
        break;
        
        case 1: // +xyyy
          if(c == 'A')parse_state = 25;      // address, adcn
          else if(c == 'B')parse_state = 35;  // bind
          else if(c == 'C')parse_state = 40; // class, cmode
          else if(c == 'D')parse_state = 45;  // disc
          else if(c == 'I')parse_state = 50;  // iac, inqm, ipscan, inq          
          else if(c == 'M')parse_state = 60; // mpio, mrad
          else if(c == 'N')parse_state = 65; // name
          else if(c == 'P')parse_state = 70; // pswd, polar, pio
          else if(c == 'R')parse_state = 75; // role, rname
          else if(c == 'S')parse_state = 80; // sniff, senm, state
          else if(c == 'U')parse_state = 90; // uart
          else if(c == 'V')parse_state = 95; // version
          else parse_state = 0;
        break;
          
        case 10: // OK
          if(c == 'K')parse_state++;
          else parse_state = 0;
        break;
        case 11: // 0x0d
          if(c == 0x0d) {
            parse_state = 210;
            buffer_contains = BA_OK;          
          }
          else parse_state = 0;
        break;
      
        case 20: // ERROR:(code)
          if(c == '(')parse_state++;
        break;
        case 21:
          last_error = ctouint8(c);
          bf[0] = c;
          parse_state++;
        break;    
        case 22:
          if(c != ')') {
            last_error <<= 4;
            last_error |= ctouint8(c);
            bf[1] = c;
            bf[2] = 0;
          }
          else {
            bf[1] = 0;
          }
          buffer_contains = BA_ERROR;         
          parse_state = 230;
        break;    
                
        case 25:  // ADDR or ADCN
          if(c == 'D')parse_state++;
          else parse_state = 0;
        break;
        case 26: // ADCN
          if(c == 'C')parse_state = 27;
          else if(c == 'D')parse_state = 29;
          else parse_state = 0;
        break;
        case 27: // ADCN
          if(c == ':')parse_state++;
        break;        
        case 28: //
          bf[0] = c;
          bf[1] = 0;
          buffer_contains = BA_ADCN;
          parse_state = 230;
        break;     
        case 29: // ADDR
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_ADDR;
            parse_state = 200; //220;
          }
        break;       

        case 30:  // FAIL
          bpnt = 0;
          buffer_contains = BA_FAIL;         
          parse_state = 200;
          if(link_attempt) {
            link_attempt = false;
          }
        break;
        
        case 35:  // BIND
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_BIND;
            parse_state = 200;
          }
        break;

        case 40:  // CLASS or CMODE
          if(c == 'L')parse_state = 42;
          else if(c == 'M')parse_state = 43;
          else parse_state = 0;
        break;        
        case 42:
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_CLASS;
            parse_state = 200;
          }           
        break;      
        case 43: // CMODE
          if(c == ':')parse_state++;
        break;  
        case 44:
          bf[0] = c;
          bf[1] = 0;
          buffer_contains = BA_CMODE;
          parse_state = 230;
        break;
  
  
        case 45:  // DISC
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_DISC;
            parse_state = 200;            
          }
        break;  
        
        
        case 50: // INQ: or INQM
          if(c == 'N')parse_state++;
          else if(c == 'A')parse_state = 57;
          else if(c == 'P')parse_state = 58;
          else parse_state = 0;        
        break;
        case 51:
          if(c == 'Q')parse_state++;
          else parse_state = 0;
        break;
        case 52:
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_INQ;
            parse_state = 200;
          }
          else if(c == 'M')parse_state = 53;
          else parse_state = 0;
        break;        
        case 53:  // INQM
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_INQM;
            parse_state = 200;
          }
        break;              
        case 57:  // IAC
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_IAC;
            parse_state = 200;            
          }
        break;        
        case 58:  // IPSCAN
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_IPSCAN;
            parse_state = 200;            
          }
        break;

                
        case 60: // mpio, mrad
          if(c == 'P')parse_state = 61;
          else if(c == 'R')parse_state = 62;
          else parse_state == 0;
        break;
        case 61: // MPIO
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_MPIO;
            parse_state = 200;
          }
        break;           
        case 62: // MRAD
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_MRAD;
            parse_state = 200;
          }
        break;   

        case 65: // +NAME
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_NAME;
            parse_state = 200;
          }
        break;
        
        case 70: // pswd, polar
          if(c == 'O')parse_state = 71;          
          else if(c == 'S')parse_state = 72;
          else parse_state = 0;
        break;
        case 71:
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_POLAR;
            parse_state = 200;
          }            
        break;       
        case 72:
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_PSWD;
            parse_state = 200;
          }            
        break;
        
        case 75: // ROLE, RNAME
          if(c == 'O')parse_state = 76;
          else if(c == 'N')parse_state = 78;
          else parse_state = 0;
        break;
        case 76:  // ROLE
          if(c == ':')parse_state = 77;
        break;      
        case 77:
          bf[0] = c;
          bf[1] = 0;
          if(c == '1')is_master = true;
          else is_master = false;
          buffer_contains = BA_ROLE;
          parse_state = 230;       
        break;
        
        case 78: 
          if(c == ':') {
            bpnt = 0; 
            buffer_contains = BA_RNAME;
            parse_state = 200;
          }
        break;
        
        case 80: //sniff, senm, state
          if(c == 'E')parse_state = 81;
          else if(c == 'N')parse_state = 82;
          else if(c == 'T')parse_state = 83;          
          else parse_state == 0;        
        break;
        case 81: // senm
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_SENM; 
            parse_state = 200;
          }
        break; 
        case 82: // sniff
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_SNIFF;
            parse_state = 200;
          }
        break;
        case 83: // state
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_STATE;
            parse_state = 200;
          }
        break;
        
        case 90: // UART
          if(c == ':') {
            bpnt = 0;
            buffer_contains = BA_UART;
            parse_state = 200;
          }
        break;
        
        case 95:  // version
          if(c == ':') {               
            bpnt = 0;
            buffer_contains = BA_VERSION;
            parse_state = 200;
          }
        break;

        case 200:  // build buffer until 0x0d
          if(c == 0x0d) {
            bf[bpnt] = 0;
//            if(buffer_contains == BA_STATE) {
              // Connected
//              if((bf[0] == 'C') && (bf[8] == 'D')) {
//              }
//            }
            parse_state = 210;
          }
          else {
            bf[bpnt++] = c;
          }          
        break;

        case 210:  // wait for 0x0a then invoke callback
          if(c == 0x0a) {              
            if(callback)(*callback)(buffer_contains);
            if(link_attempt) {
              link_attempt = false;
              if(buffer_contains == BA_OK) {
                setmode(MODE_DATA);   
              }                
            }
            if(buffer_contains == BA_OK) {
              post_callback = true;
            }
          }
          parse_state = 0;
        break;

        case 220:  // build buffer until O of OK  at the end of addr 
          if(c == 'O') {
            bf[bpnt] = 0;
            parse_state = 230;
          }
          else {
            bf[bpnt++] = c;
          }          
        break;

        case 230:  // wait for 0x0a then invoke callback
          if(c == 0x0a) {              
            if(callback)(*callback)(buffer_contains);
            if((buffer_contains == BA_OK) || (buffer_contains == BA_ERROR)) {
              post_callback = true;
            }   
            parse_state = 0;
          }
        break;        
      }
    } // operating_mode == MODE_CMMD
    
    if(operating_mode == MODE_DATA) {
      Rx_buffer.store_char(c); 
    }
  }  
}


// Parse one hex digit to binary
uint8_t HC05M::ctouint8(char c) {
  if((c >= 'A') && (c <= 'F')) {
    c -= 'A';
    c += 10;
  }
  else if((c >= '0') && (c <= '9')) {
    c -= '0';
  }  
  return(c);
}

// parse a specified number of HEX digits to 32 bit binary.
uint8_t HC05M::hextouint(char *b, uint8_t ndig, uint32_t *n) {
uint32_t d = 0;
uint8_t c;
uint8_t cnt = 0;

  while(ndig) {
    c = *b++;
    cnt++;
    
    if((c >= 'A') && (c <= 'F')) {
      c -= 'A';
      c += 10;
    }
    else if((c >= '0') && (c <= '9')) {
      c -= '0';
    }
    else { // non-hex digit
      *n = d;
      return cnt-1;
    }
    d <<= 4;     
    d |= c;
    ndig--;
  }
  *n = d;
  return cnt;
}

// parse a specified number of HEX digits to 16 bit binary.
uint8_t HC05M::hextouint(char *b, uint8_t ndig, uint16_t *n) {
uint16_t d = 0;
uint8_t c;
uint8_t cnt = 0;
  while(ndig) {    
    c = *b++;
    cnt++;    
    if((c >= 'A') && (c <= 'F')) {
      c -= 'A';
      c += 10;
    }
    else if((c >= '0') && (c <= '9')) {
      c -= '0';
    }
    else { // non-hex digit
      *n = d;
      return cnt-1;
    }    
    d <<= 4;     
    d |= c;
    ndig--;
  }
  *n = d;
  return cnt;
}

bool HC05M::store_bt_slave(uint16_t nap, uint8_t uap, uint24_t lap, int16_t rssi, uint32_t _class) {
  int i = 0;
    
  // Check if address already exits.
  for(i=0;i<ndev;i++) {        
    if(be[i].lap == lap) {
      be[i].rssi = rssi;  // update rssi
      return 0;
    }
  }

  // i left off at ndev
  if(i < MAX_BE) {
    be[i].nap = nap;
    be[i].uap = uap;
    be[i].lap = lap;
    be[i].rssi = rssi;
    be[i].clas = _class;
    be[i].nam = NULL;
    
    if(ndev < (MAX_BE-1))ndev++;
    return 1;  // return true if a new device was added  
  }
  
  return 0;
}


