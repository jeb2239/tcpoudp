#include "Logger.h"
#include "timestamp.h"
#include <string>
#include <stdio.h>
#include <bitset>
#include <iostream>

Logger lg;
unsigned short	LOGFLAG;
bool						LOGON;

using namespace std;
Logger::Logger()
{
	file.open("./Tou.log");
	file_timer.open("./Tou_timer.log");
	file_pktsent.open("./Tou_pktsent.log");
	file_pktrecv.open("./Tou_pktrecv.log");
	file_pkt.open("./Tou_pkt.log");
	file_socktb.open("./Tou_socktb.log");

	file.flush();
	file_timer.flush();
	file_pktsent.flush();
	file_pktrecv.flush();
	file_pkt.flush();
	file_socktb.flush();
}

Logger::~Logger(){
	shutdownLogger();
}

Logger Logger::logger;

Logger& Logger::getLogger()
{
	return Logger::logger;
}

void Logger::shutdownLogger()
{
	file.close();	
	file_timer.close();
	file_pktsent.close();
	file_pktrecv.close();
	file_pkt.close();
	file_socktb.close();
}

void Logger::logFile(string data){
	file << timestamp() <<  data << endl;
	file.flush();
}
void Logger::logFileTimer(string data){
	file_timer << data << endl;
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


void Logger::logData(string data, unsigned short LoggingLevel){
	if (LOGON == false)  return;
	unsigned short logs =  (LOGFLAG | LoggingLevel);
	//cout << std::bitset<4>(logs) << endl;
	
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
