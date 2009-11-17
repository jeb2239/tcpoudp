//#include "processtou.h"
#include "tou.h"
boost::mutex recvqmutex;

/*
 * This will handle all the incoming packets and 
 * will check if it is a SYN packet or data packet or FIN packet
 */

void processTou::run(int sockfd) {
	touMain tm; 
	int			rv;						//bytes recved from other side, (NOT payload size)
	int			rvpl;					//byte recved form other side, payload size
	int			lenofcb;			//size of circular buffer
	size_t	len;					//sockaddr's size
	touPkg	pkgack;				//packet for sending ACK
	socktb = sm->getSocketTable(sockfd);

	/*for test */
  sm->s->printall();
	int ggyy = 0;
	std::vector<sockTb*>::iterator stbiter;
	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++) {
		ggyy++;
	}std::cout<< "number of entries: " <<ggyy<<std::endl;
	/* end of for test */
	
	/* Check if there're data needed to be send */
	processTou::send(sockfd);

  cout << "Waiting for data from client "  << endl;
  /* Waiting for incoming data */
  rv = recvfrom(sockfd, &tp, sizeof(tp), 0, (struct sockaddr*)&sockaddrs,&len);
  tm.convertFromByteOrder(tp);

  /*
	 * Following if For Connection Control
   */
	/* Check for SYN flag: If ON, some action */
  if(tp.t.syn == FLAGON) {
    cout << "SYN Received "  << endl;
    sm->setSocketTableD(&sockaddrs,sockfd);
    cout <<"1111:"<<endl;
    sm->setSocketState(TOUS_SYN_RECEIVED,sockfd);
    cout <<"2222:"<<endl;
    sm->setTCB(tm.tp.t.seq, tm.tp.t.seq,sockfd);  
    cout <<"3333:"<<endl;    
  }else if(tp.t.fin == FLAGON) {
	/* Check for FIN flang (Close) : */
		cout<<"Closing Connection... " << endl;        
		boost::mutex::scoped_lock lock(socktabmutex1);
		tm.convertToByteOrder(tm.tp);
		sm.setTCB(tm.tp.t.seq,tm.tp.t.seq, sockfd); 
		sm.setSocketState(TOUS_CLOSE_WAIT,sockfd);
	/*
	 * End of Connection Control Block
	 */

  /* Check for ACK */
	}else if(tp.t.ack == FLAGON && socktb->sockstate == TOUS_ESTABLISHED) {
		std::cout<< "Get a (new ACK) in TOUS_ESTABLISHED state\n";	
    tm1.delete_timer(sockfd,88,tp.t.ack_seq);
    sm.s->sc->addwnd();
		/* set up snd_una only */
		sm.setTCB(socktb->tc.snd_nxt, tp.t.ack_seq, sockfd);
		/* wnd size goes up, got chance to send another pkt */
    send(sockfd);
	}
	/* End of "Check for ACK */

  /* Checking for incoming DATA, (RECV DATA)
	 * NOTICE: piggyback!?		*/
	if( 0 < (rvpl = (strlen(tp.buf))) && socktb->sockstate == TOUS_ESTABLISHED ) {
		/* For test */
		std::cout << "Get DATA in TOUS_ESTABLISHED state\n";
		std::cout << "Data: "<<tp.buf <<std::endl;

		/* set up rcv_nxt */
		sm.setTCBRcv(tp.t.seq + rvpl, sockfd);

		/* Try to put data into circular buff */
		if( rvpl <= (lenofcb = (socktb->CbRecvBuf.getAvSize())) ){
      // insert all of buf into cb
			lenofbc = sm.setCbData(tp.buf, rvpl, sockfd);
		}else{
			// just insert size of available cb
			lenofbc = sm.setCbData(tp.buf, lenofbc, sockfd);
		}

		/* sendback the ACK to sender */
		assignaddr((struct sockaddr_in *)&sockaddrs, AF_INET, socktb->dip, socktb->dport); //should ck success or not
		pkgack.putHeaderSeq(socktb->tc.snd_nxt, socktb->tc.rcv_nxt);
		pkgack.t.ack = FLAGON;

		std::cout << " >>> [SEND ACK] data sent <<< " <<std::endl;
		sendto(sockfd, &pkgack, sizeof(pkgack), 0, (struct sockaddr *)&sockaddrs, sizeof(sockaddrs));
  }/* End of Checking for incoming DATA */

	cout << " *** Leaving process tou *** " << endl;
}/* END of processtou */


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
		//sleep(1);
  
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


 

