/**
 * timestamp.cpp
 * return the current time stamp in a format of string
 *
 * Copyright 2010 by Columbia University; all rights reserved
 * Jan 22, 2010
 */
#include "timestamp.h"

using namespace std;

/**
 * timestamp:
 * return the current time in a format of string
 */
string timestamp(void) {
	const tm		*tm;			//time structure
	time_t			current;	//current time
	char				retval_c[30];

	current = time(NULL); //or time(&current);
	tm = localtime(&current);
	strftime(retval_c, 30, "%d %B %Y %I:%M:%S %p", tm);
	
	string retval = retval_c;
	return retval;
}
	
