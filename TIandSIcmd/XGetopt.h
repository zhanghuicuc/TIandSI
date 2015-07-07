/**
  * @file XGetopt.cpp  Version 1.2
  *
  * Description:
  *     XGetopt.cpp implements getopt(), a function to parse command lines.
  * History
  *     Version 1.2 - 2003 May 17
  *     - Added Unicode support
  *
  *     Version 1.1 - 2002 March 10
  *     - Added example to XGetopt.cpp module header 
  * This software is released into the public domain.
  * You are free to use it in any way you like.
  * This software is provided "as is" with no expressed
  * or implied warranty.  I accept no liability for any
  * damage or loss of business that this software may cause.
  *
  * @author Hans Dietrich hdietrich2@hotmail.com
  */
///////////////////////////////////////////////////////////////////////////////

#ifndef XGETOPT_H
#define XGETOPT_H

#include <windows.h>
#include <stdio.h>

extern int optind, opterr;
extern char *optarg;

int getopt(int argc, char *argv[], char *optstring);

#endif //XGETOPT_H
