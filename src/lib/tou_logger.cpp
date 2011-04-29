/******************************************************************************
 * tou_logger.cpp
 * logging
 *
 * Copyright 2009 by Columbia University; all rights reserved
 * March 01, 2010
 * ***************************************************************************/


#include "tou_logger.h"
#include "timestamp.h"

Logger 			lg;
unsigned short	LOGFLAG;
bool			LOGON;

using namespace std;
Logger::Logger()
{
	/* 
	 * initiate the beginning time of time stamp
	 */
	set_beg_timestamp();

	/*
	 * initiate vars
	 */
	pb_cwnd = pb_awnd = 0;
	pb_ccstat = 0;
	pb_snd_nxt = pb_ssthresh = 0;
	pb_time = 0.0;

	file.open("./log/Tou.log");
	file_timer.open("./log/Tou_timer.log");
	file_pktsent.open("./log/Tou_pktsent.log");
	file_pktrecv.open("./log/Tou_pktrecv.log");
	file_pkt.open("./log/Tou_pkt.log");
	file_socktb.open("./log/Tou_socktb.log");
	file_probe.open("./log/Tou_probe.log");

	file.flush();
	file_timer.flush();
	file_pktsent.flush();
	file_pktrecv.flush();
	file_pkt.flush();
	file_socktb.flush();
	file_probe.flush();
}

Logger::~Logger(){
	shutdownLogger();
}

void Logger::shutdownLogger()
{
	file.close();	
	file_timer.close();
	file_pktsent.close();
	file_pktrecv.close();
	file_pkt.close();
	file_socktb.close();
	file_probe.close();
}

void Logger::logFile(string data){
	file << setprecision(6) << timestamp_sec() << " " <<  data << endl;
	file.flush();
}
void Logger::logFileTimer(string data){
	file_timer << setprecision(6) << timestamp_sec() << " " << data << endl;
	file_timer.flush();
}
void Logger::logFilePktsent(string data){
	file_pktsent << data << endl;
	file_pktsent.flush();
}
void Logger::logFilePktrecv(string data){
	file_pktrecv << data << endl; 
	file_pktrecv.flush();
}
void Logger::logFilePkt(string data){
	file_pkt << data << endl;
	file_pkt.flush();
}
void Logger::logFileSocktb(string data){
	file_socktb << data << endl;
	file_socktb.flush();
}
void Logger::logFileProbe(int cwnd, unsigned long ssthresh, int awnd, 
						  unsigned long snd_nxt, short ccstat){
	if (ssthresh == 2147483647) ssthresh = 1456; /* for demo */

	file_probe << setprecision(9) << timestamp_sec() << "," << cwnd << "," 
		<< ssthresh << ","<< (int)ssthresh/1456 << "," << awnd << "," << snd_nxt << ","
		<< ccstat << endl;
	//file_probe.flush();
}
void Logger::logFileProbe(double time, int cwnd, unsigned long ssthresh, int awnd, 
						  unsigned long snd_nxt, short ccstat){
	if (ssthresh == 2147483647) ssthresh = 1456; /* for demo */

	file_probe << setprecision(9) << time << "," << cwnd << "," << ssthresh << "," 
		<< (int)ssthresh/1456 << ","<< awnd << "," << snd_nxt << "," << ccstat << endl;
	//file_probe.flush();
}

void Logger::logData(string data, unsigned short LoggingLevel){
	if (LOGON == false)  return;
	unsigned short logs =  (LOGFLAG | LoggingLevel);
	
	while( logs != 0x0000 ){
		if ((logs & TOULOG_ALL) == TOULOG_ALL){
				// log all
				logFile(data);
				logs = logs & ~TOULOG_ALL;
		}else if ((logs & TOULOG_TIMER) == TOULOG_TIMER){
				logFileTimer(data);
				logs = logs & ~TOULOG_TIMER;
		}else if ((logs & TOULOG_PKTSENT) == TOULOG_PKTSENT){
				logFilePktsent(data);
				logs = logs & ~TOULOG_PKTSENT;
		}else if ((logs & TOULOG_PKTRECV) == TOULOG_PKTRECV){
				logFilePktrecv(data);
				logs = logs & ~TOULOG_PKTRECV;
		}else if ((logs & TOULOG_PKT) == TOULOG_PKT){
				logFilePkt(data);
				logs = logs & ~TOULOG_PKT;
		}else if ((logs & TOULOG_SOCKTB) == TOULOG_SOCKTB){
				logFileSocktb(data);
				logs = logs & ~TOULOG_SOCKTB;
		}else if ((logs & TOULOG_PTSRN) == TOULOG_PTSRN){
				 //PRINT TO TERMINAL
				std::cerr << data << endl;
				logs = logs & ~TOULOG_PTSRN;
		}else{
				std::cerr << "[Logger]: logs error\n";
				break;
		}
	}/* End of While */
}

void Logger::logData(int cwnd, unsigned long ssthresh, int awnd,
					 unsigned long snd_nxt, short ccstat, 
					 unsigned short LoggingLevel){

	unsigned short logs =  (LOGFLAG | LoggingLevel);
	double cur_time = timestamp_sec();
	
	if ((cur_time-pb_time) < 0.3 &&
		pb_cwnd == cwnd && 
		pb_awnd == awnd && 
		pb_ssthresh == ssthresh && 
		pb_ccstat == ccstat) 
		return;

	if ((logs & TOULOG_PROBE) == TOULOG_PROBE) {
		logFileProbe(pb_time, pb_cwnd, pb_ssthresh, pb_awnd, pb_snd_nxt, pb_ccstat);
		logFileProbe(cwnd, ssthresh, awnd, snd_nxt, ccstat);
		pb_time = cur_time;
		pb_cwnd = cwnd;
		pb_awnd = awnd;
		pb_ssthresh = ssthresh;
		pb_snd_nxt = snd_nxt;
		pb_ccstat = ccstat;
	}
}
