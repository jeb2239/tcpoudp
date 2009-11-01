#include "tou.h"
	
	
int main(int argc, char* argv[])
{
	touHeader t;
	touMain tm;
	int sd,sockd;
	char msg[50];
	char send_data[1024],recv_data[1024];
	memset(msg,0,50);
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
	
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
	//Server socket    
    memset(&socket1, 0, sizeof(socket1));
   	socket1.sin_family = h->h_addrtype;
   	memcpy((char *) &socket1.sin_addr.s_addr,h->h_addr_list[0], h->h_length);
    socket1.sin_port = htons(1500);
	
	//CREATE socket
	 //Client socket
	sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
	
	//Set socket 1
	memset(&socket2,0,sizeof(socket2));
	socket2.sin_family = AF_INET;
	socket2.sin_addr.s_addr=htonl(INADDR_ANY);
	socket2.sin_port = htons(1501);
	
	//BIND
	sd = tm.touBind(sockd,(struct sockaddr*) &socket2,sizeof socket2);
	unsigned long len;
	len = sizeof tm.tp.t;
	
	//CONNECT
	sd = tm.touConnect(sockd,(struct sockaddr_in*)&socket1,sizeof(socket1));
	cout << "Connect returns : "<< sd << endl;
	/*
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

	}*/
	return 0;
}
