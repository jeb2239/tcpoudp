/**
 * touHeader.cpp
 * Implementation of ToU header classes. 
 *
 */

#include "touHeader.h"
#include "Logger.h"

/**
 * Create an empty buffer with the size of TOU_MSS.
 */
touPkg::touPkg(){ 
			clearFlag();
			buf = new std::string(TOU_MSS, NULL);
			buflen = TOU_MSS;
		}

		/**
		 * copy contructor
		 */
touPkg::touPkg(const touPkg &toupkt){
			t = toupkt.t;
			buf = new std::string(toupkt.buf->data(), toupkt.buflen);
			buflen = toupkt.buflen;
		}

		/**
		 * constructor
		 * specify the size with user specified number
		 */
touPkg::touPkg(int payloadsize){ 
			clearFlag();
			buf = new std::string(payloadsize, NULL); 
			buflen = payloadsize;
		}

		/**
		 * constructor
		 * convert string to touPkg format
		 */
touPkg::touPkg(std::string content){
			std::string tempdata;

			memcpy(&(t.seq), &(content[0]), 4);
			memcpy(&(t.mag), &(content[4]), 4);
			memcpy(&(t.ack_seq), &(content[8]), 4);

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

			memcpy(&(t.window), &(content[14]), 2);

			buflen = content.size()-16;
			if (buflen > 0)
				buf = new std::string( &(content[16]), buflen);
			else
			  buf = new std::string();
		}

		/**
		 * constructor
		 * specify the size the data
		 */
touPkg::touPkg(const char *str, int payloadsize){
			clearFlag();
			buf = new std::string(str, payloadsize);
			buflen = payloadsize;
}

		
		/**
		 * constructor
		 * convert touPkg format to string
		 */
std::string touPkg::toString(){
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
				content.append(buf->data(), buflen);

			return content;
		}

void touPkg::setBuf( const char *str, int size ){
			buf->clear();
			buf->append(str, size);
		}

int touPkg::putHeaderSeq(u_long seq, u_long ack_seq){
			t.seq = seq;
			t.ack_seq = ack_seq;
			t.mag = 0x7194B32E;
		}

int touPkg::clearFlag(){
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

int touPkg::clean(){
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
			buflen = 0;
		}

/**
 * log():
 * Default logging mechanism. It logs only to TOULOG_PKT
 */
void touPkg::log() {
	log(0);
}

/**
 * log(logflag):
 * Additionaly specify the file that been logged.
 */
void touPkg::log(unsigned short logflag) {
	unsigned short def_logflag = TOULOG_PKT;
	def_logflag = (def_logflag | logflag);

	lg.logData("### Packet ###", def_logflag);
	lg.logData("# SEQ: "+lg.c2s(t.seq), def_logflag);
	lg.logData("# SEQ_ACK: "+lg.c2s(t.ack_seq), def_logflag);
	lg.logData("# RES: "+lg.c2s(t.res)+" CWR: "+lg.c2s(t.cwr)+" ECE: "+
			lg.c2s(t.ece)+" ACK: "+lg.c2s(t.ack), def_logflag);
	lg.logData("# PSH: "+lg.c2s(t.psh)+" RST: "+lg.c2s(t.rst)+" SYN: "+
			lg.c2s(t.syn)+" FIN: "+lg.c2s(t.fin), def_logflag);
	lg.logData("# Size header: "+lg.c2s(sizeof(t)), def_logflag);
	lg.logData("# Size of payload: "+lg.c2s(buf->size()), def_logflag);
}

unsigned long touPkg::getSeq(){
			return (t.seq);
		}

unsigned long touPkg::getAckseq(){
			return (t.ack_seq);
		}

std::string touPkg::getBuf(){
			return *buf;
		}

