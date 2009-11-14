/***************************************************
 * This is header file.
 **************************************************/
#define TOU_MSS				1456

/***************************************************
 * TOU HEADER
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
		char		*buf

		/* default constructor would new a pkg with TOU_MAX */
		touPkg(){ buf = new char(TOU_MAX); }
		/* specified what size you want assign to payload */
		touPkg(int payloadsize) { buf = new char(payloadsize); }
		
		/* recycle the buf */
		~touPkg(){delete buf;}

		int putHeaderSeq(u_long seq=0, u_long ack_seq=0){
			t.seq = htonl(seq);
			t.ack_seq = htonl(ack_seq);
			t.mag = htonl(0x7194B32E);
		}
};
