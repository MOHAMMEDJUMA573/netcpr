#ifndef _HEADERS_N
#define _HEADERS_N
/*Libraries*/
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
/*App level libraries*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
/*Macros*/
#define CLOSESOCKET(s) close(s)
#endif