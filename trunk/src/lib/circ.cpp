#include <stdio.h>
#include <string.h>
#include "circularbuffer.h"

int main(int argc, char** argv) {
  CircularBuffer cb;
  int ret, start, end;
  char buf[10];

  cb.setSize(100);
  cb.insert("sal", 3);

  cb.remove(1);
  cb.insert("b",1);

  char testcase1[1000] = "f a\ta\na\0a f";
  char testcase2[1000] = "abcdefghij";
  cb.insert(testcase1, 11);
  //cb.remove(5);



  //cb.print();
  ret=cb.getTotalElements();
  printf("tot elements: %d\n", ret);

  start=cb.getHead();
  for(int i=0; i<cb.getSize(); i++) {
    ret=cb.getAt(buf, start, 1, end);
    start=end;
    printf("%d %d\n", buf[0], ret);
  }

}//main
