/***************************************************


				TOU CLIENT
				

***************************************************/



#include<stdio.h>
#include<stdlib.h>
#include<iostream>
using namespace std;
#include "touheader.h"
#include "timer.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<sstream>
#include<netdb.h>


//----------------------- MAIN CLASS -------------------

class touMain {
 	
 	// Declarations :
 	// TODO :Need to work on which ones should be private
 	
 	
	public :
	unsigned long test;
	touPkg tp;
	int tou_close();	//TODO : Remove when implemented 
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	
	// Socket File Descriptors
	
	int sd;
	int sd2;
	char buf1[50];
	
	// From timer.h
	timerMng timermng;
	
	//Byte stream functions
	
	void convertToByteOrder(touPkg tp) {
	tp.t.seq = htonl(tp.t.seq);
    tp.t.mag = htonl(tp.t.seq);
    tp.t.ack_seq = htonl(tp.t.seq);
    tp.t.syn = htons(tp.t.syn);
    tp.t.ack = htons(tp.t.ack);
    tp.t.ack_seq = htons(tp.t.ack_seq);
    }
	
	
	void convertFromByteOrder(touPkg tp) {
	tp.t.seq = ntohl(tp.t.seq);
    tp.t.mag = ntohl(tp.t.seq);
    tp.t.ack_seq = ntohl(tp.t.seq);
    }
    
    //------------------------------- CREATE SOCKET ------------------------------
    
	int touSocket() {
	sd = socket(AF_INET,SOCK_DGRAM,0);
	return sd;
	}
	
	//------------------------------ BIND SOCKET ------------------------------------
	
	int touBind() {
		int rv;
		rv = bind(sd,(struct sockaddr*) &socket1,sizeof socket1);
		return rv;
	}
	
	
	//	---------------------------- CONNECT -----------------------------------------
	
	int touConnect() {
		int rv;
		
		//send syn and seq no
		//TODO : change the mod to 2^32 and the magic number also
		
		tp.t.seq = rand()%(u_long)65535;
		tp.t.mag = (u_long)9999;
		convertToByteOrder(tp);
		tp.t.syn = 1;
		rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&socket2, 	sizeof(struct sockaddr_in));
		size_t len = sizeof(sockaddr);
		cout << endl << " INSIDE TOU_CONNECT () " <<endl;
		
		//Check if the ayn ack has received
		
		while(1)
		{
 			fd_set socks;
 			struct timeval tim;
 			FD_ZERO(&socks);
 			FD_SET(sd, &socks);
 			tim.tv_sec = 4;
 			
 			if (select(sd+1, &socks, NULL, NULL, &tim))
			{
 				//recvfrom(sock, data, length, 0, sockfrom, &length);
 				rv = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&socket2,	 &len);
 				cout<<"SYN ACK received : "<< endl;
 				break;
			}
			else
 				cout <<"SYNACK not received !! "<< endl;
		
		}

		convertFromByteOrder(tp);
		perror("talker: sendto");
		cout << "seq no received from server : " << tp.t.seq<<endl;
		cout << "ack no received from server : " << tp.t.ack_seq<<endl;
		
		
		tp.t.ack_seq = tp.t.seq + 1;
		tp.t.syn = 0;
		tp.t.ack = 1;
		convertToByteOrder(tp);
		
		//send final 3way handshake
		
		rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		cout << " Sent the third handshake " << endl;
		return true;
	}


	//--------------------------   SEND ----------------------------

	
	int touSend(char *buf123,int len1) {
		
		ssize_t no;
		int rec;	
		char buf12[10];	
		char buf[len1+1];
		size_t len = sizeof(sockaddr);
		memset(tp.buf, 0, TOU_MSS);
		strncpy(tp.buf, buf123, len1);
		cout << endl << " INSIDE TOUSEND () " <<endl;
		int act = (sizeof tp.t) + (strlen(tp.buf));
		int act2 = strlen(tp.buf);
		
		//Send data and header
		
		tp.t.seq = tp.t.seq + act2;
		cout << " Sequence no of data sent  : " << tp.t.seq << endl;
		convertToByteOrder(tp);
		
		no = sendto(sd, &tp, act,0,(struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		cout << " Data Sent : " << tp.buf << endl;
	
		//Recv ACK
			
		rec = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&socket2,&len);
		convertFromByteOrder(tp);
		cout <<" received ack is : " << tp.t.ack_seq << endl;
		
		return no;
	}
};
	
	
	
	
int main(int argc, char* argv[])
{
	touHeader t;
	touMain tm;
	int sd,sockd;
	char msg[50];
	char send_data[1024],recv_data[1024];
	memset(msg,0,50);
	
	// Message to be sent
	strcpy(msg,argv[2]);
	struct hostent *h;
	
	//Get host name
	h = gethostbyname(argv[1]);

	if(h==NULL) {
      printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    }

    printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name,
    inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));
    
    //Set socket 2
    
    memset(&tm.socket2, 0, sizeof(tm.socket2));
   	tm.socket2.sin_family = h->h_addrtype;
   	memcpy((char *) &tm.socket2.sin_addr.s_addr,
    h->h_addr_list[0], h->h_length);
    tm.socket2.sin_port = htons(1500);
	
	//CREATE socket
	//TODO : socket structure should be defined in the main function  
	sockd = tm.touSocket();
	
	//Set socket 1
	memset(&tm.socket1,0,sizeof(tm.socket1));
	tm.socket1.sin_family = AF_INET;
	tm.socket1.sin_addr.s_addr=htonl(INADDR_ANY);
	tm.socket1.sin_port = htons(1501);
	
	//BIND
	sd = tm.touBind();
	unsigned long len;
	len = sizeof tm.tp.t;
	
	//CONNECT
	sd = tm.touConnect();
	cout << " hi" <<endl; 
	
	
	while(1){
	cout<< "Enter data to be sent : " << endl;
	gets(send_data);
    	if (strcmp(send_data , "q") != 0 && strcmp(send_data , "Q") != 0)
           tm.touSend(send_data,strlen(send_data)); 
		else
  		{
		 	tm.touSend(send_data,strlen(send_data));
   	    	close(sockd);
   	     	break;
   		}

	}
	return 0;
}
