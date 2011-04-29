#include "tou.h"
#include <iostream>
#define MAXSNDBUF 2000000 
#define TIMEOUTVAL	4				//four seconds
using namespace std;

int main(int argc, char* argv[]){
	touMain							tm;
	struct sockaddr_in	socket1;
	struct sockaddr_in	socket2;
	char								send_data[MAXSNDBUF],recv_data[MAXSNDBUF];
	ifstream            indata;
	long								timedif, tsecs, tusecs;
	struct timeval			tstart;

	LOGFLAG = TOULOG_ALL | TOULOG_PKT;
	LOGON = true;

	int selectval = 0;
	struct in_addr ipv4addr;
	int sd,sockd;
	int bytes_recieved; 
	struct hostent *h;

	//Set up select func socket
   fd_set socks;
 	struct timeval tim;

	//Get host name
	inet_pton(AF_INET, argv[1], &ipv4addr);
	h = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);

  //Set socket 1: set up Server socket info
  memset(&socket1, 0, sizeof(socket1));
  socket1.sin_family = h->h_addrtype;
  memcpy((char *) &socket1.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  socket1.sin_port = htons(1500);

	/* assign server's addr */
	assignaddr(&socket1, AF_INET, argv[1], 1500);
		
	//Socket
	sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
	
	memset(&socket2,0,sizeof(socket2));
	socket2.sin_family = AF_INET;
	socket2.sin_addr.s_addr = INADDR_ANY;
	socket2.sin_port = htons(1501);
	
	//BIND
	sd = tm.touBind(sockd,(struct sockaddr*) &socket2,sizeof socket2);
	
	//CONNECT
	sd = tm.touConnect(sockd,(struct sockaddr_in*)&socket1,sizeof(socket1));
		
	//Reading the file
	indata.open(argv[2]); // opens the file
	indata.read(send_data, MAXSNDBUF);
	readsize = indata.gcount();

	sendsize = tm.touSend(sockd,send_data, readsize ,0);

	while(true){
		FD_ZERO(&socks);
		FD_SET(sockd, &socks);
		tim.tv_sec = TIMEOUTVAL;
		tim.tv_usec = 0;

		selectval = select(sockd+1, &socks, NULL, NULL, &tim);
		if (selectval){
				tm.ptou->run(sockd);
		}else if(selectval == 0){
				break;
		}
	}// while(true)  

		exit(1);
		tm.touClose(sockd);

	return 0;
}
