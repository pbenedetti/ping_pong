/*
 * udp_ping.c: esempio di implementazione del processo "ping" con
 *             socket di tipo DGRAM.
 *
 * versione 2.0 del 16/10/2012
 *
 * Programma sviluppato a supporto del laboratorio di
 * Sistemi di Elaborazione e Trasmissione del corso di laurea
 * in Informatica classe L-31 presso l'Università degli Studi di
 * Genova, anno accademico 2012/2013.
 *
 * Cambiamenti introdotti rispetto alla versione 1:
 *  - aggiunto richiamo della system call clock_getres()
 *  - aggiunto parametro nella chiamata della funzione print_statistics()
 *  - corretto errore nella implementazione del ciclo principale della
 *    funzione main() che richiama la funzione doPing()
 *  - aggiunto exit(0); al termine del main() per far funzionare gli
 *    gli script in laboratorio ... :-O
 *
 * Copyright (C) 2012 by Giovanni Chiola <chiolag@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
*/

/*
#define DEBUG
*/


#include "pingpong.h"

/*
* This function sends and wait for a reply on a socket.
* char message[]: message to send
* int messagesize: message length
*/

static int lost_count = 0;

double doPing(int msg_size, int msg_no, char message[msg_size], int pingSocketFD,
              double TimeOut) {
    char answer_buffer[msg_size];
    int recvBytes,sentBytes, offset;    
    struct sockaddr_storage actualPongAddr;
    int actPongAddrLen = sizeof(struct sockaddr_storage);
    struct timespec sendTime,recvTime;
    double roundTripTimeMilliSeconds;
    int re_try = 0;

    /*** write msg_no at the beginning of the message ***/
	sprintf(message, "%d\n", msg_no);
    /*** TO BE DONE ***/

    do {
	/*** Store the current time in sendTime ***/
	clock_gettime(CLOCK_REALTIME, &sendTime);
        /*** TO BE DONE ***/

	printf("SEND TIME %d : %d\n", sendTime.tv_sec, sendTime.tv_nsec);

	/*** Send the message through the socket ***/
	sentBytes=send(pingSocketFD, message, msg_size, 0);
        /*** TO BE DONE ***/

	printf("lunghezza messagio: %d\n", msg_size);

	/*** Receive an answer through the socket (non blocking mode) ***/
	recvBytes = recv(pingSocketFD,answer_buffer,sizeof(answer_buffer),0);

	/*** Sample the current time in recvTime and compute difference ***/
	clock_gettime(CLOCK_REALTIME, &recvTime);
        /*** TO BE DONE ***/

        roundTripTimeMilliSeconds = timespec_delta2milliseconds(&recvTime,&sendTime);

        for ( offset = 0 ; ( recvBytes >= 0 || errno == EAGAIN || errno == EWOULDBLOCK ) &&
              (recvBytes+offset) < sentBytes && roundTripTimeMilliSeconds < TimeOut;
              offset += ((recvBytes > 0)? recvBytes : 0) ) {
	    recvBytes = recv(pingSocketFD,answer_buffer+offset,sizeof(answer_buffer)-offset,0);

	    /*** Sample the current time in recvTime and compute difference ***/
	clock_gettime(CLOCK_REALTIME, &recvTime);
            /*** TO BE DONE ***/
	   
            roundTripTimeMilliSeconds = timespec_delta2milliseconds(&recvTime,&sendTime);
        }
        if ( recvBytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK )
            fail_errno("UDP ping could not recv from UDP socket");
	if ( (recvBytes+offset) < sentBytes ) { /*time-out elapsed: packet was lost*/
            lost_count++;
            if ( recvBytes < 0 )
                recvBytes = offset;
              else
                recvBytes += offset;
            printf("\n ... received %d bytes instead of %d (lost count = %d)\n",
                   recvBytes, sentBytes, lost_count);
            if ( ++re_try > MAXUDPRESEND ) {
                printf(" ... giving-up!\n");
                break;
            }
            printf(" ... re-trying ...\n");
        }
    } while ( sentBytes != recvBytes );

    return roundTripTimeMilliSeconds;
}


int prepare_udp_socket(char * pong_addr, char * pong_port) {
    struct addrinfo hints, *pongAddr = NULL;
     struct sockaddr_in *ipv4;
    int pingscktFD;
    int gai_rv;
    int np;

    /*** Specify the UDP sockets' options ***/
    memset(&hints, 0, sizeof hints);

    /*** Initialize hints with proper parameters ***/
	hints.ai_family=AF_INET;
	hints.ai_protocol=IPPROTO_UDP;
	hints.ai_socktype=SOCK_DGRAM;

	if(getaddrinfo(pong_addr, pong_port, &hints, &pongAddr)<0){
		fail_errno("getaddrinfo\n");
	}
    /*** TO BE DONE ***/

    /*** create a new UDP socket and connect it with the server ***/
	if( (pingscktFD= socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol))==-1){
		fail_errno("sockeT\n");
	}
    /*** TO BE DONE
         Utilizzare la system call socket() e, in caso di errore,
         la funzione fail_errno() definita nel file function.c   ***/

    /*** change socket behavior to NONBLOCKING ***/
    fcntl(pingscktFD,F_SETFL,O_NONBLOCK);

    /*** Get Pong Server address in binary form and use it to connect to Pong ***/
	ipv4 = (struct sockaddr_in *)pongAddr->ai_addr;

	if((connect(pingscktFD, pongAddr->ai_addr,pongAddr->ai_addrlen))<0){
		printf("valore di ritorno della socket()=%d\n", pingscktFD);
		fail_errno("connect\n");
	}
    /*** TO BE DONE ***/

    return pingscktFD;

}


