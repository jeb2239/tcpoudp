#include <stdio.h>
#include <string.h>
#include "circularbuffer.h"

int main(int argc, char** argv) {
  CircularBuffer cb;
  int ret, start=0, end=0;
  char buf[100] = "";
  

  cb.setSize(8);
  cb.insert("123", 3);

  cb.remove(4);
  cb.insert("k",1);
  
  cb.print();
  printf("\n");
  cb.insert("abcdefghijklmnopqrstuv", 22);
  cb.remove(7);
  cb.print();
  printf("\n");
  cb.insert("12345678", 8);
  cb.print();
  cb.getAt(buf, 5, 7, end);
  printf("\nread from buf\n%s\nindex end:%d\n", buf, end);

  printf("\n");
  cb.print();
  
  //char testcase1[1000] = "f a\ta\na\0a f";
  //char testcase2[1000] = "abcdefghij";
  //cb.insert(testcase1, 11);
  //cb.remove(5);



  //cb.print();
  /*
  ret=cb.getTotalElements();
  printf("tot elements: %d\n", ret);

  start=cb.getHead();
  for(int i=0; i<cb.getSize(); i++) {
    ret=cb.getAt(buf, start, 1, end);
    start=end;
    printf("%d %d\n", buf[0], ret);
  }
  */

}//main
