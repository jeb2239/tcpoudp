//#include "processtou.h"
#include "tou.h"
boost::mutex recvqmutex;

/*
 * This will handle all the incoming packets and 
 * will check if it is a SYN packet or data packet or FIN packet
 */

void processTou::run(int sockfd) {
	int ggyy = 0;
	touMain tm;

  sm->s->printall();


	tm.sm.s = sm->getSocketTable(sockfd);
	cout << "tm'sprintall()\n";
	tm.sm.s->printall();
  static int timerid;
  size_t len = sizeof(sockaddr_in);
  
	std::vector<sockTb*>::iterator stbiter;
	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++) {
		ggyy++;
	}
	std::cout<< "number of entries: " <<ggyy<<std::endl;
	
	
  send(sockfd);
	tm.sm.s->printall();

  sockaddr_in sockaddrs ;
  cout << "Waiting for data from client "  << endl;
  int rv = recvfrom(sockfd, &(tm.tp), sizeof (tm.tp), 0, (struct sockaddr*)&sockaddrs,&len);
  tm.convertFromByteOrder(tm.tp);
  /*
   *check for SYN
   */

  if(tm.tp.t.syn == 1) {
    cout << "SYN Received "  << endl;
    tm.sm.setSocketTableD(&sockaddrs,sockfd);
    cout <<"1111:"<<endl;
    tm.sm.setSocketState(TOUS_SYN_RECEIVED,sockfd);
    cout <<"2222:"<<endl;
    tm.sm.setTCB(tm.tp.t.seq, tm.tp.t.seq,sockfd);  
    cout <<"3333:"<<endl;    
  }

  /*
   * Check for ACK
   */
  else if(tm.tp.t.ack == 1) {
  //  tm1.delete_timer(sockfd,2,tm.tp.t.seq);
    cout << "INSIDE ACK MANNNNNNNN  " << endl;
    tm.sm.s->sc->addwnd();
    send(sockfd);
  }
  
  //if( 
	
  /*
   *check for data
   */
  
  else if((strlen(tm.tp.buf)) > 0 && (tm.tp.t.syn!= 1)) {
  cout << " Accepting data .....  " <<tm.tp.buf <<  endl;
  tm.sm.setTCB(tm.tp.t.seq,tm.tp.t.seq,sockfd);                
  boost::mutex::scoped_lock lock(socktabmutex1);        
  int lenofbuf = strlen(tm.tp.buf);
  int lenofcb = tm.sm.s->CbRecvBuf.getSize();
  
  tm.sm.setSocketState(TOUS_CLOSE_WAIT,sockfd);
  if(lenofbuf <= lenofcb) {
    tm.sm.s->CbRecvBuf.insert(tm.tp.buf, lenofbuf);
  }
  else {
    tm.sm.s->CbRecvBuf.insert(tm.tp.buf, lenofcb);
  }
    tm.sm.setCbData(tm.tp.buf,sockfd,lenofbuf);
    cout << " Leaving process tou " << endl;
    
  //write code for ack send
  //
  //sockTb = tm.sm.getSocketTable(sd);
   //struct sockaddr_in sockaddrs,socket1;
  //u_short port = (s->dport);
  //assignaddr(&sockaddrs,AF_INET,sockTb->dip,port);
  //tp.t.ack = 1;
  //sendto(sd,&tp,sizeof(tp),(struct sockaddr *)&sockaddrs,sizeof(sockaddr));
  }

  /*
   *check for close
   */

  else if(tm.tp.t.fin == 1) {
  cout<<"Closing Connection... " << endl;        
  boost::mutex::scoped_lock lock(socktabmutex1);
  tm.convertToByteOrder(tm.tp);
  tm.sm.setTCB(tm.tp.t.seq,tm.tp.t.seq, sockfd); 
  tm.sm.setSocketState(TOUS_CLOSE_WAIT,sockfd);
  
  }


}


/*************************************************
 * sending
 * **********************************************/

boost::mutex sndqmutex;

int processTou::popsndq(sockTb *socktb, char *sendbuf, int len) {
  //boost::mutex::scoped_lock lock(sndqmutex);
  int	bread, end;
  memset(sendbuf, 0, TOU_MSS);

  /* try to get data form circular buffer */
  if( 0 < (bread = socktb->CbSendBuf.getAt(sendbuf, len, end))) {
    socktb->CbSendBuf.remove(bread);
    return bread;
  }
  return 0;
}

/* get the sockaddr_in infomation */
int processTou::assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, unsigned short port){
  bzero(sockaddr, sizeof(*sockaddr));
  sockaddr->sin_family = sa_family;
  sockaddr->sin_port = htons(port);
  if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
    return 0;
  return 1; 
}

/* sending pkt to other end */
void processTou::send(int sockfd) {
  socktb = sm->getSocketTable(sockfd);
  touPkg toupkg;
  int	totalelm;
  int 	len;
  unsigned long	curwnd; //current wnd size
  unsigned long sndsize = 0;
  sockaddr_in sockaddrs ;
  /* set up recv's info */
  assignaddr(&sockaddrs, AF_INET, socktb->dip, socktb->dport);

  //while( 0 < (totalelm = socktb->CbSendBuf.getTotalElements()))
	{
  	// there're some data in the buf need to be send 
		cout << " number of bytes in the circular buff: "<< socktb->CbSendBuf.getTotalElements() <<endl;
  
  	/* JUST FOR TEST DEMO, Wait a little while */
		sleep(1);
  
  	/* get the current window size */
		curwnd = socktb->sc->getwnd();
  	sndsize=((socktb->tc.snd_una + curwnd)-socktb->tc.snd_nxt);

		socktb->printall();//TEST

  	cout <<"how much i can snd base on current window size: "<< sndsize<<std::endl;
  	while( (0 < socktb->CbSendBuf.getTotalElements() ) && (0 < ( sndsize=((socktb->tc.snd_una + curwnd)- socktb->tc.snd_nxt)))) {
  	  toupkg.putHeaderSeq(socktb->tc.snd_nxt, socktb->tc.snd_ack);
  	  if( sndsize >= TOU_SMSS ){
  	    //available size is bigger than MSS, so send size of SMSS at most
				len = popsndq(socktb, toupkg.buf, TOU_SMSS); //sizeof(toupkg.buf) should be the same as TOU_SMS
				cout << "Client send (>= TOU_SMSS)"<<endl;
  	  }else{
  	    //wnd available for sending is less than SMSS
				len = popsndq(socktb, toupkg.buf, sndsize);
				cout << "Client send (< TOU_SMSS)"<<endl;
  	  }

  	  //snd_nxt moves on
  	  socktb->tc.snd_nxt += len;
  
  	  // send
  	  std::cout << " >>> [SEND] data length sent: "<<len<<", and origianl sndsize is "<<sndsize<<std::endl;
  	  sendto(sockfd, &toupkg, sizeof(toupkg), 0, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
			tm1.add(sockfd, 88, toupkg.t.seq, socktb, toupkg.buf);
  	}//end of  while( 0 < ( sndsize=((socktb->tc.snd_una + curwnd)-socktb->tc.snd_nxt)))
  }//end of  while( socktb->CbSendBf.getTotalElements() > 0)
}


 

