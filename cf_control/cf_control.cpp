#include "hystereseControllerClass.h"

hystereseControllerClass::hystereseControllerClass()
{
}

void hystereseControllerClass::init(float init_soll, float init_h, int init_ast)
{
	soll=init_soll;
  h=init_h;
	ast=init_ast;
}

void hystereseControllerClass::run(float ist)
{
	if(ast==0)
	{
		if(ist>soll-h)
			out=0;
		else
		{
			out=1;
			ast=1;
		}
	}
	else
	{
		if(ist<soll+h)
			out=1;
		else
		{
			out=0;
			ast=0;
		}
	}
}

int hystereseControllerClass::output()
{
  return out;
}

void hystereseControllerClass::set(float newsoll, float newh)
{
  soll = newsoll;
  h = newh;
}

void hystereseControllerClass::seth(float newh)
{
  h = newh;
}
