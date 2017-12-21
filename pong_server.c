/*
 * pong_server.c: server in ascolto su una porta TCP "registrata".
 *                Il client invia una richiesta specificando il protocollo
 *                desiderato (TCP/UDP), il numero di messaggi e la loro lunghezza.
 *                Il server (mediante una fork()) crea un processo dedicato
 *                a svolgere il ruolo di "pong" secondo le richieste del client.
 *
 * versione 2.0 del 16/10/2012
 *
 * Programma sviluppato a supporto del laboratorio di
 * Sistemi di Elaborazione e Trasmissione del corso di laurea
 * in Informatica classe L-31 presso l'Università degli Studi di
 * Genova, anno accademico 2012/2013.
 *
 * Cambiamenti introdotti rispetto alla versione 1:
 *  - corretto errori nell'invio delle risposte OK ed ERROR, sostituendo strlen()
 *    a sizeof()
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

void tcp_pong(int message_no, int message_size, FILE * in_stream, int out_socket) {
    char buffer[message_size], * cp;
    int n_msg, n_c;

    for ( n_msg = 1 ; n_msg <= message_no ; n_msg++ ) {
        int seq = 0;
        for ( cp = buffer, n_c = 0 ; n_c < message_size ; n_c++, cp++ ) {
            int cc = getc(in_stream);
            if ( cc == EOF )
                fail("TCP Pong received fewer bytes than expected");
            *cp = cc;
        }
        sscanf(buffer," %d\n",&seq);
#ifdef DEBUG
        printf(" Got %d sequence number (expecting %d)\n%s\n",seq,n_msg,buffer);
#endif
        if ( seq != n_msg )
            fail("TCP Pong received wrong message sequence number");
        if ( send(out_socket,buffer,message_size,0) < message_size )
            fail_errno("TCP Pong failed sending data back");
    }
}


void udp_pong(int dgrams_no, int dgram_sz, int pongSocketFD) {
    char buffer[dgram_sz];
    int n, receivedBytes, sentBytes, resend;         
    struct sockaddr_storage pingAddr;
    int pingAddrLen;

    for ( n = resend = 0 ; n < dgrams_no ; ) {
        int i;

        pingAddrLen = sizeof(struct sockaddr_storage);
        if ( (receivedBytes = recvfrom(pongSocketFD,buffer,sizeof buffer,0,
                                       (struct sockaddr*)&pingAddr,&pingAddrLen)) < 0 )
            fail_errno("UDP Pong recv failed");
        if ( receivedBytes < dgram_sz )
            fail("UDP Pong received fewer bytes than expected");
        sscanf(buffer,"%d\n",&i);
#ifdef DEBUG
{ struct sockaddr_in * ipv4_addr = (struct sockaddr_in*)&pingAddr;
  char addrstr[INET_ADDRSTRLEN];
  char * cp = inet_ntop(AF_INET,(const void*)&(ipv4_addr->sin_addr),addrstr,INET_ADDRSTRLEN);
  int j;
        if ( cp == NULL )
            printf(" could not print address string\n");
          else
            printf("  ... received %d bytes from %s, port %x, sequence number %d (expecting %d)\n",
                   receivedBytes,cp,ipv4_addr->sin_port,i,n);
        for ( j = 0 ; j < dgram_sz ; j++ )
            printf("%c",buffer[j]);
        printf("\n-------------------------------------\n");
}
#endif
        if ( i < n || i > dgrams_no || i < 1 )
            fail("UDP Pong received wrong datagram sequence number");
        if ( i > n ) { // new datagram to pong back
            n = i; resend = 0;
        } else { // resend previous datagram
            if ( ++resend > MAXUDPRESEND )
                fail("UDP Pong maximum resend count exceeded");
        }
        if ( (sentBytes = sendto(pongSocketFD,buffer,receivedBytes,0,
                                 (struct sockaddr*)&pingAddr,pingAddrLen)) < 0 )
            fail_errno("UDP Pong failed sending datagram back");
    }
}


static int pong_port = -1;

/*** The following function creates a new UDP socket and binds it
 *   to a free Ephemeral port according to IANA definition.
 *   The port number is stored in the static variable "pong_port"
 *   as a side effect.
 */
int open_udp_socket(void) {
    struct addrinfo hints, * pongAddr;
    int scktFD, np, gai_rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;

    for ( np = IANAMINEPHEM ; np <= IANAMAXEPHEM ; np++ ) {
      char npstring[6];
      sprintf(npstring,"%d",np);
      if ((gai_rv = getaddrinfo(NULL, npstring, &hints, &pongAddr)) != 0) 
      	  fail((char *)gai_strerror(gai_rv));
      if ( (scktFD = socket(pongAddr->ai_family,pongAddr->ai_socktype,pongAddr->ai_protocol))
            == -1 ) {
          freeaddrinfo(pongAddr);
          fail_errno("UDP Pong could not get socket");
      }
      if ( bind(scktFD,pongAddr->ai_addr,pongAddr->ai_addrlen) == 0 ) {
        /*** a free ephemeral port was found ***/
          struct timeval optval;
          optval.tv_sec = PONGRECVTOUT;
          optval.tv_usec = 0;
          // set timeout on datagram receive
          setsockopt(scktFD,SOL_SOCKET,SO_RCVTIMEO,(void*)&optval,sizeof(struct timeval));
          freeaddrinfo(pongAddr);
          pong_port = np;
          return scktFD;
      }
      /*** else ***/
      freeaddrinfo(pongAddr);
      if ( errno != EADDRINUSE ) {
          close(scktFD);
          fail_errno("UDP Pong could not bind socket");
      }
      close(scktFD);
    }
    fprintf(stderr,"UDP Pong could not find any free ephemeral port\n");
    return -1;
}


