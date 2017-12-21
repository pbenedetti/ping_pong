#!/bin/bash

gnuplot <<-eNDgNUPLOTcOMMAND
	set term png size 1000, 800
	set output "../data/throughput_tcp.png"
	set logscale x 2
	set xlabel "msg size (B)"
	set ylabel "throughput (KB/s)"
	plot "../data/tcp_throughput.dat" using 1:2 title "TCP msg=32 Throughput" \
			with linespoints, \
		"../data/tcp_modelloBL.dat" using 1:2 title "TCP msg=32 Modello Banda Latenza" \
		with linespoints
	clear

	set term png size 1000, 800
	set output "../data/throughput_udp.png"
	set logscale x 2
	set xlabel "msg size (B)"
	set ylabel "throughput (KB/s)"
	plot "../data/udp_throughput.dat" using 1:2 title "UDP msg=32 Throughput" \
			with linespoints, \
		"../data/udp_modelloBL.dat" using 1:2 title "UDP msg=32 Modello Banda Latenza" \
		with linespoints
	clear

	set term png size 1000, 800
	set output "../data/RTT_32_tcp.png"
	set logscale x 2
	set xlabel "Repetitions"
	set ylabel "RTT"
	plot "../data/tcp_sample32.dat" using 1:2 title "TCP msg=32 Sample_RTT" \
			with linespoints, \
		"../data/tcp_estimated32.dat" using 1:2 title "TCP msg=32 Estimated_RTT" \
			with linespoints, \
		"../data/tcp_variability32.dat" using 1:2 title "TCP msg=32 Variability" \
		with linespoints
	clear

	set term png size 1000, 800
	set output "../data/RTT_262144_tcp.png"
	set logscale x 2
	set xlabel "Repetitions"
	set ylabel "RTT"
	plot "../data/tcp_sample262144.dat" using 1:2 title "TCP msg=262144 Sample_RTT" \
			with linespoints, \
		"../data/tcp_estimated262144.dat" using 1:2 title "TCP msg=262144 Estimated_RTT" \
			with linespoints, \
		"../data/tcp_variability262144.dat" using 1:2 title "TCP msg=262144 Variability" \
		with linespoints
	clear

	set term png size 1000, 800
	set output "../data/RTT_32_udp.png"
	set logscale x 2
	set xlabel "Repetitions"
	set ylabel "RTT"
	plot "../data/udp_sample32.dat" using 1:2 title "UDP msg=32 Sample_RTT" \
			with linespoints, \
		"../data/udp_estimated32.dat" using 1:2 title "UDP msg=32 Estimated_RTT" \
			with linespoints, \
		"../data/udp_variability32.dat" using 1:2 title "UDP msg=32 Variability" \
		with linespoints
	clear

	set term png size 1000, 800
	set output "../data/RTT_16384_udp.png"
	set logscale x 2
	set xlabel "Repetitions"
	set ylabel "RTT"
	plot "../data/udp_sample16384.dat" using 1:2 title "UDP msg=16384 Sample_RTT" \
			with linespoints, \
		"../data/udp_estimated16384.dat" using 1:2 title "UDP msg=16384 Estimated_RTT" \
			with linespoints, \
		"../data/udp_variability16384.dat" using 1:2 title "UDP msg=16384 Variability" \
		with linespoints
	clear

eNDgNUPLOTcOMMAND
