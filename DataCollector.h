#ifndef datacollectorClass_h
#define datacollectorClass_h

#include "datacollectorClass.h"
#include "Arduino.h"

#define RINGBUFFER_SIZE 128  // maximum size, = array size

#define ERR 0
#define OK 1

#define MINTEMP -40.0f
#define MAXTREMP 60.0f


class datacollectorClass
{
  private:
    float data[RINGBUFFER_SIZE];
    int   statusflags[RINGBUFFER_SIZE];
    int   wp = 0; //writepointer
    float minimum = 0.0f;
    float maximum = 0.0f;
    float avg = 0.0f;
    bool  valid = false;
    int   buffersize = 0;

  public:
    datacollectorClass();
    void init(int newbuffersize);
    void addData(float newdata);
    void calc();
    float getMin();
    float getMax();
    float getAvg();
    bool dataValid();
};

#endif
