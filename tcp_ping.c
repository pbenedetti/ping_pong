/*
 * tcp_ping.c: esempio di implementazione del processo "ping" con
 *             socket di tipo STREAM.
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
 * int msg_size: message length
 * int msg_no: message sequence number (written into the message)
 * char message[msg_size]: buffer to send
 * int tcp_socketFD: socket file descriptor
 */
double doPing(int msg_size, int msg_no, char message[msg_size], int tcp_socketFD) {
    char rec_buffer[msg_size];
    int recvBytes, sentBytes, offset;    
    struct timespec sendTime,recvTime;

    /*** write msg_no at the beginning of the message ***/
    /*** TO BE DONE ***/sprintf(message,"%d\n",msg_no);

    /*** Store the current time in sendTime ***/
    /*** TO BE DONE ***/clock_gettime(CLOCK_REALTIME,&sendTime);

	printf("SEND TIME %d : %d\n",sendTime.tv_sec,sendTime.tv_nsec);

    /*** Send the message through the socket ***/
    /*** TO BE DONE ***/sentBytes=send(tcp_socketFD, message, msg_size,0);

	printf("\nlunghezza messaggio: %d\n",msg_size);

    /*** Receive an answer through the socket (blocking) ***/
    for ( offset = 0 ;
          (offset+(recvBytes = recv(tcp_socketFD,rec_buffer+offset,sentBytes-offset,MSG_WAITALL))) < msg_size ;
          offset += recvBytes ) {
#ifdef DEBUG
        printf(" ... received %d bytes back\n", recvBytes);
#endif
        if ( recvBytes < 0 )
            fail_errno("Error receiving data");
    }

    /*** Sample the current time in recvTime and return the difference ***/
    /*** TO BE DONE ***/clock_gettime(CLOCK_REALTIME,&recvTime);

	  	printf("REC TIME %d : %d\n",recvTime.tv_sec,recvTime.tv_nsec);	

    printf("tcp_newping received %d bytes back\n", recvBytes);

    return timespec_delta2milliseconds(&recvTime,&sendTime);
}


int main(int argc, char *argv[]) {
    struct addrinfo hints, *serverAddr;
    int msgsz;
    int gai_rv;
    int optval = 1;
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in *ipv4;
    int tcp_socketFD;
    char request[30], answer[10];
    int nr;

    if ( argc != 4 )
    	fail("Incorrect parameters provided. Use: tcp_ping PONG_ADDR PONG_PORT SIZE\n");

    /*** Specify socket options ***/
    memset(&hints, 0, sizeof hints);

    /*** Initialize hints with proper parameters ***/
	hints.ai_family=AF_INET;
	hints.ai_protocol= IPPROTO_TCP;
	hints.ai_socktype=SOCK_STREAM;
	if(getaddrinfo(argv[1], argv[2], &hints, &serverAddr)<0){
	//printf("%s\n", gai_strerror(getaddrinfo(argv[1], argv[2], &hints, &serverAddr)));
		fail_errno("\nerrore getaddrinfo()\n");	
	}		
    /*** TO BE DONE ***/



    /*** Get Pong Server address in binary form ***/    
 	ipv4 = (struct sockaddr_in *)serverAddr->ai_addr;
    /*** TO BE DONE ***/

    /*** Print address of the Pong server before trying to connect ***/
   
    printf("TCP Ping trying to connect to server %s: (%s) on port %s\n", argv[1],
           inet_ntop(AF_INET,&(ipv4->sin_addr),ipstr,INET_ADDRSTRLEN),argv[2]);   

    /*** create a new TCP socket and connect it with the server ***/
	if((tcp_socketFD= socket(AF_INET, SOCK_STREAM, 0))==-1){
		fail_errno("\nerrore apertura socket()\n");
	}	

	if(connect(tcp_socketFD, serverAddr->ai_addr, serverAddr->ai_addrlen)==-1){
		fail_errno("\nerrore connect(): perche??\n");	
	}
    /*** TO BE DONE
         Utilizzare le system call socket() e connect() e, in caso di errore, 
         la funzione fail_errno() definita nel file function.c   ***/


    /*** Disable Nagle's algorithm ***/
    setsockopt(tcp_socketFD,IPPROTO_TCP,TCP_NODELAY,(void*)&optval,sizeof(int));

    sscanf(argv[3],"%d",&msgsz);
    if ( msgsz < MINSIZE )
        msgsz = MINSIZE;
    else if ( msgsz > MAXTCPSIZE )
        msgsz = MAXTCPSIZE;
    printf(" ... connected to Pong server: asking for %d repetitions of %d Bytes TCP messages\n",
           REPEATS,msgsz);
    sprintf(request,"TCP %d %d\n", msgsz, REPEATS);
    
    /*** Write the request on socket ***/
	if(write(tcp_socketFD,request,strlen(request))>msgsz){
			fail_errno("\nerrore write()\n");	
		}
    /*** TO BE DONE ***/

    for ( nr = read(tcp_socketFD,(void*)answer,sizeof(answer)) ; nr < 1 ;
          nr = read(tcp_socketFD,(void*)answer,sizeof(answer)) )
        if ( nr < 0 )
    	    fail_errno("TCP Ping could not receive answer from Pong server");

    /*** Check if the answer is OK, and fail if it is not ***/
	if(answer[0]=='E'){
		fail_errno("\nerrore risposta server\n");
	}	
    /*** TO BE DONE ***/

    printf(" ... Pong server agreed :-)\n");


    { double currentPingTimes[REPEATS];
      struct timespec zero, resolution;
      char message[msgsz];
      int rep;

      memset(message,0,msgsz);
      for ( rep = 1 ; rep <= REPEATS ; rep++ ) {  
          currentPingTimes[rep-1] = doPing(msgsz,rep,message,tcp_socketFD);
	  printf("Round trip time was %lg milliseconds in repetition %d\n",
                 currentPingTimes[rep-1],rep);
      }

      memset((void*)(&zero),0,sizeof(struct timespec));
      if ( clock_getres(CLOCK_MONOTONIC,&resolution) != 0 )
          fail_errno("TCP Ping could not get timer resolution");
      print_statistics(stdout,"TCP Ping: ",REPEATS,currentPingTimes,msgsz,
                       timespec_delta2milliseconds(&resolution,&zero));

    }

    shutdown(tcp_socketFD, SHUT_RDWR); 
    close(tcp_socketFD);
    exit(0);

}
