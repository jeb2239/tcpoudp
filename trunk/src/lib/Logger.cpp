#include "Logger.h"
#include <string>
#include <stdio.h>
#include <iostream>

Logger lg;

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

Logger::~Logger()
{
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
	file << data << endl;
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
	logstate = LoggingLevel;
	
	while( logstate != 0x0000 ){
		if ((logstate & TOULOG_ALL) == TOULOG_ALL){
				// log all
				logFile(data);
				logstate = logstate & ~TOULOG_ALL;
		}else if ((logstate & TOULOG_TIMER) == TOULOG_TIMER){
				logFileTimer(data);
				logstate = logstate & ~TOULOG_TIMER;
		}else if ((logstate & TOULOG_PKTSENT) == TOULOG_PKTSENT){
				logFilePktsent(data);
				logstate = logstate & ~TOULOG_PKTSENT;
		}else if ((logstate & TOULOG_PKTRECV) == TOULOG_PKTRECV){
				logFilePktrecv(data);
				logstate = logstate & ~TOULOG_PKTRECV;
		}else if ((logstate & TOULOG_PKT) == TOULOG_PKT){
				logFilePkt(data);
				logstate = logstate & ~TOULOG_PKT;
		}else if ((logstate & TOULOG_SOCKTB) == TOULOG_SOCKTB){
				logFileSocktb(data);
				logstate = logstate & ~TOULOG_SOCKTB;
		}else if ((logstate & TOULOG_PTSRN) == TOULOG_PTSRN){
				 //PRINT TO TERMINAL
				std::cerr << data;
				logstate = logstate & ~TOULOG_PTSRN;
		}else{
				std::cerr << "[Logger]: logstate error\n";
				break;
		}
	}/* End of While */
}
