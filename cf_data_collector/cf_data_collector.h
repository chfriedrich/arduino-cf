#ifndef DataCollector_h
#define DataCollector_h

#include "Arduino.h"

#define RINGBUFFER_SIZE 1024  // maximum size, = array size

#define ERR 0
#define OK 1


class DataCollector
{
  private:
    float data[RINGBUFFER_SIZE];
    int   statusflags[RINGBUFFER_SIZE];
    int   wp = 0; //writepointer
	
	bool  checklimits = false;
	float minlimit = 0.0f;
	float maxlimit = 0.0f;
	
    float minimum = 0.0f;
    float maximum = 0.0f;
    float avg = 0.0f;
	
    bool  valid = false;
    int   buffersize = 0;

  public:
    DataCollector();
    void init(int newbuffersize);
    void addData(float newdata);
    void calc();
    float getMin();
    float getMax();
    float getAvg();
    bool dataValid();
};

#endif
