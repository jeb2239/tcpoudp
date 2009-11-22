/******************************************************
 * tou.h
 * This is ToU library header file. User who wants to 
 * use ToU library functions hould include this header 
 * file once and only.
 *****************************************************/
 
/******************************************************
 * Type define here
 * ***************************************************/
//Types
typedef	unsigned long 	u_long;
typedef	unsigned short	u_short;
typedef unsigned char	u_char;

//Timers
#define TOUT_TIMERS			4
#define TOUT_REXMIT			0
#define TOUT_PERSIST		1
#define TOUT_KEEP			2
#define TOUT_2MSL			3

//Ethernet 1500-20-24
#ifndef TOU_MSS
#define TOU_MSS				1456
#endif

//Status
#define TOUS_CLOSED			0
#define TOUS_LISTEN			1
#define TOUS_SYN_SENT		2
#define TOUS_SYN_RECEIVED	3
#define TOUS_ESTABLISHED	4
#define TOUS_CLOSE_WAIT		5
#define TOUS_FIN_WAIT_1		6
#define TOUS_CLOSING		7
#define TOUS_LAST_ACK		8
#define TOUS_FIN_WAIT_2		9
#define TOUS_TIME_WAIT		10

/***************************************************
 * Include from STD library
 **************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>

#include <vector>
/***************************************************
 * Include from BOOST library
 **************************************************/

/***************************************************
 * Include from self-define header
 **************************************************/
#include "timer.h"				//tou timer library
#include "processtou.h"
#include "trace.h"				//for debug

//#include "touHeader.h"			//tou header & header mng
//#include "touSockTable.h"
//#include "touCongestion.h"


extern FILE *_fptrace;
extern timerMng tm1;

/******************************************************
 * tou main class
 * ***************************************************/
class touMain {
	private:
  int sd;
  int yes;
  /* for sending: pushing data into circular buf */
	int pushsndq(int sockfd, char *sendbuf, int &len);
	
	public:
  touPkg tp;
	/* TOU socket table management */
  sockMng sm;
  processTou *ptou;	/* process TOU*/	
	int touSocket(int , int , int );
	int touAccept(int , struct sockaddr_in *, socklen_t * );
	int touConnect(int , struct sockaddr_in *, int );
	int touBind(int , struct sockaddr *, int );
	int touListen(int, int);
	int touSend(int , char *, int , int );
	int touRecv(int , char *, int , int );
	int touClose(int);
	
	int proTou(int sockfd) {
	ptou = new processTou(sockfd, &sm);
	}

	void convertToByteOrder(touPkg&);
	void convertFromByteOrder(touPkg&);
	int timero(int , int);
	int assignaddr(struct sockaddr_in *, sa_family_t , std::string , u_short);
	
};	
