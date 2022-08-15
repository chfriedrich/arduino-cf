#include "cf_data_collector.h"
#include "Arduino.h"

DataCollector::DataCollector()
{
}

void DataCollector::init(int newbuffersize)
{
  for(int i=0; i<RINGBUFFER_SIZE; i++)
  {
    data[i] = 0.0f;
    statusflags[i] = ERR;
  }

  buffersize = newbuffersize;
}

void DataCollector::addData(float newdata)
{
  int   f = OK;
  float v = newdata;
  
  if( checklimits && (newdata<minlimit || newdata>maxlimit) )
	{
		f = ERR;
  }
	
	data[wp] = v;
	statusflags[wp] = f;
  wp++;
  if(wp>buffersize)   wp = 0;

  calc();
}

void DataCollector::calc()
{
  float mi = maxlimit;
  float ma = minlimit;

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
  avg = av / valid_data_points;
  
  if(valid_data_points>0)  valid = true;
  else                     valid = false;
}

float DataCollector::getMin()
{
  return minimum;
}

float DataCollector::getMax()
{
  return maximum;
}

float DataCollector::getAvg()
{
  return avg;
}

bool DataCollector::dataValid()
{
  return valid;
}
