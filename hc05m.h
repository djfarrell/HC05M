// hc05m.h v1.0 June 2014
// Bluetooth Master control library.
// Copyright (C) 2014 David Farrell @ Protoparts.
// See license.txt for terms.
// Tested on the Arduino Micro.
// Your HC05 module MUST have STATE and KEY lines. The modules
// with EN will not work properly.

// Some info:
// iPhone class is 0x7A020C
// Samsung phone class is 0x5A02
// I use class 0x1F00 for HC05 slaves.

#ifndef _HC05M_H
#define _HC05M_H

#include "Arduino.h"
#include <inttypes.h>

// I need a 24 bit type for clarity
typedef uint32_t uint24_t;
    
//#define SDBG

// Class of Device/Service
// Major Service Class
#define MSC_INFORMATION (1<<23)
#define MSC_TELEPHONY (1<<22)
#define MSC_AUDIO (1<<21)
#define MSC_OBJECT_TRANSFER (1<<20)

#define MSC_CAPTURING (1<<19)
#define MSC_RENDERINF (1<<18)
#define MSC_NETWORKING (1<<17)
#define MSC_POSITIONING (1<<16)

#define MSC_LIMITED (1<<13)

// bits 12..8 Major Device Class
#define MDC_MISC (0<<8)
#define MDC_COMPUTER (1<<8)
#define MDC_PHONE (2<<8)
#define MDC_LAN (3<<8)
#define MDC_AUDIO (4<<8)
#define MDC_PERIPHERAL (5<<8)
#define MDC_IMAGING (6<<8)
#define MDC_UNCAT (0x1f<<8)

// Minor classes too numerous for me to type.
#define IAC_GIAC "AT+IAC=9E8B33"
#define IAC_LIAC "AT+IAC=9e8B00"
#define IAC_RFU0 "AT+IAC=9e8B01"
#define IAC_RFU1 "AT+IAC=9e8B32"
#define IAC_RFU3 "AT+IAC=9e8B34"
#define IAC_RFU4 "AT+IAC=9e8B3F"

enum BUF_ARG {
  BA_NONE = (uint8_t )0, BA_IAC, BA_PSWD, BA_IPSCAN, BA_VERSION,
  BA_NAME, BA_ADDR, BA_INQM, BA_INQ, BA_BIND,
  BA_DISC, BA_UART, BA_CLASS, BA_POLAR, BA_MRAD,   
  BA_MPIO, BA_RNAME, BA_SENM, BA_SNIFF, BA_STATE,
  BA_ERROR, BA_ADCN, BA_CMODE, BA_ROLE,
  BA_OK, BA_FAIL
};

#define MAX_BF 40
#define MAX_BE 8

#define STRM_BUFF_SIZE 16
class RingBuffer
{
  public:
    uint8_t _aucBuffer[STRM_BUFF_SIZE];
    uint32_t _iHead ;
    uint32_t _iTail ;
    RingBuffer( void ) ;
    void store_char(uint8_t c) ;
} ;

class HC05M : public Stream
{
  public:  
    typedef void (*cmdcallback) (enum BUF_ARG arg);
    cmdcallback callback;
    
    HC05M(uint8_t _cmd = 6, uint8_t _mode = 7);
    void begin(void);
    void begin(class HardwareSerial *_u, uint32_t baud = 38400);

    enum MODE {MODE_CMMD = (uint8_t )0, MODE_DATA = (uint8_t )1, MODE_UNDEF = (uint8_t)2};
    void setmode(enum MODE mode);
    
    void setMaster(void);  // config HC05 as master
    void setSlave(void);   // config HC05 as slave
    bool state(void);      // read the state I/O pin
    
    bool parse_inq(void);  // parse inq response
      
    bool store_bt_slave(uint16_t nap, uint8_t uap, uint24_t lap, int16_t rssi, uint32_t clas);
    
    void wrt_cmmd(char *cmd);
    void wrt_cmmd(char *cmd, uint16_t _class);
    void wrt_cmmd(char *cmd, uint16_t nap, uint8_t uap, uint24_t lap);
    void wrt_cmmd(char *cmd, uint16_t nap, uint8_t uap, uint24_t lap, uint16_t sec);
    
    void link(uint16_t nap, uint8_t uap, uint24_t lap);    
    void noApple(uint8_t *s);
      
    void process(void);  // main process (call frequently)
   
    uint8_t bf[MAX_BF];  // response buffer
    uint8_t bpnt;        // build string running index
    
    // List of devices seen via inq
    struct BLUE_E {      
      uint16_t nap; // IEEE assigned
      uint8_t uap;  // also IEEE but part lap
      uint24_t lap; // sent with packets   
      char *nam;    // I limit to 16
      uint32_t clas;
      int16_t rssi;
    };
    struct BLUE_E be[MAX_BE];  // up to 8 devices
    uint8_t ndev;

    bool sync(uint8_t sync_time = 120);
    uint32_t cached_class;
  
    // stream
    int available( void ) ;
    int peek( void ) ;
    int read( void ) ;
    void flush( void ) ;
    size_t write( const uint8_t c ) ;
	size_t write( const char c ) ;
	
#if defined __GNUC__ /* GCC CS3 */
	using Stream::print ;
	
    using Print::write ; // pull in write(str) and write(buf, size) from Print
#elif defined __ICCARM__ /* IAR Ewarm 5.41+ */
//    virtual void write( const char *str ) ;
//    virtual void write( const uint8_t *buffer, size_t size ) ;
#endif

  protected:

    // bin to ascii hex
    uint8_t hex2asc(uint8_t d, char *p);
    uint8_t hex2asc(uint16_t d, char *p);
    uint8_t hex2asc(uint24_t d, char *p);

    // ascii hex to bin
    uint8_t ctouint8(char c);
    uint8_t hextouint(char *b, uint8_t ndig, uint32_t *n);
    uint8_t hextouint(char *b, uint8_t ndig, uint16_t *n);
    
    RingBuffer Rx_buffer;
    
    bool post_callback;
    bool previous_state;    
    bool link_attempt;
    
    enum BUF_ARG buffer_contains;

    uint8_t last_error;
//    uint32_t cmmd_timestamp;
    
    uint8_t key_pin;
    uint8_t state_pin;
    class HardwareSerial *u;
    
    enum MODE operating_mode;
    bool is_master;
    
    uint16_t timeout;
    
    uint8_t parse_state;
};

#endif

