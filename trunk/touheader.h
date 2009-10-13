#include <stdint.h>
#include <iostream>

/******************************************************
 * Type define here
 * ***************************************************/
typedef	unsigned long 	u_long;
typedef	unsigned short	u_short;
typedef unsigned char	u_char;

/******************************************************
 * ToU header
 * ***************************************************/
class touheader {
  public:
    u_long	seq;
    u_long	mag;
    u_long	ack_seq;
    u_short	doff:4,
		rsv:4;
    u_char	flags;
#define	TOU_FIN	0x01
#define	TOU_SYN	0x02
#define	TOU_RST	0x04
#define	TOU_PSH	0x08
#define	TOU_ACK	0x10
#define	TOU_RSV	0x20
#define	TOU_ECE	0x40
#define	TOU_CWR	0x80
#define	TOU_FLAGS (TOU_FIN|TOU_SYN|TOU_RST|TOU_PSH|TOU_ACK|TOU_RSV|TOU_ECE|TOU_CWR)
    u_short	wnd:16;
};


/* //original def
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
*/



