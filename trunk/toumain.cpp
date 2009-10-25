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

class toumain {
 	
	public :
	unsigned long test;
	toupkg tp;
	//touheader t;
	int tou_close();
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	int sd;
	int sd2;
	char buf1[50];
	timerMng timermng;
	
	void converttobyteorder(toupkg tp) {
	tp.t.seq = htonl(tp.t.seq);
    tp.t.mag = htonl(tp.t.seq);
    tp.t.ack_seq = htonl(tp.t.seq);
    tp.t.syn = htons(tp.t.syn);
    tp.t.ack = htons(tp.t.ack);
    tp.t.ack_seq = htons(tp.t.ack_seq);
    }
	
	void convertfrombyteorder(toupkg tp) {
	tp.t.seq = ntohl(tp.t.seq);
    tp.t.mag = ntohl(tp.t.seq);
    tp.t.ack_seq = ntohl(tp.t.seq);
    }
    
	int tou_socket() {
	sd = socket(AF_INET,SOCK_DGRAM,0);
	return sd;
	}
	
	int tou_bind() {
		int rv;
		rv = bind(sd,(struct sockaddr*) &socket1,sizeof socket1);
		return rv;
	}
	
	
	//	---------		CONNECT   --------------
	
	int tou_connect() {
		int rv;
		
		//send syn and seq no
		
		tp.t.seq = rand()%(u_long)65535;
		tp.t.mag = (u_long)9999;
		converttobyteorder(tp);
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

		
		
		
		/*while(!(rv>0))  {
			cout << "SYN ACK not received : " << endl;
			sleep(4);	
		}*/
		convertfrombyteorder(tp);
		perror("talker: sendto");
		cout << "seq no received from server : " << tp.t.seq<<endl;
		//cout << "mag no " << tp.t.mag<<endl;
		cout << "ack no received from server : " << tp.t.ack_seq<<endl;
		tp.t.ack_seq = tp.t.seq + 1;
		tp.t.syn = 0;
		tp.t.ack = 1;
		converttobyteorder(tp);
		
		//send final 3way handshake
		
		rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		cout << " Sent the third handshake " << endl;
		return true;
	}


//--------------------------   SEND ----------------------------

	
	int tou_send(char *buf123,int len1) {
		ssize_t no;	
		char buf12[10];	
		char buf[len1+1];
		size_t len = sizeof(sockaddr);
		memset(tp.buf, 0, TOU_MSS);
		strncpy(tp.buf, buf123, len1);
		cout << endl << " INSIDE TOU_SEND () " <<endl;
		int act = (sizeof tp.t) + (strlen(tp.buf));
		//cout << "before :" << tp.t.seq << endl;
		//no = sendto(sd, buf, strlen(buf),0,(struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		tp.t.seq = tp.t.seq + no;
		cout << " Sequence no of data sent  : " << tp.t.seq << endl;
		
		converttobyteorder(tp);
		no = sendto(sd, &tp, act,0,(struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		
		cout << " Data Sent : " << tp.buf << endl;
		memset(buf12,0,sizeof buf12);
		sprintf(buf12,"%d",no);
		sendto(sd, buf12, sizeof(buf12),0,(struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		//tp.t.seq = tp.t.seq + no;
		//cout << "sequence no after : " << tp.t.seq << endl;
		convertfrombyteorder(tp);
		int rec = -1;
		rec = recvfrom(sd, &tp, sizeof tp, 0, (struct sockaddr *)&socket2,&len);
		if(rec == -1) {
			timermng.add(1,3333,4000,101);
			cout << "Timer started " << endl;
		}
		else
		cout <<" received ack is : " << tp.t.ack_seq << endl;
		
		return true;
	}
};
	
int main(int argc, char* argv[])
{
	touheader t;
	toumain tm;
	int sd,sockd;
	char msg[50];
	char send_data[1024],recv_data[1024];
	memset(msg,0,50);
	strcpy(msg,argv[2]);
	struct hostent *h;
	h = gethostbyname(argv[1]);

	if(h==NULL) {
      printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    }

    printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name,
    inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));
    memset(&tm.socket2, 0, sizeof(tm.socket2));
   	tm.socket2.sin_family = h->h_addrtype;
   	memcpy((char *) &tm.socket2.sin_addr.s_addr,
    h->h_addr_list[0], h->h_length);
    tm.socket2.sin_port = htons(1500);
	sockd = tm.tou_socket();
	memset(&tm.socket1,0,sizeof(tm.socket1));
	tm.socket1.sin_family = AF_INET;
	tm.socket1.sin_addr.s_addr=htonl(INADDR_ANY);
	tm.socket1.sin_port = htons(1501);
	sd = tm.tou_bind();
	unsigned long len;
	len = sizeof tm.tp.t;
	//cout<< "Length of header  : " << len <<endl;
	sd = tm.tou_connect();
	cout << " hi" <<endl; 
	string s1 = inet_ntoa(tm.socket2.sin_addr);
	string s2 = inet_ntoa(tm.socket1.sin_addr);
	//for (int i = 0; i< 5; i++) {
	while(1){
	cout<< "Enter data to be sent : " << endl;
	gets(send_data);
    if (strcmp(send_data , "q") != 0 && strcmp(send_data , "Q") != 0)
           tm.tou_send(send_data,strlen(send_data)); 
	else
    {
		tm.tou_send(send_data,strlen(send_data));
        close(sockd);
        break;
   	}

	//i++;
	//}
	}
	return 0;
}
