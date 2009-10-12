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


class toumain {
 	
	public :
	unsigned long test;
	touheader t;
	//int tou_socket(void);
	//int tou_accept();
	//int tou_bind();
	//int tou_listen();
	int tou_send();
	int tou_recv();
	int tou_close();
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	int sd;
	int sd2;
	char buf[50],buf1[50];
	/*
	
	int initsoc(struct sockaddr_in &socket) {
	memset(&socket,0,sizeof(socket));
	socket.sin_family = AF_INET;
	socket.sin_addr.s_addr=htonl(INADDR_ANY);	
	//return socket;
	return 0;	
	}
	
	int r = initsoc(socket1);	
	*/
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
	/*
	?*int tou_accept() {
	int rv;
	rv = recvfrom(sd, buf1, 50, 0, (struct sockaddr *)&socket3, sizeof(struct sockaddr));
	return rv;
	}
	*/
	int tou_accept() {
	int rv, control=0;
	size_t len = sizeof(sockaddr);
	rv = recvfrom(sd, buf, 50, 0, (struct sockaddr *)&socket2, &len);
	while(control)
        

	return rv;
	}//*/
};
	
int main()
{
	toumain tm;
	int sd;
	sd = tm.tou_socket();
	cout<<"Socket"<< sd <<endl;
	memset(&tm.socket1,0,sizeof(tm.socket1));
	tm.socket1.sin_family = AF_INET;
	tm.socket1.sin_addr.s_addr=htonl(INADDR_ANY);
	tm.socket1.sin_port = htons(1500);
	memset(&tm.socket2,0,sizeof(tm.socket2));
	tm.socket2.sin_family = AF_INET;
	tm.socket2.sin_addr.s_addr=htonl(INADDR_ANY);
	sd = tm.tou_bind();
	cout<<"Bind" << sd <<endl;
	unsigned long len;
	len = sizeof tm.t;
	cout << tm.t.syn <<std::endl;
	cout<< len <<endl;
	//sd = tou_connect();
	//cout << sd << endl;
	while(1) {
	sd = tm.tou_accept();
	cout << sd << endl;
	}//sd = tm.tou_listen();
	//cout<<sd<<endl;
	return 0;
}
