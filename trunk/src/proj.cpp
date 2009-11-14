#include "tou.h"
#include "processtou.h"	
	

int main(int argc, char* argv[])
{
	std::cout << " Welcom to ToU \n" ;
/* ./tou <svr ip> -c  */
if(argc ==3 && !strncmp( argv[2], "-c", 2))
{
cout << "Client mode.. " << endl;
//touHeader t;
touMain tm;
int sd,sockd;
char send_data[1024],recv_data[1024];
//sockMng sm;
char buf[1024];
int bytes_recieved; 
struct sockaddr_in socket1;
struct sockaddr_in socket2;
struct sockaddr_in socket3;
struct hostent *h;
	//Get host name
	h = gethostbyname(argv[1]);

	if(h==NULL) {
      printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    }

    printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name,
    inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));
    
   //Set socket 1
	//Server socket    
    memset(&socket1, 0, sizeof(socket1));
   	socket1.sin_family = h->h_addrtype;
   	memcpy((char *) &socket1.sin_addr.s_addr,h->h_addr_list[0], h->h_length);
    socket1.sin_port = htons(1500);
	
	//CREATE socket
  //Client socket
	sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
	
	//Set socket 2
	memset(&socket2,0,sizeof(socket2));
	socket2.sin_family = AF_INET;
	socket2.sin_addr.s_addr=inet_addr("127.0.0.1");
	socket2.sin_port = htons(1501);
	
	//BIND
	sd = tm.touBind(sockd,(struct sockaddr*) &socket2,sizeof socket2);
	unsigned long len;
	//len = sizeof tm.tp.t;
	
	//CONNECT
	sd = tm.touConnect(sockd,(struct sockaddr_in*)&socket1,sizeof(socket1));
	cout << "Connect returns : "<< sd << endl;
	///*
//	while(1)
  {
	cout<< "Enter data to be sent : " << endl;
	gets(send_data);
  //cout << " Data acquired now sending ...  "<<send_data <<"   "<< strlen(send_data) << endl;
    	if (strcmp(send_data , "q") != 0 && strcmp(send_data , "Q") != 0)
           tm.touSend(sockd,send_data,strlen(send_data),0); 
		else
  		{
		 	tm.touSend(sd,send_data,strlen(send_data),0);
   	    	close(sockd);
  // 	     	break;
   		}
  //cout << "done with the sending wait for close " << endl;
	} 
 	tm.touClose(sockd);
	
  //*/
	return 0;
}else if ( argc==2 && !strncmp(argv[1], "-s", 2))
{
	std::cout << "Welcome to Sever Mode!\n";
  touMain tm;
	//touHeader t;
	//sockTb s1;
	//sockMng sm;
	int sd,bytes_recieved;
	int sockd;
	char buf[1024];
	struct sockaddr_in socket1;
	struct sockaddr_in socket2;
	struct sockaddr_in socket3;
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
  tm.touListen(sockd,1);
	unsigned long len;
	cout << "Ready to accept " << endl;
	//while(1) 

  /*
  * Check if it is tou socket
  */
    fd_set socks;
 		struct timeval tim;
 		FD_ZERO(&socks);
 		FD_SET(sockd, &socks);
 		tim.tv_sec = 4;
  //  while(1)
  {    
  		
    //if (select(sockd+1, &socks, NULL, NULL, &tim))
    {
    //TODO : Write logic for checking tou sockd here
      cout << "Inside ...... " << endl;
      processTou *p = new processTou(sockd);
    }
  }      
     //processTou *p = new processTou(sockd);
		socklen_t sinlen = sizeof(socket2);
		sd = tm.touAccept(sockd,(struct sockaddr_in*)&socket2,&sinlen);
	
   cout << " And now for the receiving part " << endl;
 // while(1)
  {    
  if (select(sockd+1, &socks, NULL, NULL, &tim))
    {
    //TODO : Write logic for checking tou sockd here
      cout << "Inside 2 ...... " << endl;
      processTou *p = new processTou(sockd);
    }
   }      
   // processTou *p = new processTou(sockd);
		tm.touRecv(sockd,buf,100,0);
   // cout << "Received Data : " << buf << endl;
//    cout << "done with the receiving part " << endl;
	
   //if (select(sockd+1, &socks, NULL, NULL, &tim))
    {
    //TODO : Write logic for checking tou sockd here
  //    cout << "Inside 3...... " << endl;
      processTou *p = new processTou(sockd);
    }      
   //processTou *p = new processTou(sockd);
 	 tm.touClose(sockd);
	
	return 0; 

  }}
