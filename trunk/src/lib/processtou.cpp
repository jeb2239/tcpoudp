#include "tou.h"
boost::mutex recvqmutex;

/* PKTLOSTTEST test */
int pktlosttest_int = 1;

/*
 * processTou
 */
void processTou::run(int sockfd) {
	touMain tm; 
	int			rv;						//bytes recved from other side, (NOT payload size)
	int			rvpl;					//byte recved form other side, payload size
	int			rvplq;				//byte stored in HpRecvBuf, payload size
	int			lenofcb;			//size of circular buffer
	size_t	len = sizeof(sockaddrs);//sockaddr's size
	touPkg	pkgack;				//packet for sending ACK
	socktb = sm->getSocketTable(sockfd);
	pkgack.clean();

	/*for test */
/*	int ggyy = 0;
	std::vector<sockTb*>::iterator stbiter;
	for(stbiter=SS.begin(); stbiter!=SS.end(); stbiter++) {
		ggyy++;
	}std::cout<< "number of entries: " <<ggyy<<std::endl;
*//* end of for test */
	
	/* Check if there're data needed to be send */
	//processTou::send(sockfd);

  cout << " *** processTou Start Processtou now Waiting for data from client *** "  << endl;
  /* Waiting for incoming data */
	tp->clean();
  rv = recvfrom(sockfd, tp, sizeof(*tp), 0, (struct sockaddr*)&sockaddrs,&len);
	tp->printall();
	cout << "Received from : " << inet_ntoa(sockaddrs.sin_addr) <<  " " << htons(sockaddrs.sin_port) << endl; 


  /*
	 * Connection Control (chinmay plz add here) 
   */
	/* Check for SYN flag: If ON, some action */
  if(tp->t.syn == FLAGON /*&& socktb. condition is not right*/) { //server. 
    cout << "SYN Received "  << endl;
    sm->setSocketTableD(&sockaddrs,sockfd);
    sm->setSocketState(TOUS_SYN_RECEIVED,sockfd);
  }else if(tp->t.fin == FLAGON) {
	/* Check for FIN flang (Close) : */
		cout<<"Closing Connection... " << endl;        
		boost::mutex::scoped_lock lock(socktabmutex1);
		sm->setSocketState(TOUS_CLOSE_WAIT,sockfd);
	/*
	 * End of Connection Control Block
	 */

  /* Check for ACK */
	}else if(tp->t.ack == FLAGON && socktb->sockstate == TOUS_ESTABLISHED && isClient ) {
		std::cout<< "Get a (new ACK) in TOUS_ESTABLISHED state\n";	

		if(!tm1.ck_del_timer(sockfd, 88, tp->t.ack_seq)){//ck if already have an ACK
			tm1.delete_timer(sockfd,88,tp->t.ack_seq);
			socktb->sc->addwnd();
			sm->setTCB(socktb->tc.snd_nxt, tp->t.ack_seq, sockfd); // set up snd_una only
		}else{//duplicate ack (already have one in delqueue)
			socktb->sc->setdwnd();
			std::cout << "duplicate ACK: "<< tp->t.ack_seq << " it's #: "<< socktb->tc.dupackcount<<std::endl;
		}
			
		tp->printall();
		socktb->printall();

		// wnd size goes up, got chance to send another pkt
    //send(sockfd);
	}
	/* End of "Check for ACK */

  /* Check DATA (receiving DATA)
	 * NOTICE: piggyback!?		*/
	if( 0 < (rvpl = (strlen(tp->buf))) && socktb->sockstate == TOUS_ESTABLISHED ) {
		if(rvpl > TOU_MSS) rvpl = TOU_MSS; /* err: it'll read 1464, some dirty bits in the rear */

		/* For test */
		std::cout << "Get DATA in TOUS_ESTABLISHED state\n";
		std::cout << "@@@ DATA GET @@@ Data size: "<<rvpl <<std::endl;

		/* test sent pkt lost */
		if(tp->t.seq == 53715 && pktlosttest_int ){

			std::cout << "\n *** pkg 53715 lost test: no ack once PKTLOSTTEST *********\n";
			std::cout << " *** pkg 53715 lost test: no ack once PKTLOSTTEST *********\n\n";
			pktlosttest_int = 0;
			/* FOR TEST */goto PKTLOSTTEST;
		}

    if( tp->t.seq == socktb->tc.rcv_nxt ){
			if(!socktb->HpRecvBuf.empty()){
				/* HpRecvBuf is not empty, and get the correct seq. Start recovery */
				std::cout<< " >>>>***>>> Inside recovery. match the current tp. solved.  seq#: " <<  socktb->tc.rcv_nxt << "number of elm in HpRecvBuf: " << socktb->HpRecvBuf.size() << "\n";
				sm->setTCBRcv(tp->t.seq + rvpl, sockfd);
				lenofcb = putcircbuf(socktb, sockfd, tp->buf, rvpl);
				std::cerr<< "socktb->HpRecvBuf.top().t.seq) == (socktb->tc.rcv_nxt " << socktb->HpRecvBuf.top().t.seq << " " <<socktb->tc.rcv_nxt <<std::endl;

				while( (socktb->HpRecvBuf.top().t.seq) == (socktb->tc.rcv_nxt) ){// recovery form HpRecvBuf
					/* out of order pkt: looply ck pkt in HpRecvBuf, and try to place this.tp into circ buf */
					std::cout<< " >>>>***>>> Inside matching while seq#: " <<  socktb->HpRecvBuf.top().t.seq << "number of elm in HpRecvBuf: " << socktb->HpRecvBuf.size()<<"\n";
					rvplq = strlen(socktb->HpRecvBuf.top().buf);
					if(rvplq > TOU_MSS) rvplq = TOU_MSS;
					if(socktb->ckHpRecvBuf(socktb->HpRecvBuf.top())){
						/* dup pkt in heap, just pop it. so do nothing */
					}else{
						/* not dup, let this pkt in file */
						sm->setTCBRcv( socktb->HpRecvBuf.top().t.seq + rvplq, sockfd); /* set up rcv_nxt */
						char tempBuf[rvplq];
						strncpy(tempBuf, socktb->HpRecvBuf.top().buf, rvplq);
						lenofcb = putcircbuf(socktb, sockfd, tempBuf, rvplq);
					}

					/* this top pkt in q is back in file, pop it */
					socktb->HpRecvBuf.pop();
				}
				recovery = true;
			}

      if( recovery == false ){
				/* in-order pkt: expecting rcv_nxt number */
				sm->setTCBRcv(tp->t.seq + rvpl, sockfd); /* set up rcv_nxt */
				/* Try to put data into circular buff */
				lenofcb = putcircbuf(socktb, sockfd, tp->buf, rvpl);
				std::cout<< " >>>>***>>> Seqnumber matched \n";
			}
			recovery = false;

		}else if( tp->t.seq < socktb->tc.rcv_nxt ){
			/* don't care */
			std::cout<< "\n >>>>***>>> tp->t.seq < socktb->tc.rcv_nxt DONT CARE" <<  tp->t.seq  << "number of elm in HpRecvBuf: " << socktb->HpRecvBuf.size() << "\n";

		}else{
			socktb->HpRecvBuf.push(*tp);
			std::cout<< "\n >>>>***>>> Inside push into the HpRecvBuf: " <<  tp->t.seq  << "number of elm in HpRecvBuf: " << socktb->HpRecvBuf.size() << "\n";
		}/* End of  if( tp->t.seq == socktb->tc.rcv_nxt ) */

		if (sndack){
	   	/* sendback the ACK to sender */
	    assignaddr((struct sockaddr_in *)&sockaddrs, AF_INET, socktb->dip, socktb->dport); //should ck success or not
	    pkgack.putHeaderSeq(socktb->tc.snd_nxt, socktb->tc.rcv_nxt);
	    pkgack.t.ack = FLAGON;

	  	pkgack.printall();
	  	std::cout << " >>> [SEND ACK] data sent <<< " <<std::endl;
	  	sendto(sockfd, &pkgack, sizeof(pkgack), 0, (struct sockaddr *)&sockaddrs, sizeof(sockaddrs));
	  	socktb->printall();
		}
		
  }/* End of Checking for incoming DATA */

PKTLOSTTEST: /* for test */
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

/* push data into circular buf */
int processTou::putcircbuf(sockTb *socktb, int sockfd, char *buf, int rvpl){
	int lenofcb;
	/* Try to put data into circular buff */
	if( rvpl <= (lenofcb = (socktb->CbRecvBuf.getAvSize())) ){
		// insert all of buf into cb
		lenofcb = sm->setCbData(buf, rvpl, sockfd);
	}else{
		// just insert size of available cb
		lenofcb = sm->setCbData(buf, lenofcb, sockfd);
	}
	return lenofcb;
}

/* sending pkt to other end */
void processTou::send(int sockfd) {
  socktb = sm->getSocketTable(sockfd);
  touPkg toupkg;
	toupkg.clean();
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
		cout << " *** Inside: processtou::send number of bytes in the circular buff: "<< socktb->CbSendBuf.getTotalElements() <<endl;
  
  	/* JUST FOR TEST DEMO, Wait a little while */
		//sleep(1);
  
  	/* get the current window size */
		curwnd = socktb->sc->getwnd();
  	sndsize=((socktb->tc.snd_una + curwnd)-socktb->tc.snd_nxt);
		socktb->printall();//TEST

  	cout <<"how much i can snd base on current window size: "<< sndsize<<std::endl;

  	while( (0 < socktb->CbSendBuf.getTotalElements() ) && (0 < ( sndsize=((socktb->tc.snd_una + curwnd)- socktb->tc.snd_nxt)))) {
  	  toupkg.putHeaderSeq(socktb->tc.snd_nxt, socktb->tc.rcv_nxt);
			toupkg.t.ack = FLAGON;

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
  	  sendto(sockfd, &toupkg, sizeof(toupkg), 0, (struct sockaddr *)&sockaddrs, sizeof(sockaddr));
			tm1.add(sockfd, 88, toupkg.t.seq + len, socktb, toupkg.buf);
  	}//end of  while( 0 < ( sndsize=((socktb->tc.snd_una + curwnd)-socktb->tc.snd_nxt)))
		cout << " *** LEAVE: processtou::sendi\n " <<endl;
  }//end of  while( socktb->CbSendBf.getTotalElements() > 0)
}
 
