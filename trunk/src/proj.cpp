/*******************************************************************************
 * test case
 * revised by sol @ Nov 24, 2009
 * ****************************************************************************/

#include "tou.h"
#include <fstream>
#include <sys/socket.h>
#include <netdb.h>

#define MAXSNDBUF 20000 
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

	std::cout << " Welcom to ToU \n" ;
  if(argc == 4 && !strncmp( argv[3], "-c", 2)) /* ./tou 127.0.0.1 ./test_file -c */
	{
		cout << "Client mode.. " << endl;

		/* for test */
		isClient = 1;
		int selectval = 0;


		int sd,sockd;
		int bytes_recieved; 
		struct hostent *h;
		//Get host name
		if( (h=gethostbyname(argv[1])) == NULL ) 
      printf("%s: unknown host '%s' \n", argv[0], argv[1]);

    //printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name, inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));
    
    //Set socket 1
	  //set up Server socket info
		//
    memset(&socket1, 0, sizeof(socket1));
		std::cerr <<  h->h_addrtype << " " << argv[1] <<std::endl;
   	socket1.sin_family = h->h_addrtype;
		std::cerr <<  h->h_addr_list[0] <<std::endl;
   	memcpy((char *) &socket1.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    socket1.sin_port = htons(1500);

		/* assign server's addr */
		assignaddr(&socket1, AF_INET, "127.0.0.1", 1500);
		std::cout<<inet_ntoa(socket1.sin_addr)<< "  " <<htons(socket1.sin_port) << std::endl;

		//Client socket
		sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
	
		//set up Client socket info 
		memset(&socket2,0,sizeof(socket2));
		socket2.sin_family = AF_INET;
		socket2.sin_addr.s_addr=inet_addr("127.0.0.1");
		socket2.sin_port = htons(1501);
	
		//BIND //CLIENT SHOULD NOT BINE?????????????????? Chinmay Make it _TODO_ list
		sd = tm.touBind(sockd,(struct sockaddr*) &socket2,sizeof socket2);
	
		//CONNECT
		sd = tm.touConnect(sockd,(struct sockaddr_in*)&socket1,sizeof(socket1));
		cout << "Connect returns : "<< sd << endl;

		//set up select func socket
    fd_set socks;
 		struct timeval tim;
		/*
 		FD_ZERO(&socks);
 		FD_SET(sockd, &socks);
 		tim.tv_sec = 2;
		tim.tv_usec = 0;
		*/

		/* reading file */
		indata.open(argv[2]); // opens the file
		if(!indata) { // file couldn't be opened
			cerr << "Error: file could not be opened" << endl;
			exit(1);
		}
		
		//Initialize process tou
		tm.proTou(sockd);


			if( !indata.eof() ) {
			  cerr << "Reading data from file: "<< argv[2] << std::endl;
				indata.read(send_data, MAXSNDBUF);
				readsize = indata.gcount();

        /* touSend */
				sendsize = tm.touSend(sockd,send_data, readsize ,0);
				std::cout << "Call touSend: Trying to send "<< readsize<< "; touSend returns "<<sendsize << " bytes. \n";
				
				tm.ptou->send(sockd);
			}else {
				 cout << " *** EOF *** \n";
			}


		while(1)
		{
			FD_ZERO(&socks);
			FD_SET(sockd, &socks);
			tim.tv_sec = 1;
			tim.tv_usec = 0;

			selectval = select(sockd+1, &socks, NULL, NULL, &tim);
			cout<< "\n >>>>>> Begin of LOOP: Inside while(1) select Loop... <<<<<< selectval: "<<selectval<< "\n"; 
			if (selectval){
				cout<< "processTou->RUN: recv ACK"<<endl;
				tm.ptou->run(sockd);
			}else if(selectval == 0){
				cout<< "processTou->SEND: Try to send data left in circular buff..."<<endl;
				tm.ptou->send(sockd);
			}else{
				cerr << "Select timeout: Move tto touClose func!" << endl;
				break;
			}

		} 

		cerr << "While(1) LOOP over, move to touClose function now..." << endl;
		exit(1);
		tm.touClose(sockd);
		/* End of Client Code */
	
	}else if ( argc==2 && !strncmp(argv[1], "-s", 2)){
		std::cout << "Welcome to Sever Mode!\n";
		int sd,bytes_recieved,lis_return;
		int sockd;
		cout << "INSIDE SERVER MODE.. " << endl;

		//CREATE 
		sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
		cout << " Socket function returned : " << sockd << endl;

		//Set socket structures
		memset(&socket1,0,sizeof(socket1));
		socket1.sin_family = AF_INET;
		socket1.sin_addr.s_addr=inet_addr("127.0.0.1");
		socket1.sin_port = htons(1500);
		memset(&socket2,0,sizeof(socket2));

		//BIND
		sd = tm.touBind(sockd,(struct sockaddr*) &socket1,sizeof socket1);
		cout << " Bind Returns : "<< sd << endl;

		//LISTEN
		lis_return = tm.touListen(sockd,1);
		cout << "Ready to accept. Return: "<<lis_return  << endl;

		//Check if it is tou socket
    fd_set socks;
 		struct timeval tim;

		//Initialize processTou
		tm.proTou(sockd);

		socklen_t sinlen = sizeof(socket2);
		tm.ptou->run(sockd);
		sd = tm.touAccept(sockd,(struct sockaddr_in*)&socket2,&sinlen);
		cout << " Accept return, "<< sd <<" , and now for the receiving part " << endl;

		while(1){    
			FD_ZERO(&socks);
			FD_SET(sockd, &socks);
			tim.tv_sec = 5;

			if (select(sockd+1, &socks, NULL, NULL, &tim)){ //SELECT FORM WHAT TO WAHT?????????????????????????????????????????????????
				cout << "Inside Begin of While(1), Select Looop...... " << endl;
				tm.ptou->run(sockd);
				
				memset(recv_data, 0, sizeof(recv_data));
				sd = tm.touRecv(sockd,recv_data,MAXSNDBUF,0);
				cout << "Received Data : touRecv return :" << sd << " siseof recv buf is :"<<sizeof(recv_data) << endl;
			}else{
				cout << "Done with the receiving part, move to touClose \n";
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
