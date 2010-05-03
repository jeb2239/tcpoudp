//#include "closer.h"
#include "timer.h"
typedef unsigned int conn_id;
typedef unsigned long seq_id;
typedef unsigned int close_id;
/*
extern boost::mutex closermutex;
void closer::dothis() {
    	fd_set socks;
      fd_set socks1;
      FD_ZERO(&socks);
      FD_SET(fdc,&socks);
      FD_ZERO(&socks1);
      FD_SET(fds,&socks1);
      struct timeval tim;
      tim.tv_sec = 1;
  while(1){
    if(select(fdc+1,&socks, NULL, NULL, &tim) ) {
      boost::mutex::scoped_lock lock(closermutex);
      cout << "inside close client ... " << endl;
      read(fdc,sd,sizeof(sd));
      closer::closeClient(sd);
    }
    
    if(select(fds+1,&socks1, NULL, NULL, &tim) ) {
      boost::mutex::scoped_lock lock(closermutex);
      cout << " Inside close server ... " << endl;
      read(fds,sd,sizeof(sd));
      closer::closeServer(sd);
    }
  }
}

closer::go() {
     cout << "inside go " << endl;
     assert(!m_thread);
        m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&closer::dothis, this)));
}

closer::clientClose(int sd) {
sockTb *s;
sockaddr_in sockaddrs,socket1;
s = sm.getSocketTable(sd);
size_t len = sizeof(sockaddr);
tp.t.seq = s->tc.snd_nxt;
u_short port = (s->dport);
assignaddr(&sockaddrs,AF_INET,s->dip,port);
tp.t.fin = 1;
tp.t.ack_seq = tp.t.seq + 1;
cout << "sending fin " << endl;
int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
sm.setSocketState(TOUS_FIN_WAIT_1,sd);
cout << " Receiving FIn ACK " << endl;
recvfrom(sd,&tp,sizeof(tp),0,(struct sockaddr *)&socket1,&len);
if(tp.t.fin ==1 && tp.t.ack ==1) {
  sm.setSocketState(TOUS_TIME_WAIT,sd);
  }
tp.t.ack == 1;
cout << " Sending ACK " << endl;
sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in));
sm.delSocketTable(sd);
close(sd);  
}

closer::serverClose(int sd) {
sockTb *s;
sockaddr_in sockaddrs,socket1;
s = sm.getSocketTable(sd);
size_t len = sizeof(sockaddr);
tp.t.seq = s->tc.snd_nxt;
u_short port = (s->dport);
assignaddr(&sockaddrs,AF_INET,s->dip,port);
if(s->sockstate == TOUS_CLOSE_WAIT) {
  cout << "Sending FIN ACK " << endl;
  tp.t.fin = 1;
  tp.t.ack = 1; 
  int rv = sendto(sd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs , sizeof(struct sockaddr_in)); 

  fd_set socks;
 	struct timeval tim;
 	FD_ZERO(&socks);
 	FD_SET(sd, &socks);
 	tim.tv_sec = 30; 
  size_t  len = sizeof(sockaddrs);
  while(1){    
			if (select(sd+1, &socks, NULL, NULL, &tim)) {
        cout << " Waiting 2 MSL " << endl ;
        recvfrom(sd,&tp, sizeof(tp),0,(struct sockaddr*)&sockaddrs , &len);
        if(tp.t.ack == 1)
         {
          close(sd);
          return 1;
         }
       }
    else {
        cout << " 2 MSL Over "  << endl;
         close(sd);
          return 1;
      }
    }
  }
}

*/
