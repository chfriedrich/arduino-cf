#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#ifdef DEBUG

  #if DEBUG == 1
    #define DEBUG_PRINT(x) Serial.print (x)
    #define DEBUG_PRINTDEC(x) Serial.print (x, DEC)
    #define DEBUG_PRINTHEX(x) Serial.print (x, HEX)
    #define DEBUG_PRINTLN(x) Serial.println (x)

    #define DEBUG2_PRINT(x)
    #define DEBUG2_PRINTDEC(x)
    #define DEBUG2_PRINTHEX(x)
    #define DEBUG2_PRINTLN(x)
  
  #elif DEBUG == 2
    #define DEBUG_PRINT(x) Serial.print (x)
    #define DEBUG_PRINTDEC(x) Serial.print (x, DEC)
    #define DEBUG_PRINTHEX(x) Serial.print (x, HEX)
    #define DEBUG_PRINTLN(x) Serial.println (x)

    #define DEBUG2_PRINT(x) Serial.print (x)
    #define DEBUG2_PRINTDEC(x) Serial.print (x, DEC)
    #define DEBUG2_PRINTHEX(x) Serial.print (x, HEX)
    #define DEBUG2_PRINTLN(x) Serial.println (x)
  #endif
  
#else

  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTHEX(x) 
  #define DEBUG_PRINTLN(x) 

  #define DEBUG2_PRINT(x)
  #define DEBUG2_PRINTDEC(x)
  #define DEBUG2_PRINTHEX(x)
  #define DEBUG2_PRINTLN(x)
  
#endif

#endif
