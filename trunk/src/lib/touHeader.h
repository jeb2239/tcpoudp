/**
 * touHeader
 * Primitive ToU header definitions.
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * Jan 10, 2010
 */

/* ToU packet's payload size */
#define TOU_MSS				1456
#define TOU_MAX				1456

/* ToU flag definition */
#define FLAGON				1
#define FLAGOFF				0

#include <iostream>
#include <string>
#include <stdio.h>

/**
 * touHeader: 
 * TOU HEADER. The size of the fixed ToU  header is 16 bytes, whereas the size 
 * of fixed TCP header is 20 bytes. The fixed ToU header and UDP header have a 
 * cumulative size of 24 bytes, four more than a fixed TCP header.
 */
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

/**
 * touPkg:
 * ToU application layer: ToU hdr + Data(payload)
 */
class touPkg {
	public:
		touHeader	t;
		std::string	*buf;

		/**
		 * default constructor 
		 * new a pkg with size of TOU_MAX 
		 */
		touPkg(){ 
			clearFlag();
			buf = new std::string(TOU_MSS, ' ');
		}

		/**
		 * copy contructor
		 */
		touPkg(const touPkg &toupkt){
			t = toupkt.t;
			buf = new std::string(toupkt.buf->data());
		}

		/**
		 * constructor
		 * specify the size with user specified number
		 */
		touPkg(int payloadsize){ 
			clearFlag();
			buf = new std::string(payloadsize, ' '); 
		}
		/**
		 * constructor
		 * convert string to touPkg format
		 */
		touPkg(std::string content){
			std::string tempdata;
			tempdata.clear();

			//tempdata = content.substr(0, 4);
			memcpy(&(t.seq), &(content[0]), 4);
			//tempdata = content.substr(4, 4);
			memcpy(&(t.mag), &(content[4]), 4);
			//tempdata = content.substr(8, 4);
			memcpy(&(t.ack_seq), &(content[8]), 4);

			//tempdata = content.substr(13, 1);
			char tempdata_c[1];
			memcpy(tempdata_c, &(content[13]), 1);

			t.res = ( ((tempdata_c[0]) & 0x80) == 0x80 )? 1 : 0;
			t.cwr = ( ((tempdata_c[0]) & 0x40) == 0x40 )? 1 : 0;
			t.ece = ( ((tempdata_c[0]) & 0x20) == 0x20 )? 1 : 0;
			t.ack = ( ((tempdata_c[0]) & 0x10) == 0x10 )? 1 : 0;
			t.psh = ( ((tempdata_c[0]) & 0x08) == 0x08 )? 1 : 0;
			t.rst = ( ((tempdata_c[0]) & 0x04) == 0x04 )? 1 : 0;
			t.syn = ( ((tempdata_c[0]) & 0x02) == 0x02 )? 1 : 0;
			t.fin = ( ((tempdata_c[0]) & 0x01) == 0x01 )? 1 : 0;

			//tempdata = content.substr(14, 2);
			memcpy(&(t.window), &(content[14]), 2);

			if(content.size()-16 > 0){
				std::cerr << "size of content: " << content.size() << std::endl;
				buf = new std::string( &(content[16]), content.size()-16);
			}else{
			  buf = new std::string();
			}

		}
		/**
		 * constructor
		 * specify the size the data
		 */
		touPkg(const char *str, int payloadsize){
			clearFlag();
			buf = new std::string(str, payloadsize);
		}

		/**
		 * destructor
		 */
		~touPkg(){
			//delete buf;
		}

		std::string toString(){
			std::string content;
			char	tempdata[4];
			char	tempdata_b[2];
			char	tempdata_c[1];
			u_short tempdata_doff = 0x0000;
			u_short tempdata_dres = 0x0000;
			u_short tempdata_f = 0x0000;

			memcpy(tempdata, &(t.seq), 4);
			content.append(tempdata, 4);

			memcpy(tempdata, &(t.mag), 4);
			content.append(tempdata, 4);

			memcpy(tempdata, &(t.ack_seq), 4);
			content.append(tempdata, 4);

			tempdata_doff = tempdata_doff | t.doff ;
			tempdata_dres = tempdata_dres | t.dres;
			tempdata_f = (tempdata_doff << 4) | (tempdata_dres) & 0x0011;
			memcpy(tempdata_c, &(tempdata_f), 1);
			content.append(tempdata_c, 1);

			tempdata_c[0] = 0x00;
			if(t.res == 1) tempdata_c[0] = tempdata_c[0] | 0x80;
			if(t.cwr == 1) tempdata_c[0] = tempdata_c[0] | 0x40;
			if(t.ece == 1) tempdata_c[0] = tempdata_c[0] | 0x20;
			if(t.ack == 1) tempdata_c[0] = tempdata_c[0] | 0x10;
			if(t.psh == 1) tempdata_c[0] = tempdata_c[0] | 0x08;
			if(t.rst == 1) tempdata_c[0] = tempdata_c[0] | 0x04;
			if(t.syn == 1) tempdata_c[0] = tempdata_c[0] | 0x02;
			if(t.fin == 1) tempdata_c[0] = tempdata_c[0] | 0x01;
			content.append(tempdata_c, 1);

			memcpy(tempdata_b, &(t.window), 2);
			content.append(tempdata_b, 2);

			if(buf->size() > 0)
				content.append(*buf);

			return content;
		}

		void setBuf( const char *str, int size ){
			buf->clear();
			buf->append(str, size);
		}

		int putHeaderSeq(u_long seq=0, u_long ack_seq=0){
			t.seq = seq;
			t.ack_seq = ack_seq;
			t.mag = 0x7194B32E;
		}

		int clearFlag(){
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
			buf->clear();
		}

		void printall(){/* test function */
			std::cout<< "### Packet ###\n";
			std::cout<< "SEQ: "<< (t.seq) << " SEQ_ACK: "<< (t.ack_seq) << " WINDOW: "<<t.window << std::endl;
			std::cout<< "RES: "<< t.res << " CWR: "<< t.cwr << " ECE: "<< t.ece << " ACK: "<< t.ack;
			std::cout<< " PSH: "<< t.psh << " RST: "<< t.rst << " SYN: "<< t.syn << " FIN: "<< t.fin << std::endl;
			std::cout<< "Size header: "<< sizeof(t) <<std::endl;
			std::cout<< "Size of payload: "<< buf->size() << std::endl;
			//std::cout<< *buf << std::endl;
		}

		unsigned long getSeq(){
			return (t.seq);
		}

		unsigned long getAckseq(){
			return (t.ack_seq);
		}

		std::string getBuf(){
			return *buf;
		}
};

class parsingPacket {
	public:
	void parseToString(std::string *str, touPkg *pt, int lengthofData);
	void parseToPacket(touPkg *pt, std::string *str, int lengthofData);

};