int main(int argc, char *argv[]) {
    struct addrinfo hints, *serverAddr;
    int pingSocketFD, askSocketFD;;
    int msg_size;
    int gai_rv;
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in *ipv4;
    char request[40], answer[10];
    int nr, pong_port;

    if ( argc != 4 )
    	fail("Incorrect parameters provided. Use: udp_ping PONG_ADDR PONG_PORT MESSAGE_SIZE\n");
    sscanf(argv[3],"%d",&msg_size);
    if ( msg_size < MINSIZE || msg_size > MAXUDPSIZE )
        fail("Wrong message size");

    /*** Specify socket options ***/
    memset(&hints, 0, sizeof hints);
    /*** Initialize hints with proper parameters ***/
	hints.ai_family=AF_INET;
	hints.ai_protocol=IPPROTO_TCP;
	hints.ai_socktype=SOCK_STREAM;

	if(getaddrinfo(argv[1], argv[2], &hints, &serverAddr)<0){
		fail_errno("getaddrinfo\n");
	}
    /*** TO BE DONE ***/


    /*** Get Pong Server address in binary form ***/
	ipv4 = (struct sockaddr_in *)serverAddr->ai_addr;
    /*** TO BE DONE ***/

    /*** Print address of the Pong server before trying to connect ***/
    
    printf("UDP Ping trying to connect to server %s: (%s) on TCP port %s\n", argv[1],
           inet_ntop(AF_INET,&(ipv4->sin_addr),ipstr,INET_ADDRSTRLEN),argv[2]);

    /*** create a new TCP socket and connect it with the server ***/
	if( (askSocketFD= socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol))==-1){
		fail_errno("sockeT\n");
	}

	if((connect(askSocketFD, serverAddr->ai_addr,serverAddr->ai_addrlen))<0){
		fail_errno("connect\n");
	}
    /*** TO BE DONE
         Utilizzare le system call socket() e connect() e, in caso di errore,
         la funzione failr_errno definita nel file function.c   ***/


    printf(" ... connected to Pong server: asking for %d repetitions of %d Bytes UDP messages\n",
           REPEATS,msg_size);
    sprintf(request,"UDP %d %d\n", msg_size, REPEATS);

    /*** Write the request on socket ***/
	if(write(askSocketFD, request, strlen(request))>msg_size){
		fail_errno("4\n");
	}
    /*** TO BE DONE ***/


    for ( nr = read(askSocketFD,(void*)answer,sizeof(answer)) ; nr < 1 ;
          nr = read(askSocketFD,(void*)answer,sizeof(answer)) )
        if ( nr < 0 )
            fail_errno("UDP Ping could not receive answer from Pong server");

    /*** Check if the answer is OK, and fail if it is not ***/
	if(answer[0]=='E'){
		fail_errno("5\n");
	}
    /*** TO BE DONE ***/

    sscanf(answer+3,"%d\n",&pong_port);
    printf(" ... Pong server agreed to ping-pong using port %d :-)\n",pong_port);

    /*** Pong returned the port number to be used in the subsequent UDP connection ***/
    sprintf(answer,"%d",pong_port);
    shutdown(askSocketFD,SHUT_RDWR); /*** close TCP connection ***/
    close(askSocketFD); /*** release TCP socket ***/

    pingSocketFD = prepare_udp_socket(argv[1],answer);

    { char message[msg_size];
      memset(&message,0,msg_size);
      double currentPingTimes[REPEATS];
      struct timespec zero, resolution;
      int repeat;
      for ( repeat = 0 ; repeat < REPEATS ; repeat++ ) {  
          /*Pings the server*/
          currentPingTimes[repeat] = doPing(msg_size,repeat+1,message,pingSocketFD,
                                            UDP_TIMEOUT);
          printf("Round trip time was %6.3lf milliseconds in repetition %d\n",
                 currentPingTimes[repeat],repeat+1);
       }

      memset((void*)(&zero),0,sizeof(struct timespec));
      if ( clock_getres(CLOCK_MONOTONIC,&resolution) != 0 )
          fail_errno("UDP Ping could not get timer resolution");
      print_statistics(stdout,"UDP Ping: ",REPEATS,currentPingTimes,msg_size,
                       timespec_delta2milliseconds(&resolution,&zero));

    }

    /*** Free the socket descriptor and terminate ***/
    close(pingSocketFD);
    exit(0);
}
