#!/bin/bash
# expects transport protocol name as parameter #1

if [[ $# != 1 ]] ; then printf "\nError: protocol name expected as a parameter\n\n" ; exit 1; fi

#/bin/rm -f ../data/$1_throughput.dat ../data/$1_unsorted.dat 

declare -a Tstring
declare -a Sstring
declare -a Dstring

for fname in ../data/$1_*.out ; do
	Tstring=(`grep Throughput $fname`)
	Sstring=(`grep repetitions $fname | grep Ping`)
	Dstring=(`grep RTT $fname | grep average`)
  	echo ${Sstring[7]} ${Tstring[8]} $( echo "scale=8;0.5*${Dstring[11]}" |bc )>> ../data/$1_unsorted.dat
done
sort -n --key=1,1 ../data/$1_unsorted.dat > ../data/$1_throughput.dat

fname=../data/$1_32.out
Rtt=(`grep Round $fname | grep repetition`)
var=0
echo $var> ../data/$1_var.txt
for (( ind=0 ; $ind<909 ; ind=$ind+9 )) do
	ival=$ind+4
	irip=$ind+8
	echo ${Rtt[$irip]} ${Rtt[$ival]}>> ../data/$1_unsorted2.dat
done
sort -n --key=1,1 ../data/$1_unsorted2.dat > ../data/$1_sample32.dat

if [ "$1" = "tcp" ]; then
	fname=../data/$1_262144.out
	Rtt=(`grep Round $fname | grep repetition`)
	var=0
	echo $var> ../data/$1_var1.txt
	for (( ind=0 ; $ind<909 ; ind=$ind+9 )) do
		ival=$ind+4
		irip=$ind+8
		echo ${Rtt[$irip]} ${Rtt[$ival]}>> ../data/$1_unsorted3.dat
	done
	sort -n --key=1,1 ../data/$1_unsorted3.dat > ../data/$1_sample262144.dat
else
	fname=../data/$1_16384.out
	Rtt=(`grep Round $fname | grep repetition`)
	var=0
	echo $var> ../data/$1_var2.txt
	for (( ind=0 ; $ind<909 ; ind=$ind+9 )) do
		ival=$ind+4
		irip=$ind+8
		echo ${Rtt[$irip]} ${Rtt[$ival]}>> ../data/$1_unsorted4.dat
	done
	sort -n --key=1,1 ../data/$1_unsorted4.dat > ../data/$1_sample16384.dat
fi

/bin/rm -f ../data/$1_unsorted.dat ../data/$1_unsorted2.dat ../data/$1_unsorted3.dat ../data/$1_unsorted4.dat

fname=../data/$1_sample32.dat
Rtt=(`grep 1 $fname`)
echo ${Rtt[1]}> ../data/$1_stimato.txt
for (( ind=1 ; $ind<=101 ; ind=$ind+1 )) do
	estimated=(`cat ../data/$1_stimato.txt`)
	echo $ind $estimated>> ../data/$1_estimated32.dat
	Rtt=(`grep $ind $fname`)
	sample=${Rtt[1]}
	var=(`cat ../data/$1_var.txt`)
	echo $ind $(echo "scale=8;(0.25*(sqrt(($sample-$estimated)*($sample-$estimated))))+0.75*$var" |bc)>> ../data/$1_variability32.dat
echo "32\n%s\n" $(echo "scale=8;(0.875*$estimated)+(0.125*$sample)" | bc)>> ../data/$1_variab.dat
	echo $(echo "scale=8;(0.25*(sqrt(($sample-$estimated)*($sample-$estimated))))+0.75*$var" |bc)> ../data/$1_var.txt

	echo $(echo "scale=8;(0.875*$estimated)+(0.125*$sample)" | bc)> ../data/$1_stimato.txt
done


if [ "$1" = "tcp" ]; then
fname=../data/$1_sample262144.dat
Rtt=(`grep 1 $fname`)
echo ${Rtt[1]}> ../data/$1_stimato.txt
for (( ind=1 ; $ind<=101 ; ind=$ind+1 )) do
	estimated=(`cat ../data/$1_stimato.txt`)
	echo $ind $estimated>> ../data/$1_estimated262144.dat
	Rtt=(`grep $ind $fname`)
	sample=${Rtt[1]}
	var=(`cat ../data/$1_var1.txt`)
	echo $ind $(echo "scale=8;(0.25*(sqrt(($sample-$estimated)*($sample-$estimated))))+0.75*$var" |bc)>> ../data/$1_variability262144.dat
echo "TCP 262144\n%s\n" $(echo "scale=8;(0.875*$estimated)+(0.125*$sample)" | bc)>> ../data/$1_variab.dat
	echo $(echo "scale=8;(0.25*(sqrt(($sample-$estimated)*($sample-$estimated))))+0.75*$var" |bc)> ../data/$1_var1.txt

	echo $(echo "scale=8;(0.875*$estimated)+(0.125*$sample)" | bc)> ../data/$1_stimato.txt
done
else
fname=../data/$1_sample16384.dat
Rtt=(`grep 1 $fname`)
echo ${Rtt[1]}> ../data/$1_stimato.txt
for (( ind=1 ; $ind<=101 ; ind=$ind+1 )) do
	estimated=(`cat ../data/$1_stimato.txt`)
	echo $ind $estimated>> ../data/$1_estimated16384.dat
	Rtt=(`grep $ind $fname`)
	sample=${Rtt[1]}
	var=(`cat ../data/$1_var2.txt`)
	echo $ind $(echo "scale=8;(0.25*(sqrt(($sample-$estimated)*($sample-$estimated))))+0.75*$var" |bc)>> ../data/$1_variability16384.dat
echo "UDP 16384\n%s\n" $(echo "scale=8;(0.875*$estimated)+(0.125*$sample)" | bc)>> ../data/$1_variab.dat
	echo $(echo "scale=8;(0.25*(sqrt(($sample-$estimated)*($sample-$estimated))))+0.75*$var" |bc)> ../data/$1_var2.txt

	echo $(echo "scale=8;(0.875*$estimated)+(0.125*$sample)" | bc)> ../data/$1_stimato.txt
done
fi
/bin/rm -f ../data/$1_stimato.txt ../data/$1_var.txt ../data/$1_var1.txt ../data/$1_var2.txt

	fname=../data/$1_throughput.dat
	if [ "$1" = "tcp" ]; then
		Tstring=(`grep 32 $fname`)	#N1
		Sstring=(`grep 262144 $fname`)	#N2
	else
		Tstring=(`grep 32 $fname`)
		Sstring=(`grep 16384 $fname`)	
	fi

	echo $(echo "scale=8;(${Sstring[0]}-${Tstring[0]})/(${Sstring[2]}-${Tstring[2]})" | bc)> ../data/$1_B.txt
	B=(`cat ../data/$1_B.txt`)
	echo $(echo "scale=8;((${Tstring[2]}*${Sstring[0]})-(${Sstring[2]}*${Tstring[0]}))/(${Sstring[0]}-${Tstring[0]})" | bc)> ../data/$1_L0.txt
	L0=(`cat ../data/$1_L0.txt`)
	if [ "$1" = "tcp" ]; then
		und=524288
	else
		und=16384	
	fi
	for (( indx=32 ; $indx<=$und ; indx=$indx+$indx )) do
		echo $indx $(echo "scale=8;($indx*$B)/(($B*$L0)+$indx)" | bc)>> ../data/$1_modelloBL.dat
	done
