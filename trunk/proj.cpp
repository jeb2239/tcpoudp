#include "timer.h"

int main(){
  timerMng timermng;
  for(int i=1;i<=380; i++)
    timermng.add(i, i*1000);
 
  sleep(1);
  timermng.delete_timer(3);
  timermng.delete_timer(5);

  while(1);
  return 0;
}

