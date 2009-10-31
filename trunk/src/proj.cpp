#include "timer.h"
#include "trace.h"

int main(){
  FILE* _fptrace;
  _fptrace = fopen("debug.txt", "w");
  fprintf(_fptrace, "abc%d\n", 1);

  printf("Hello\n");
  TRACE(5,"TRACE test: %d LObject map: %d\n", 1, 2 );

/* test code for timer 
  timerMng timermng;
  for(int i=1,j=1;i<=15000; i++,j++)
  {
    timermng.add(j,i+1, (long)i*1000, i+2); //here the first and last parameter will come automatically from connection
    timermng.add(j,(i*1000)+1, (long)i*1000 + 1, (i*1000)+2);
  }
  timermng.add(1,3333,4000,101);
  sleep(1);
  timermng.delete_timer(1, 4, 5);
  timermng.delete_timer(2, 5, 6);

  while(1);
  
*/


  return 0;
}

