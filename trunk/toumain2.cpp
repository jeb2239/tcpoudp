/***************************************************


				TOU SERVER
				

***************************************************/

#include<stdio.h>
#include <iostream>
using namespace std;
#include "touheader.h"
#include "timer.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

//-------------------  TOUMAIN CLASS ---------------
boost::mutex soctabmutex;
class touMain {
 	
	public :
	unsigned long test;
	touPkg tp;
	//touheader t;
	sockTb s;
	
	//TODO : Remove when implemented
	int tou_close(); 
	
	// Define structures
	/*struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	*/
	// Socket Descriptors
	int sd;
	int sd2;
	int yes;
	string touhstring;
	char buf[50],buf1[50],buf3[50],buf4[50],buf5[50];
	timerMng timermng;
	
	// Byte order conversions
	
	void convertToByteOrder(touPkg tp) {
	tp.t.seq = htonl(tp.t.seq);
    tp.t.mag = htonl(tp.t.mag);
    tp.t.ack_seq = htonl(tp.t.ack_seq);
    tp.t.syn = htons(tp.t.syn);
    tp.t.ack = htons(tp.t.ack);
    
    }
	
	void convertFromByteOrder(touPkg tp) {
	tp.t.seq = ntohl(tp.t.seq);
    tp.t.mag = ntohl(tp.t.seq);
    tp.t.ack_seq = ntohl(tp.t.seq);
    }
    
    
    // ------------------------------CREATE ----------------------------------
    
	int touSocket(int domain, int type, int protocol) {
	
		if ((domain != AF_INET) || (type != SOCK_DGRAM) || (protocol != 0 ))
		{
			cout << "ERROR CREATING SOCKET" ;
			return -1 ;
		}
		sd = socket(domain,type,0);
		setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		return sd;
	
	}
	
	//----------------------------BIND ---------------------------------------
	
	int touBind(int sockfd, struct sockaddr *my_addr, int addrlen) {
	
		int rv;
		rv = bind(sockfd,my_addr,addrlen);
		return rv;
	}
	
	//-----------------------------LISTEN -----------------------------------
	//TODO : Modify this 
	
	int touListen() { 
	int rv;
	rv = listen(sd,1);
	}
	
	
	//---------------------------ACCEPT --------------------- 
	
	int touAccept(int sd, struct sockaddr *socket2, socklen_t *addrlen) {
		int rv, control=0, flagforsyn = 1;
		size_t len = sizeof(sockaddr);
		convertToByteOrder(tp);
		cout << endl << " INSIDE TOUACCEPT () " <<endl;
		
		// receive first handshake
		
		rv = recvfrom(sd, &tp, sizeof tp, 0,socket2, &len);
		cout << "seq no received from client : " << tp.t.seq<<endl;
		
		//send syn ack
		
		tp.t.ack_seq = tp.t.seq + 1;
		cout << "ack no sent to client : " << tp.t.ack_seq<<endl;
		tp.t.seq = rand()%(u_long)65530;
		tp.t.syn = 1;
		tp.t.ack = 1;
		convertToByteOrder(tp);
		
		//Code for testing begin
		//	sleep(3);
		
				rv = sendto(sd, &tp, sizeof(tp), 0, socket2, sizeof(struct sockaddr_in));
		
		//Code for testing end
		
		//recv third handshake
		
		rv = recvfrom(sd, &tp, sizeof tp, 0, socket2, &len);
		convertFromByteOrder(tp);
		cout << "ack no received for the third handshake : " << tp.t.ack_seq<<endl;
		cout << endl << " LEAVING TOUACCEPT () " <<endl;
		
		if (tp.t.ack_seq == tp.t.seq + 1) cout<<"SUCCESS !!!" << endl;
		return true;
	}
	
	
	//-----------------	RECEIVE  --------------------------
	/*
	int touRecv() {
	
	memset(tp.buf,0,50);
	char buf12[10];
	int no1;
	memset(buf12,0,10);
	size_t len = sizeof(sockaddr);
	cout << endl << " INSIDE TOURECV () " <<endl;
		
		//timermng.add(1,3333,4000,101);
		
		//Recv data
		
		no1 = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&socket2, &len);
		cout << " Data Received :  " << tp.buf << endl; 
		cout << " no of bytes received : " << no1 <<endl;
		
		convertFromByteOrder(tp);
		cout << " Sequence no received is :  " << tp.t.seq << endl;
		
		//TODO : Put condition to check the data here
		
		//Send ACK
		{
			tp.t.ack_seq = tp.t.seq + 1;
			convertToByteOrder(tp);
			sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
			cout << " Ack sent " <<endl; 
		}
		return no1;
	}
	*/	
};
	
int main()
{
	touMain tm;
	touHeader t;
	sockTb s;
	int sd,bytes_recieved;
	int sockd;
	tm.yes = 1;
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	
	//CREATE 
	sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
	cout << " Socket function returned : " << sockd << endl;
	//Set socket structures
	memset(&socket1,0,sizeof(socket1));
	socket1.sin_family = AF_INET;
	socket1.sin_addr.s_addr=htonl(INADDR_ANY);
	socket1.sin_port = htons(1500);
	memset(&socket2,0,sizeof(socket2));
	socket2.sin_family = AF_INET;
	socket2.sin_addr.s_addr=htonl(INADDR_ANY);
	
	//BIND
	sd = tm.touBind(sockd,(struct sockaddr*) &socket1,sizeof socket1);
	cout << " Bind Returns : "<< sd << endl;
	
	unsigned long len;
	
	while(1) {
		socklen_t sinlen = sizeof(socket2);
		sd = tm.touAccept(sockd,(struct sockaddr*)&socket2,&sinlen);
		if (sd == 1) {
				cout << "SUCCESS !! " << endl;
				}
	/*	
	//SOCKET TABLE INFO 
			boost::mutex::scoped_lock lock(soctabmutex);
			s.sockd = sockd;
			s.dport = tm.socket1.sin_port;
			s.sport = tm.socket2.sin_port;
			s.dip = inet_ntoa(tm.socket2.sin_addr);
			s.sip = inet_ntoa(tm.socket1.sin_addr);
			
				
			cout <<"socket table info : "<<endl;
			cout <<"Sockfd : " << s.sockd<< endl;
			cout <<"-------------------"<<endl;
			cout <<"ports : "<< ntohs(s.dport) <<" " << ntohs(s.sport) <<endl;
			cout <<" ip : "<< s.sip <<" "<<s.dip << endl ;
			}
			while(1) 
			{
			cout << "get the msg from client connect() \n";	
			tm.touRecv();
			  fflush(stdout);

			}
	*/
	}
	
	return 0;
}
