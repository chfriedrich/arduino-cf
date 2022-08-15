#ifndef hystereseControllerClass_h
#define hystereseControllerClass_h

class hystereseControllerClass
{
  private:
	  float soll = 0.0f;
	  int   ast  = 0 ;
    float h    = 0.0f;
    int   out  = 0;
    
  public:
	  hystereseControllerClass();
	  void init(float init_soll, float init_h, int init_ast);
	  void run(float);
    int output();
    void set(float, float);
    void seth(float);
  
};

#endif
