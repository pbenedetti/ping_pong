#!/bin/bash

# Expects MaxUDPsize and MaxTCPsize as parameters
if [[ $# != 2 ]] ; then printf "\nError: MaxUDPsize and MaxTCPsize expected as parameters\n\n" ; exit 1; fi

declare -a Usz
declare -a Uname
for (( (undx=1 , sz=32) ; $sz <= $1 ; (undx=$undx+1 , sz=$sz+$sz) )) do \
	Usz[$undx]=$sz
	Uname[$undx]=../data/udp_$sz.out
done

declare -a Tsz
declare -a Tname
for (( (tndx=1 , sz=32) ; $sz <= $2 ; (tndx=$tndx+1 , sz=$sz+$sz) )) do \
	Tsz[$tndx]=$sz
	Tname[$tndx]=../data/tcp_$sz.out
done

/bin/rm -f ../data/Makefile


printf "throughput : ../data/throughput.png\n\n" > ../data/Makefile
printf "all_data : all_udp_data all_tcp_data\n\nall_udp_data :" >> ../data/Makefile
for (( indx=1 ; $indx < $undx ; indx=$indx+1 )) do \
	printf " %s" ${Uname[$indx]} >> ../data/Makefile
done
printf "\n\nall_tcp_data :" >> ../data/Makefile
for (( indx=1 ; $indx < $tndx ; indx=$indx+1 )) do \
	printf " %s" ${Tname[$indx]} >> ../data/Makefile
done

for (( indx=1 ; $indx < $undx ; indx=$indx+1 )) do \
	printf "\n\n%s :\n\t../scripts/udp_run.bash %s" ${Uname[$indx]} ${Usz[$indx]} >> ../data/Makefile
done

for (( indx=1 ; $indx < $tndx ; indx=$indx+1 )) do \
	printf "\n\n%s :\n\t../scripts/tcp_run.bash %s" ${Tname[$indx]} ${Tsz[$indx]} >> ../data/Makefile
done

printf "\n\n../data/throughput.png : ../data/tcp_throughput.dat ../data/udp_throughput.dat\n\t../scripts/gplot.bash\n\n" >> ../data/Makefile
printf "../data/udp_throughput.dat : all_udp_data\n\t../scripts/collect_throughput.bash udp\n\n" >> ../data/Makefile
printf "../data/tcp_throughput.dat : all_tcp_data\n\t../scripts/collect_throughput.bash tcp\n\n" >> ../data/Makefile
