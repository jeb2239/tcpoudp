/*******************************************************************************
 * test case
 * @ Nov 24, 2009 (sol) - basic test case.
 * @ Jan 09, 2010 (sol) - adding some logging mech.
 * @ Feb 01, 2011 		
 * @ Mar 01, 2011
 * ****************************************************************************/

#include "tou.h"
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netdb.h>

#define MAXSNDBUF 3000000 
#define TIMEOUTVAL 10 //ten seconds
using namespace std;


int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, 
			   unsigned short port){
	bzero(sockaddr, sizeof(*sockaddr));
	sockaddr->sin_family = sa_family;
	sockaddr->sin_port = htons(port);
	if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
	  return 0;
	return 1;
};

int main(int argc, char* argv[]){
	touMain				tm;
	struct sockaddr_in	socket1;
	struct sockaddr_in	socket2;
	char				send_data[MAXSNDBUF],recv_data[MAXSNDBUF];
	ifstream            indata;
	int					readsize; //how much data been read
	int					sendsize; //how much data been sent
	int					totalsendsize = 0;
	long				timedif, tsecs, tusecs;
	struct timeval		tstart;

	//LOGFLAG = TOULOG_PTSRN;
	//LOGFLAG = TOULOG_ALL;
	LOGFLAG = 0;
	LOGON = false; 

  if(argc == 4 && !strncmp( argv[3], "-c", 2)) /* ./tou 127.0.0.1 ./test_file -c */ {
		cout << "############ Client mode ############ " << endl;
		int selectval = 0;
		struct in_addr ipv4addr;
		int sd,sockd;
		int bytes_recieved; 

		//Set up select func socket
    	fd_set socks;
 		struct timeval tim;
		bool sentelapse = false; 

		//Get host name
		if (inet_pton(AF_INET, argv[1], &ipv4addr) <= 0)
			perror("[Server]inet_pton ");

		//Set socket 1: set up Server socket info
		memset(&socket1, NULL, sizeof(socket1));
		socket1.sin_addr.s_addr = inet_addr(argv[1]);
		socket1.sin_family = AF_INET;
		socket1.sin_port = htons(8888);

		/* assign server's addr */
		assignaddr(&socket1, AF_INET, argv[1], 8888);
		cout<<"# Sending msg to: "<< inet_ntoa(socket1.sin_addr)<< " "
		   	<<htons(socket1.sin_port) << "#\n";
		
		//Client socket
		sockd = tm.touSocket(AF_INET,SOCK_TOU,0);
	
		//set up Client socket info 
		memset(&socket2,0,sizeof(socket2));
		socket2.sin_family = AF_INET;
		socket2.sin_addr.s_addr = INADDR_ANY;
		socket2.sin_port = htons(1501);
	
		//BIND
		sd = tm.touBind(sockd,(struct sockaddr*) &socket2,sizeof socket2);
		cout << "touBind returns: "<< sd << endl;
	
		//CONNECT
		sd = tm.touConnect(sockd,(struct sockaddr_in*)&socket1,sizeof(socket1));
		cout << "touConnect returns: "<< sd << endl;
		
		//Reading the file
		indata.open(argv[2]); // opens the file
		if(!indata) { // file couldn't be opened
			cerr << "Error: file could not be opened" << endl;
			exit(1);
		}

		if(!indata.eof()) {
			cerr << "Reading File: "<< argv[2] << endl << endl;
			indata.read(send_data, MAXSNDBUF);
			readsize = indata.gcount();

			//SEND: touSend
			cerr << "|||SEND TIMESTAMP||| " << timestamp() << endl;
			gettimeofday(&tstart, NULL);//start counting

			sendsize = tm.touSend(sockd,send_data, readsize ,0);
			totalsendsize += sendsize;
		}

		while(true){
			FD_ZERO(&socks);
			FD_SET(sockd, &socks);
			tim.tv_sec = TIMEOUTVAL;
			tim.tv_usec = 0;

			if(!indata.eof() && tm.getCirSndBuf()>1197152) {
				if(!indata.eof()) {
					indata.read(send_data, 1100000);
					readsize = indata.gcount();
	
					//SEND: touSend
					sendsize = tm.touSend(sockd,send_data, readsize ,0);
					totalsendsize += sendsize;
					cout << "touSend: try to send "<< readsize << " bytes"
					<< " return "<< sendsize << " bytes\n"; 
				}
			}		

			selectval = select(sockd+1, &socks, NULL, NULL, &tim);
			if (selectval){
				//cout<< "Execute processTou->RUN: recv ACK"<< endl;
				tm.run(sockd);

			}else if(selectval == 0){
				break;
			}else{
				break;
			}

			if ( totalsendsize < readsize ) {
				sendsize = tm.touSend(sockd,send_data, readsize ,0);
				totalsendsize += sendsize;
			}

		}// while(true)  

		cout << "Exit!" << endl;
		cout << "Total send: "<< totalsendsize << endl;
		exit(1);
		tm.touClose(sockd);
		/* End of Client Code */
	
	}else if ( argc==2 && !strncmp(argv[1], "-s", 2)){
		cout << "############ Sever Mode ############"<< endl;
		int sd,bytes_recieved,lis_return;
		int sockd;
    	fd_set socks;
 		struct timeval tim;
		int recv_cnt = 0;

		//delete file for output
		if(remove("output") == -1)
			fprintf(stderr,"Remove file(output) failed");

		//file received output
		ofstream file_output("output",ios::app);

		//Set socket structures
		memset(&socket1, 0, sizeof(socket1));
		socket1.sin_family = AF_INET;
		socket1.sin_addr.s_addr = INADDR_ANY;
		socket1.sin_port = htons(8888);
		memset(&socket2, 0, sizeof(socket2));

		sockd = tm.touSocket(AF_INET,SOCK_TOU,0);
		sd = tm.touBind(sockd,(struct sockaddr*) &socket1,sizeof socket1);
		cout << "touBind returns: "<< sd << endl;
		lis_return = tm.touListen(sockd,1);
		cout << "touListen returns: "<< lis_return << endl;

		socklen_t sinlen = sizeof(socket2);
		tm.run(sockd);
		sd = tm.touAccept(sockd,(struct sockaddr_in*)&socket2,&sinlen);
		cout << "touAccept returns: "<< sd << endl;

		while(true){    
			FD_ZERO(&socks);
			FD_SET(sockd, &socks);
			tim.tv_sec = TIMEOUTVAL;

			if (select(sockd+1, &socks, NULL, NULL, &tim)){
				tm.run(sockd);
				
				//RECV: touRecv data
				memset(recv_data, 0, sizeof(recv_data));
				sd = tm.touRecv(sockd,recv_data,MAXSNDBUF,0);

				//output to file
				//file_output.write(recv_data, sd);
				recv_cnt += sd;

			}else{
				cout << "Select Timeout: Exit!" << endl;
				break;
			}      
		}
		
		file_output.close();
		cout << "CLOSING ... " << endl;
		cout << "# of recv_cnt: " << recv_cnt << endl;
		//exit(1);
		return 0;
		tm.run(sockd);
		tm.touClose(sockd);
       
  }else {/*END OF SERVER PART*/
	  cout << "Arguments Errors" << endl;

  }

	return 0;
}
