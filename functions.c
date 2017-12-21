/*
 * functions.c: funzioni ausiliarie per il ping-pong
 *
 * versione 2.0 del 02/10/2012
 *
 * Programma sviluppato a supporto del laboratorio di
 * Sistemi di Elaborazione e Trasmissione del corso di laurea
 * in Informatica classe L-31 presso l'Università degli Studi di
 * Genova, anno accademico 2012/2013.
 *
 * Cambiamenti introdotti rispetto alla versione 1:
 *  - cambiati i nomi dei parametri della funzione timespec_delta2milliseconds()
 *    per togliere ambiguita` di interpretazione
 *  - cambiato il calcolo del range dell'istogramma basato sulla risoluzione
 *    del timer utilizzato, aggiungendo un parametro alla funzione print_statistics()
 *  - corretto gli errori di calcolo dei valori estremi per l'istogramma

 * Copyright (C) 2012 by Giovanni Chiola <chiolag@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

/* Printing errors */

void fail_errno(char *msg) {
  perror(msg);
  exit(1);
}

void fail(char *msg) {
  fprintf(stderr,"%s\n",msg);
  exit(1);
}

double timespec_delta2milliseconds( struct timespec * last, struct timespec * previous) {

    /* This function computes the difference (last - previous) and returns the result expressed in milliseconds as a double */
	double sendT;
	double rcvT;

	sendT=(double)(previous->tv_sec*1000.0)+((double)previous->tv_nsec)/1000000.0;
	rcvT=(double)(last->tv_sec*1000.0)+((double)last->tv_nsec)/1000000.0;

	return rcvT-sendT;
    
    /* TO BE DONE */

}


int double_cmp( const void * p1, const void * p2 ) {
  double d1 = *((double*)p1);
  double d2 = *((double*)p2);
  if ( d1 > d2 ) return 1;
  if ( d1 < d2 ) return -1;
  return 0;
}


void print_statistics(FILE * outf, const char * name, int repeats, double rtt[repeats],
                      int msg_sz, double resolution) {
#define HISTOITEMS 21
    int median = repeats/2;
    int p10 = repeats/10;
    int p90 = 9*p10;
    int histogram[HISTOITEMS];
    double h_min, h_max, h_incr;
    int i, j;
    double mean, var, ns;

    qsort((void*)rtt,repeats,sizeof(double),double_cmp);
    printf("\n ... clock resolution was %lg",resolution);
    h_min = resolution*(double)((long)(rtt[0]/resolution))-0.5*resolution;
#ifdef DEBUG
    printf(" h_min=%lg, ",h_min);
#endif
    if ( h_min < 0.0 )
        h_min = 0.0;
    h_max = resolution*(double)((long)((rtt[repeats-1]+0.5*resolution)/resolution))+0.5*resolution;
    h_incr = (h_max-h_min)/(double)(HISTOITEMS-1);
#ifdef DEBUG
    printf(" h_max=%lg, h_incr=%lg\n",h_max,h_incr);
#endif
    for ( j = 0 ; j < HISTOITEMS ; j++ )
        histogram[j] = 0;
    for ( i = 0, mean = var = ns = 0.0 ; i < repeats ; i++ ) {
        double v = rtt[i];
        double delta = v - mean;
#ifdef DEBUG
        printf("     rtt[%d]=%lg\n",i,rtt[i]);
#endif

        ns += 1.0;
        mean += delta/ns;
        if ( ns > 1.5 ) {
            var += delta*(v-mean);
        }
        v -= h_min; 
	v += h_incr*0.5;	
	v /= h_incr;
	j = (int)v;
        histogram[j]++;
    }
    var /= (double)(repeats-1);
    fprintf(outf,"\n%s Statistics over %d repetitions of %d Byte messages\n\n",
            name, repeats,msg_sz);
    fprintf(outf,"RTT : percentile 10: %lg, median: %lg, percentile 90: %lg, average: %lg variance: %lg\n\n",
             rtt[p10], rtt[median], rtt[p90], mean, var);
    fprintf(outf,"RTT histogram:\n");
    for ( j = 0, ns = h_min ; j < HISTOITEMS ; j++, ns += h_incr )
        fprintf(outf,"%lg %d\n",ns,histogram[j]);
    fprintf(outf,"\n");
    if ( rtt[median] > 0.0 )
        fprintf(outf,"   median Throughput : %lg KB/s",2.0*(double)(msg_sz)/rtt[median]);
    if ( mean > 0.0 )
        fprintf(outf,"   overall Throughput : %lg KB/s",2.0*(double)(msg_sz)/mean);
    fprintf(outf,"\n\n");
}
