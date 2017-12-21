#ifndef pINGpONG_h
# define pINGpONG_h

/*
 * pingpong.h: include file per l'implementazione del ping-pong TCP e UDP.
 *
 * versione 2.0 del 16/10/2012
 *
 * Programma sviluppato a supporto del laboratorio di
 * Sistemi di Elaborazione e Trasmissione del corso di laurea
 * in Informatica classe L-31 presso l'Università degli Studi di
 * Genova, anno accademico 2012/2013.
 *
 * Cambiamenti introdotti rispetto alla versione 1:
 * - aumentato il valore della costante REPEATS
 * - ridotto il valore della costante MINSIZE
 * - cambiato il prototipo della funzione print_statistics() aggiungendo un parametro
 * - cambiato il prototipo della funzione timespec_delta2milliseconds()
 *   usando nomi meno ambigui
 *
 * Copyright (C) 2012 by Giovanni Chiola <chiolag@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#define REPEATS 101 //Number of tests
#define MAXREPEATS 1001 //Max number of tests
#define MINSIZE 16 //Minimum message size
#define LISTENBACKLOG 1
#define UDP_TIMEOUT ((double)1000.0) // 1 second
#define MAXTCPSIZE (1024*1024)
#define MAXUDPSIZE 65500
#define MAXUDPRESEND 3
#define IANAMINEPHEM 49152
#define IANAMAXEPHEM 65535
#define PONGRECVTOUT 10

extern void fail_errno(char *msg);
extern void fail(char *msg);
extern double timespec_delta2milliseconds(struct timespec * last, struct timespec * previous);
extern void print_statistics(FILE * outf, const char * name, int repeats, double rtt[repeats], int msg_sz, double resolution);

#endif /* pINGpONG_h */
