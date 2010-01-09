/*
 * Copyright (C) 2008 The Trustees of Columbia University in the City of New York. 
 * All rights reserved.
 *
 * See the file LICENSE included in this distribution for details.
 */

#ifndef _TRACE_H
#define _TRACE_H

#include <stdio.h>
//#include "types.h"
#include <string>
using namespace std;

extern FILE* _fptrace;
extern int g_tracelevel;
extern int idlen;


//TRACE(5,"KadPeer::OnTimer...transaction map: %d LObject map: %d\n", m_TMap.size(), m_LO.size() );
//#ifdef _LINUX_

#include <stdarg.h>

inline void TRACE(int level, char* format, ...) {

  va_list ap;
  int d;
  unsigned int ud;
  char c, *s, cflag;
  unsigned int uc;
  string str="%";
  //char temp[10];
printf("21 TRACE(5,KadPeer::OnTimer...transaction map:  LObject map: \n");
  va_start(ap, format);
  while (*format){
    
    str="%";

    if(*format!='%'){
      fprintf(_fptrace, "%c", *format);
      format += 1;
      continue;
    }
    format++;

    while( (*format < 97 || *format > 122) && *format != '\0') {
      str += *format;
      format++;
    }

    str+=*format;

    cflag = *((char*)str.c_str() + str.size()-1);

    switch(cflag) {
    case 's':                       // string
      s = va_arg(ap, char *);
      fprintf(_fptrace,"%s", s);
      break;
    case 'd':                       // int
      d = va_arg(ap, int);
      fprintf(_fptrace,"%d", d);
      break;
    case 'c':                       // char
      // need a cast here since va_arg only
      // takes fully promoted types
      c = (char) va_arg(ap, int);
      fprintf(_fptrace,"%c", c);
      break;

    case 'u':
      ud = va_arg(ap, unsigned int);
      fprintf(_fptrace,"%u", ud);
      break;
    
    case 'x':
      uc = (unsigned int) va_arg(ap, unsigned int);
      fprintf(_fptrace, (char*)str.c_str(), uc);
      break;
      
    }//end switch

  
    format++;
  }
  va_end(ap);
}

#define TRACE(level, format, args ...) { \
  if(level <= 5) { \
    fprintf(_fptrace, format, args ); \
    fflush(_fptrace); \
  } \
}


/*
  if(level == 2) \
    fprintf(_fptrace, format, args ); \

*/

//#endif

#ifdef WIN32

#include <stdarg.h>

//inline void TRACE(int level, char* format, ...) {

#ifdef DEBUG 

  va_list ap;
  int d;
  unsigned int ud;
  char c, *s, cflag;
  unsigned int uc;
  string str="%";
  //char temp[10];

  va_start(ap, format);
  while (*format){
    
    str="%";

    if(*format!='%'){
      fprintf(_fptrace, "%c", *format);
      format += 1;
      continue;
    }
    format++;

    while( (*format < 97 || *format > 122) && *format != '\0') {
      str += *format;
      format++;
    }

    str+=*format;

    cflag = *((char*)str.c_str() + str.size()-1);

    switch(cflag) {
    case 's':                       // string
      s = va_arg(ap, char *);
      fprintf(_fptrace,"%s", s);
      break;
    case 'd':                       // int
      d = va_arg(ap, int);
      fprintf(_fptrace,"%d", d);
      break;
    case 'c':                       // char
      // need a cast here since va_arg only
      // takes fully promoted types
      c = (char) va_arg(ap, int);
      fprintf(_fptrace,"%c", c);
      break;

    case 'u':
      ud = va_arg(ap, unsigned int);
      fprintf(_fptrace,"%u", ud);
      break;
    
    case 'x':
      uc = (unsigned int) va_arg(ap, unsigned int);
      fprintf(_fptrace, (char*)str.c_str(), uc);
      break;
      
    }//end switch

  
    format++;
  }
  va_end(ap);

#endif

//}//TRACE

/*
#define TRACE(level, format, args) { \
}

#define TRACE(level, format, args, a) { \
}

#define TRACE(level, format, args, a, b) { \
}

#define TRACE(level, format, args, a, b, c) { \
}
*/

#endif

#endif


  /*  va_list argptr;

  va_start(argptr, format);
  char *s=NULL;
  char c=0;
  int i=0;

  while (*format != 0) {
    if (*format != '%') {
      fprintf(_fptrace, "%c", *format);
      format += 1;
      continue;
    }
    format += 1;
    if (*format == 's') {
      s = (char*)va_arg(argptr, char *);
      fprintf(_fptrace, "%s", s);
    } else if (*format == 'c') {
      c = va_arg(argptr, int);
      fprintf(_fptrace, "%c", c);
    } else if (*format == 'd') {
      i = va_arg(argptr, int);
      fprintf(_fptrace, "%d", i);
    }
    format += 1;
  }
  va_end(argptr);
 }
  */
