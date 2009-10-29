#include "tou.h"

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
	
	//Byte stream functions	
	void convertToByteOrder(touPkg tp) {
	tp.t.seq = htonl(tp.t.seq);
    tp.t.mag = htonl(tp.t.seq);
    tp.t.ack_seq = htonl(tp.t.seq);
    tp.t.syn = htons(tp.t.syn);
    tp.t.ack = htons(tp.t.ack);
    tp.t.ack_seq = htons(tp.t.ack_seq);
    }
	
    
    //------------------------------- CREATE SOCKET ------------------------------
	int touSocket(int domain, int type, int protocol) {
	
		if ((domain != AF_INET) || (type != SOCK_DGRAM) || (protocol != 0 ))
		{
			cout << "ERROR CREATING SOCKET" ;
			return -1 ;
		}
		sd = socket(domain,type,0);
		//setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		return sd;
	}
	int touSocket(int domain, int type, int protocol) {
	
		if ((domain != AF_INET) || (type != SOCK_DGRAM) || (protocol != 0 ))
		{
			cout << "ERROR CREATING SOCKET" ;
			return -1 ;
		}
		sd = socket(domain,type,0);
		setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); //????????????????????????????????????????????????????? why this one not marked, another is marked. they sould be identical
		return sd;
	}
	
	//-----------------------------LISTEN -----------------------------------
	//TODO : Modify this 
	
	int touListen() { 
	int rv;
	rv = listen(sd,1);
	}
	
	//------------------------------ BIND SOCKET ------------------------------------
	
	int touBind(int sockfd, struct sockaddr *my_addr, int addrlen) {
	
		int rv;
		rv = bind(sockfd,my_addr,addrlen);
		return rv;
	}
	
	//	---------------------------- CONNECT -----------------------------------------
	
	int touConnect(int sd, struct sockaddr *socket2, int addrlen) {
		int rv;
		
		//send syn and seq no
		//TODO : change the mod to 2^32 and the magic number also
		
		tp.t.seq = rand()%(u_long)65535;
		tp.t.mag = (u_long)9999;
		convertToByteOrder(tp);
		tp.t.syn = 1;
		rv = sendto(sd, &tp, sizeof(tp), 0,socket2,addrlen);
		size_t len = sizeof(sockaddr);
		cout << endl << " INSIDE TOU CONNECT () " <<endl;
		
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
 				rv = recvfrom(sd, &tp, sizeof tp, 0, socket2,&len);
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
		
		rv = sendto(sd, &tp, sizeof(tp), 0, socket2, addrlen);
		cout << " Sent the third handshake " << endl;
		return true;
	}


	//--------------------------   SEND ----------------------------
	/*
	int touSend(char *buf123,int len1) {
		
		ssize_t no;
		int rec;	
		char buf12[10];	
		char buf[len1+1];
		size_t len = sizeof(sockaddr);
		memset(tp.buf, 0, TOU_MSS);
		memcpy(tp.buf, buf123, len1);
		cout << endl << " INSIDE TOUSEND () " <<endl;
		int act = (sizeof tp.t) + len1;
		int act2 = len1;
		
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
	
	*/
	//-------------------------CLOSE------------------------------
	//TODO
	
	int touClose()
	{
		
		tp.t.fin = 1;
		tp.t.seq = tp.t.seq + 1;
		tp.t.ack_seq = 1;
	}		
	
};