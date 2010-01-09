#include "Logger.h"
#include <string>
#include <stdio.h>
#include <iostream>
using namespace std;

Logger::Logger()
{
	file.open("./ToU.log");
	file.flush();
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
}


void Logger::logData(string data,int LoggingLevel)
{
	//DEBUG
	if(LoggingLevel == 0)
		{
			file << data;
			file.flush();
		}
	//PRINT TO TERMINAL
	 else if(LoggingLevel == 1)
				cout << data;
}