int main(int argc, char *argv[]) {
    struct addrinfo hints, *pongServerAddr;
    int pongSocketFD, requestFD, gai_rv;

    if (argc!=2)
        fail("Pong Sever incorrect syntax. Use: pong_server PORTNUM\n");

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;
    if ((gai_rv = getaddrinfo(NULL, argv[1], &hints, &pongServerAddr)) != 0)
        fail((char *)gai_strerror(gai_rv));
    if ((pongSocketFD = socket(pongServerAddr->ai_family,pongServerAddr->ai_socktype,
                               pongServerAddr->ai_protocol)) == -1 )
        fail_errno("Pong server could not get socket descriptor");
    if ( bind(pongSocketFD,pongServerAddr->ai_addr,pongServerAddr->ai_addrlen) == -1 ) {
        close(pongSocketFD);
        fail_errno("Pong server could not bind socket");
    }
    if ( listen(pongSocketFD,LISTENBACKLOG) == -1 ) {
        close(pongSocketFD);
        fail_errno("Pong server could not listen on socket");
    }
    fprintf(stderr,"Pong server listening on port %s ...\n",argv[1]);
    freeaddrinfo(pongServerAddr);

    for(;;) {
        struct sockaddr_storage clientAddr;
        socklen_t addrSize = sizeof(clientAddr);
        int requestFD = accept(pongSocketFD,(struct sockaddr*)&clientAddr,&addrSize);
        pid_t pid;

        if ( requestFD == -1 ) {
            close(pongSocketFD);
            fail_errno("Pong server could not accept client connection");
        }
        if ( (pid = fork()) < 0 )
            fail_errno("Pong server could not fork");
        if ( pid == 0 ) {
          /*** await for client request ***/
            FILE * requeststream = fdopen(requestFD,"r");
            char answer_buf[10];
            char * request_str = NULL, * strtokr_save;
            char * protocol_str, * size_str, * number_str;
            size_t n;
            int message_size, message_no;
            struct timeval optval;
            char client_string[INET_ADDRSTRLEN];
            struct sockaddr_storage clientAddr;
            socklen_t addrSize=sizeof(clientAddr);

            if ( inet_ntop(AF_INET,(const void*)&(((struct sockaddr_in*)&clientAddr)->sin_addr),client_string,INET_ADDRSTRLEN) == NULL )
                fail_errno("Pong server could not print client address string");

            optval.tv_sec = PONGRECVTOUT;
            optval.tv_usec = 0;
            setsockopt(requestFD,SOL_SOCKET,SO_RCVTIMEO,(void*)&optval,sizeof(struct timeval));

            if ( getline(&request_str,&n,requeststream) > 0 ) {
                int pongFD;

                protocol_str = strtok_r(request_str," ",&strtokr_save);
                if ( strncmp(protocol_str,"TCP",3) != 0 && strncmp(protocol_str,"UDP",3) != 0 )
                    goto request_error;
                size_str = strtok_r(NULL," ",&strtokr_save);
                sscanf(size_str,"%d",&message_size);
                if ( message_size < MINSIZE || message_size > MAXTCPSIZE ||
                     strncmp(protocol_str,"UDP",3) == 0 && message_size > MAXUDPSIZE )
                    goto request_error;
                number_str = strtok_r(NULL," ",&strtokr_save);
                sscanf(number_str,"%d",&message_no);
                if ( message_no < 1 || message_no > MAXREPEATS )
                    goto request_error;
                if ( strncmp(protocol_str,"UDP",3) == 0 ) {
                    pongFD = open_udp_socket();
                    if ( pongFD < 0 )
                        goto request_error;
                    sprintf(answer_buf,"OK %d\n",pong_port);
                    send(requestFD,answer_buf,strlen(answer_buf),0);
                    shutdown(requestFD,SHUT_RDWR);
                    close(requestFD);
                    udp_pong(message_no,message_size,pongFD);
                } else {
                    int optv = 1;
                    pongFD = requestFD;
                    setsockopt(pongFD,IPPROTO_TCP,TCP_NODELAY,(void*)&optv,sizeof(int));
                    sprintf(answer_buf,"OK\n");
                    send(requestFD,answer_buf,strlen(answer_buf),0);
                    tcp_pong(message_no,message_size,requeststream,pongFD);
                    shutdown(pongFD,SHUT_RDWR);
                }
                close(pongFD);
                exit(0);
            }
  request_error:
            sprintf(answer_buf,"ERROR\n");
            send(requestFD,answer_buf,sizeof(answer_buf),0);
            close(requestFD);
            exit(1);
        }
        /*** else await for next client connection ***/
        close(requestFD);
    }

}
