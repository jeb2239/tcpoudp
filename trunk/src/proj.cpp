/*******************************************************************************
 * test case
 * @ Nov 24, 2009 (sol) - basic test case.
 * @ Jan 09, 2010 (sol) - adding some logging mech.
 *		
 * ****************************************************************************/

#include "tou.h"
#include <fstream>
#include <sys/socket.h>
#include <netdb.h>

#define MAXSNDBUF 30000 
using namespace std;

int isClient = 0;

int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, unsigned short port){
	bzero(sockaddr, sizeof(*sockaddr));
	sockaddr->sin_family = sa_family;
	sockaddr->sin_port = htons(port);
	if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
	  return 0;
	return 1;
};

int main(int argc, char* argv[]){
	touMain							tm;
	struct sockaddr_in	socket1;
	struct sockaddr_in	socket2;
	char								send_data[MAXSNDBUF],recv_data[MAXSNDBUF];
	ifstream            indata;
	int									readsize; //how much data been read
	int									sendsize; //how much data been sent

	//logging
	LOGFLAG = TOULOG_PTSRN;

  if(argc == 4 && !strncmp( argv[3], "-c", 2)) /* ./tou 127.0.0.1 ./test_file -c */
	{
		cout << " ############ Client mode ############ " << endl;
		/* for test */
		isClient = 1;
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
    if (h == NULL)  perror("[Server]gethostbyaddr: ");

    //Set socket 1: set up Server socket info
    memset(&socket1, 0, sizeof(socket1));
   	socket1.sin_family = h->h_addrtype;
   	memcpy((char *) &socket1.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    socket1.sin_port = htons(1500);

		/* assign server's addr */
		assignaddr(&socket1, AF_INET, argv[1], 1500);
		std::cout<<"# Sending msg to: "<< inet_ntoa(socket1.sin_addr)<< " " <<htons(socket1.sin_port) << endl;
		
		//Client socket
		sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
		cout << "touSocket returns: " << sockd << endl;
	
		//set up Client socket info 
		memset(&socket2,0,sizeof(socket2));
		socket2.sin_family = AF_INET;
		socket2.sin_addr.s_addr = INADDR_ANY;
		socket2.sin_port = htons(1501);
	
		//BIND //CLIENT SHOULD NOT BINE?????????????????? Chinmay Make it _TODO_ list
		sd = tm.touBind(sockd,(struct sockaddr*) &socket2,sizeof socket2);
		cout << "touBind returns: "<< sd << endl;
	
		//CONNECT
		sd = tm.touConnect(sockd,(struct sockaddr_in*)&socket1,sizeof(socket1));
		cout << "touConnect returns : "<< sd << endl;
		
		//Initialize process tou
		tm.proTou(sockd);

		//Reading the file
		indata.open(argv[2]); // opens the file
		if(!indata) { // file couldn't be opened
			cerr << "Error: file could not be opened" << endl;
			exit(1);
		}
		if( !indata.eof() ) {
		  cerr << "Reading data from file: "<< argv[2] << std::endl;
			indata.read(send_data, MAXSNDBUF);
			readsize = indata.gcount();

      //SEND: touSend
			sendsize = tm.touSend(sockd,send_data, readsize ,0);
			cout << "touSend: Trying to send "<< readsize << "bytes ";
			cout << "touSend returns "<<sendsize << " bytes" << endl; 
		}

		while(true){
			FD_ZERO(&socks);
			FD_SET(sockd, &socks);
			tim.tv_sec = 1;
			tim.tv_usec = 0;

			selectval = select(sockd+1, &socks, NULL, NULL, &tim);
			cout<< "\n >>> NEW ITERATION >>> Select LOOP: select returns: "<<selectval<< endl; 
			if (selectval){
				cout<< "Execute processTou->RUN: recv ACK"<< endl;
				tm.ptou->run(sockd);
			}else if(selectval == 0){
				cerr<< ">> Select timeout: leave Select LOOP"<< endl;
				break;
			}else{
				break;
			}
		} 

		cout << "Exit!" << endl;
		exit(1);
		tm.touClose(sockd);
		/* End of Client Code */
	
	}else if ( argc==2 && !strncmp(argv[1], "-s", 2)){
		cout << " ############ Sever Mode ############"<< endl;
		int sd,bytes_recieved,lis_return;
		int sockd;
		//Check if it is tou socket
    fd_set socks;
 		struct timeval tim;

		//CREATE 
		sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
		cout << "touSocket returns: " << sockd << endl;

		//Set socket structures
		memset(&socket1,0,sizeof(socket1));
		socket1.sin_family = AF_INET;
		socket1.sin_addr.s_addr = INADDR_ANY;
		socket1.sin_port = htons(1500);
		memset(&socket2,0,sizeof(socket2));

		//BIND
		sd = tm.touBind(sockd,(struct sockaddr*) &socket1,sizeof socket1);
		cout << "touBind returns: "<< sd << endl;

		//LISTEN
		lis_return = tm.touListen(sockd,1);
		cout << "touListen returns: "<< lis_return << endl;

		//Initialize processTou
		tm.proTou(sockd);

		socklen_t sinlen = sizeof(socket2);
		tm.ptou->run(sockd);
		sd = tm.touAccept(sockd,(struct sockaddr_in*)&socket2,&sinlen);
		cout << "touAccept returns: "<< sd << endl;

		while(true){    
			FD_ZERO(&socks);
			FD_SET(sockd, &socks);
			tim.tv_sec = 5;

			if (select(sockd+1, &socks, NULL, NULL, &tim)){
				tm.ptou->run(sockd);
				
				//RECV: touRecv data
				memset(recv_data, 0, sizeof(recv_data));
				sd = tm.touRecv(sockd,recv_data,MAXSNDBUF,0);
				cout << "touRecv returns: "<< sd << " recv buffer size: " << sizeof recv_data << endl;

			}else{
				cout << "Select Timeout: Exit!" << endl;
				break;
			}      
		}
	
		
		cout << "Inside Close...... " << endl;
		exit(1);
		tm.ptou->run(sockd);
		tm.touClose(sockd);
       
  }/*END OF SERVER PART*/

	return 0;
}
