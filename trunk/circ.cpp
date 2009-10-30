#include <stdio.h>

#include "circularbuffer.h"


int main(int argc, char** argv) {
  CircularBuffer cb;
  int ret, start, end;
  char buf[10];

  cb.setSize(3);
  cb.insert("sal", 3);
  ret=cb.getTotalElements();

  printf("tot elements: %d\n", ret);

  cb.remove(1);

  cb.insert("b",1);

  //cb.print();

  start=cb.getHead();
  for(int i=0; i<cb.getSize(); i++) {
    ret=cb.getAt(buf, start, 1, end);
    start=end;
    printf("%c %d\n", buf[0], ret);
  }

}//main
