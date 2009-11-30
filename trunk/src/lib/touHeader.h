/***************************************************
 * This is header file.
 **************************************************/
#define TOU_MSS				1456
#define TOU_MAX				1456
#define FLAGON				1
#define FLAGOFF				0

#include <iostream>


/***************************************************
 *8 TOU HEADER
 * The size of the fixed ToU  header is 16 bytes, 
 * whereas the size of fixed TCP header is 20 bytes.
 * The fixed ToU header and UDP header have a cumulative 
 * size of 24 bytes, four more than a fixed TCP header.
 **************************************************/ 
class touHeader {
  public:
	u_long	seq;
	u_long	mag;
	u_long	ack_seq;
	
	u_short	doff:4;
	u_short	dres:4;
	u_short	res:1;
	u_short	cwr:1;
	u_short	ece:1;
	u_short	ack:1;
	u_short	psh:1;
	u_short	rst:1;
	u_short	syn:1;
	u_short	fin:1;

	u_short	window;
};

/******************************************************
 * ToU application layer: ToU hdr + Data(payload)
 * ***************************************************/
class touPkg {
	public:
		touHeader	t;
		char			buf[TOU_MSS];

		touPkg(){}
		/* default constructor would new a pkg with TOU_MAX */
		/*touPkg(){ buf = new char[TOU_MAX]; }
		 specified what size you want assign to payload */
		//touPkg(int payloadsize) { buf = new char[payloadsize]; }
		
		/* recycle the buf */
		~touPkg(){//delete buf;
		}

		int putHeaderSeq(u_long seq=0, u_long ack_seq=0){
			/*
			t.seq = htonl(seq);
			t.ack_seq = htonl(ack_seq);
			t.mag = htonl(0x7194B32E);
		  */	
			t.seq = seq;
			t.ack_seq = ack_seq;
			t.mag = 0x7194B32E;
		}

		int clean(){
			t.seq = 0;
			t.mag = 0x7194B32E;
			t.ack_seq = 0;
			t.doff = 0;
			t.dres = 0;
			t.res = 0;
			t.cwr = 0;
			t.ece = 0;
			t.ack = 0;
			t.psh = 0;
			t.rst = 0;
			t.syn = 0;
			t.fin = 0;
			t.window = 0;
			memset(buf, 0, TOU_MSS);
		}

		void printall(){
			std::cout<< "### Packet ###\n";
			std::cout<< "SEQ: "<< (t.seq) << " SEQ_ACK: "<< (t.ack_seq) <<std::endl;
			std::cout<< "SYN: "<< t.syn << " ACK: "<< t.ack << " FIN: "<< t.fin <<std::endl;
		}

		unsigned long getSeq(){
			return (t.seq);
		}

		unsigned long getAckseq(){
			return (t.ack_seq);
		}
};
