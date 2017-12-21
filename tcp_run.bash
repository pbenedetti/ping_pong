#!/bin/bash
# expects message size as parameter #1

if [[ $# != 1 ]] ; then printf "\nError: message size expected as a parameter\n\n" ; exit 1; fi

../bin/tcp_ping felix.disi.unige.it 1491 $1 > ../data/tcp_$1.out

