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
/*
void addTouHd(char *buf, int n, touheader *th) {
  memset(buf, 0x00, n);
  string a,b,c,d,e,f;
  stringstream outputv,outputv2,outputv3,outputv4,outputv5;
  outputv << th->seq;
  a = outputv.str();
  cout<<"size of seq is "<<sizeof(th->seq)<<endl;
  //a.append(3,'!');
  strncpy(buf,a.c_str(), sizeof(a));
  outputv2 << th->mag;
  b = outputv2.str();
  //b.append(3,'!');
  strncat(buf,b.c_str(), sizeof(a));
  outputv3 << th->ack_seq;
  c = outputv3.str();
  //c.append(3,'!');
  strncat(buf,c.c_str(), sizeof(a));
  //outputv4 << th->flags;
  //cout<<"size of flags is "<<sizeof(th->flags	)<<endl;
  d = outputv4.str();
  //d.append(3,'!');
  strncat(buf,d.c_str(), sizeof(a));
  outputv5 << th->wnd;
  e = outputv5.str();
  //e.append(3,'!');
  strncat(buf,e.c_str(), sizeof(a));
};
*/

//----------------------- MAIN CLASS -------------------

class toumain {
 	
	public :
	unsigned long test;
	touheader t;
	int tou_close();
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	int sd;
	int sd2;
	char buf1[50];
	timerMng timermng;
	
	void converttobyteorder(touheader t) {
	t.seq = htonl(t.seq);
    t.mag = htonl(t.seq);
    t.ack_seq = htonl(t.seq);
    t.syn = htons(t.syn);
    t.ack = htons(t.ack);
    t.ack_seq = htons(t.ack_seq);
    }
	
	void convertfrombyteorder(touheader t) {
	t.seq = ntohl(t.seq);
    t.mag = ntohl(t.seq);
    t.ack_seq = ntohl(t.seq);
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
		
		t.seq = rand()%(u_long)65535;
		t.mag = (u_long)9999;
		converttobyteorder(t);
		t.syn = 1;
		rv = sendto(sd, &t, sizeof(t), 0, (struct sockaddr*)&socket2, 	sizeof(struct sockaddr_in));
		size_t len = sizeof(sockaddr);
		
		//Check if the ayn ack has received
		while(1)
		{
 			fd_set socks;
 			struct timeval t;
 			FD_ZERO(&socks);
 			FD_SET(sd, &socks);
 			t.tv_sec = 4;
 			
 			if (select(sd+1, &socks, NULL, NULL, &t))
			{
 				//recvfrom(sock, data, length, 0, sockfrom, &length);
 				rv = recvfrom(sd, &t, sizeof t, 0, (struct sockaddr *)&socket2,	 &len);
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
		convertfrombyteorder(t);
		perror("talker: sendto");
		cout << "seq no received from server : " << t.seq<<endl;
		cout << "mag no " << t.mag<<endl;
		cout << "ack no received from server : " << t.ack_seq<<endl;
		t.ack_seq = t.seq + 1;
		t.syn = 0;
		t.ack = 1;
		converttobyteorder(t);
		
		//send final 3way handshake
		
		rv = sendto(sd, &t, sizeof(t), 0, (struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		return true;
	}


//--------------------------   SEND ----------------------------

	
	int tou_send(char *buf123,int len1) {
		ssize_t no;	
		char buf12[10];	
		//char buf[len1+1];
		size_t len = sizeof(sockaddr);
		memset(t.buf, 0, TOU_MSS);
		strncpy(t.buf, buf123, len1);
		//cout << "before :" << t.seq << endl;
		//no = sendto(sd, buf, strlen(buf),0,(struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		t.seq = t.seq + no;
		cout << " Sequence no of data sent  : " << t.seq << endl;
		converttobyteorder(t);
		no = sendto(sd, &t, sizeof(t),0,(struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		
		cout << " Data Sent : " << t.buf << endl;
		memset(buf12,0,sizeof buf12);
		sprintf(buf12,"%d",no);
		sendto(sd, buf12, sizeof(buf12),0,(struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
		//t.seq = t.seq + no;
		//cout << "sequence no after : " << t.seq << endl;
		convertfrombyteorder(t);
		int rec = -1;
		rec = recvfrom(sd, &t, sizeof t, 0, (struct sockaddr *)&socket2,&len);
		if(rec == -1) {
			timermng.add(1,3333,4000,101);
			cout << "Timer started " << endl;
		}
		else
		cout <<" received ack is : " << t.ack_seq << endl;
		
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
	len = sizeof tm.t;
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
       close(sockd);
       break;
   	}

	//i++;
	//}
	}
	return 0;
}
