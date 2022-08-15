#include "datacollectorClass.h"
#include "Arduino.h"

datacollectorClass::datacollectorClass()
{
}

void datacollectorClass::init(int newbuffersize)
{
  for(int i=0; i<RINGBUFFER_SIZE; i++)
  {
    data[i] = 0.0f;
    statusflags[i] = ERR;
  }

  buffersize = newbuffersize;
}

void datacollectorClass::addData(float newdata)
{
  if(newdata>MINTEMP && newdata<MAXTREMP)
  {
    data[wp] = newdata;
    statusflags[wp] = OK;
  }
  else
  {
    data[wp] = 0.0f;
    statusflags[wp] = ERR;
  }
  
  wp++;
  if(wp>buffersize)   wp = 0;

  calc();
}

void datacollectorClass::calc()
{
  float mi = MAXTREMP;
  float ma = MINTEMP;

  int valid_data_points = 0;
  float av = 0.0f;
  
  for(int i=0; i<buffersize; i++)
  {
    if( statusflags[i]== OK )
    {
      if( data[i] < mi )  mi = data[i];
      if( data[i] > ma)   ma = data[i];

      av += data[i];
      valid_data_points++;
    }
  }
  minimum = mi;
  maximum = ma;
  avg = av * (1.0f / valid_data_points);
  
  if(valid_data_points>0)  valid = true;
  else                     valid = false;
}

float datacollectorClass::getMin()
{
  return minimum;
}

float datacollectorClass::getMax()
{
  return maximum;
}

float datacollectorClass::getAvg()
{
  return avg;
}

bool datacollectorClass::dataValid()
{
  return valid;
}
