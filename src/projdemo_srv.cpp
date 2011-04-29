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

	LOGFLAG = TOULOG_ALL | TOULOG_PKT;;
	LOGON = false;

	int sd,bytes_recieved,lis_return;
	int sockd;
  fd_set socks;
 	struct timeval tim;
	int recv_cnt = 0;

	//SOCKET 
	sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);

	//Set socket structures
	memset(&socket1,0,sizeof(socket1));
	socket1.sin_family = AF_INET;
	socket1.sin_addr.s_addr = INADDR_ANY;
	ocket1.sin_port = htons(1500);
	memset(&socket2,0,sizeof(socket2));

	//BIND
	sd = tm.touBind(sockd,(struct sockaddr*) &socket1,sizeof socket1);

	//LISTEN
	lis_return = tm.touListen(sockd,1);

	socklen_t sinlen = sizeof(socket2);
	tm.ptou->run(sockd);
	sd = tm.touAccept(sockd,(struct sockaddr_in*)&socket2,&sinlen);

	while(true){    
		FD_ZERO(&socks);
		FD_SET(sockd, &socks);
		tim.tv_sec = TIMEOUTVAL;
		if (select(sockd+1, &socks, NULL, NULL, &tim)){
				tm.ptou->run(sockd);
				sd = tm.touRecv(sockd,recv_data,MAXSNDBUF,0);
		}else{
				cout << "Select Timeout: Exit!" << endl;
				break;
		}      
	}

		exit(1);
		tm.ptou->run(sockd);
		tm.touClose(sockd);
}
