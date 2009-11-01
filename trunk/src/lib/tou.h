/***************************************************
 * This is ToU library header file. Programs which 
 * want to use ToU library functions should include
 * this header file.
 **************************************************/
 
/******************************************************
 * Type define here
 * ***************************************************/
//Types
typedef	unsigned long 	u_long;
typedef	unsigned short	u_short;
typedef unsigned char	u_char;

//Timers
#define TOUT_TIMERS		4
#define TOUT_REXMIT		0
#define TOUT_PERSIST		1
#define TOUT_KEEP		2
#define TOUT_2MSL		3

//Ethernet 1500-20-24
#define TOU_MSS			14 

//Status
#define TOUS_CLOSED		0
#define TOUS_LISTEN		1
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
#include <iostream>
#include <sstream>
#include<vector>
/***************************************************
 * Include from BOOST library
 **************************************************/


/***************************************************
 * Include from self-define header
 **************************************************/
#include "timer.h"					//timer library
#include "trace.h"
#include "touControlBlock.h"				//ToU Control Block
#include "circularbuffer.h"
#include "touheader.h"					//touheader
#include "toucong.h"					//congestion



/******************************************************
 * socket table
 * ***************************************************/
class sockTb {
  public:
    touCb		*tc;			//tcp control block
    int			sockd;			//socket file descriptor
    u_short 		sport;
    u_short 		dport;
    std::string  	sip;
    std::string	  	dip;
    int 		cid;
    sockTb()
	{
		cid++;
	}
	
	//circular buf send 
	//circular buf recv
	//mutex
  void setcid(int a){
        cid = a;   
  }
  void printall()
  {
	cout<<"sockd : "<<sockd<<" sport :"<<sport<<" dport :"<<dport <<endl;
	cout<<" sip : "<<sip<<" dip : "<<dip<<" cid : "<< cid <<endl;
  }

};
vector<sockTb*> SS;
int cid_ =0;
class sockMng {
        
	
	public :
	sockTb *s;
	
  	void setSocketTable(sockaddr_in *sockettemp, int sd) {
		s = new sockTb;
		cout <<"Address : in  table " << inet_ntoa(sockettemp->sin_addr) <<endl;
		//cout<<"\tport:-> "<<ntohs(socket2->sin_port)<<endl;
		boost::mutex::scoped_lock lock(soctabmutex);
		s->sockd = sd;
		s->dport = 1500;
		s->sport = ntohs(sockettemp->sin_port);
		s->sip = inet_ntoa(sockettemp->sin_addr);
                s->setcid(cid_++);
		SS.push_back(s);

		for(int i = 0;i < SS.size();i++)
		{
				
			cout << "printing vector"<<endl ;
			cout << i << " : ";
			SS.at(i)->printall();
		}
	
	  }
	
	//void getSocketTable()
	
};

/******************************************************
 * tou main class
 * ***************************************************/
class tou {
  public :
    unsigned long 	test;
    touHeader 		t;
    int tou_socket(void);
    int tou_accept();
    int tou_bind();
    int tou_listen();
    int tou_send();
    int tou_recv();
    int tou_close();
};	

