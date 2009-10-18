#include<stdio.h>
#include <iostream>
using namespace std;
#include "touheader.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>


class toumain {
 	
	public :
	unsigned long test;
	touheader t;
	socktb s;
	int tou_close();
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	int sd;
	int sd2;
	string touhstring;
	char buf[50],buf1[50],buf3[50],buf4[50],buf5[50];
	
	int tou_socket() {
	sd = socket(AF_INET,SOCK_DGRAM,0);
	return sd;
	}
	
	int tou_bind() {
	int rv;
	rv = bind(sd,(struct sockaddr*) &socket1,sizeof socket1);
	return rv;
	}
	
	int tou_listen() { 
	int rv;
	rv = listen(sd,1);
	}
	
	int tou_accept() {
	int rv, control=0;
	size_t len = sizeof(sockaddr);
	rv = recvfrom(sd, &t, sizeof t, 0, (struct sockaddr *)&socket2, &len);
	cout << "seq no received from client : " << t.seq<<endl;
	cout << "mag no received from client : " << t.mag<<endl;
	t.ack_seq = t.seq + 1;
	t.seq = rand()%(u_long)65530;
	rv = sendto(sd, &t, sizeof(t), 0, (struct sockaddr*)&socket2, sizeof(struct sockaddr_in));
	rv = recvfrom(sd, &t, sizeof t, 0, (struct sockaddr *)&socket2, &len);
	cout << "ack no received for the third handshake : " << t.ack_seq<<endl;
	len = sizeof t;
	cout << " tou header size received :  "<< len<<endl; 
	if (t.ack_seq == t.seq + 1) cout<<"SUCCESS !!!" << endl;
	return true;
	}
	
	int tou_recv() {
	size_t len = sizeof(sockaddr);
		int no1 = recvfrom(sd, buf, sizeof buf, 0, (struct sockaddr *)&socket2, &len);
		cout << " Data Received :  " << buf << endl; 
		cout << " no of bytes received : " << no1 <<endl;
		recvfrom(sd, buf1, strlen (buf1), 0, (struct sockaddr *)&socket2, &len);
		cout << buf1 << endl;
		int no = atoi(buf1);
		cout << " No of bytes received is :  " << no << endl;
		recvfrom(sd, &t, sizeof t, 0, (struct sockaddr *)&socket2, &len);
		cout << " Sequence no received is :  " << t.seq << endl;
		return true;
	}	
};
	
int main()
{
	toumain tm;
	touheader t;
	socktb s;
	int sd;
	int sockd;
	sockd = tm.tou_socket();
	//cout<<"Socket"<< sockd <<endl;
	memset(&tm.socket1,0,sizeof(tm.socket1));
	tm.socket1.sin_family = AF_INET;
	tm.socket1.sin_addr.s_addr=htonl(INADDR_ANY);
	tm.socket1.sin_port = htons(1500);
	memset(&tm.socket2,0,sizeof(tm.socket2));
	tm.socket2.sin_family = AF_INET;
	tm.socket2.sin_addr.s_addr=htonl(INADDR_ANY);
	sd = tm.tou_bind();
	//cout<<"Bind" << sd <<endl;
	unsigned long len;
	
	while(1) {
		sd = tm.tou_accept();
		if (sd == 1) {
				s.sockd = sockd;
				s.dport = tm.socket1.sin_port;
				s.sport = tm.socket2.sin_port;
				s.dip = inet_ntoa(tm.socket2.sin_addr);
				s.sip = inet_ntoa(tm.socket1.sin_addr);
				}
	//	cout <<"socket table info : "<<"Sockfd  " << s.sockd<< " ports : "<< ntohs(s.dport)<<" " << ntohs(s.sport) <<" ip : "<< s.sip <<" "<<s.dip << endl ;
	//cout << sd << endl;
	cout << "get the msg from client connect() \n";
	tm.tou_recv();
	}
	return 0;
}
