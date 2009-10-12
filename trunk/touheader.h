#include <stdint.h>
#include <iostream>
using namespace std;

class touheader
{
public :
unsigned long seq;
unsigned long magic;
unsigned long ack_seq;
unsigned short res1:4;
unsigned short doff:4;
unsigned short fin:1;
unsigned short syn:1;
unsigned short rst:1;
unsigned short psh:1;
unsigned short ack:1;
unsigned short rev:1;
unsigned short ece:1;
unsigned short cwr:1;
unsigned short wnd:16;

};
