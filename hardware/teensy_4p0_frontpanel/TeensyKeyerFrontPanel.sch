EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 1 1
Title "SofterHardware CW Keyer Front Panel"
Date "2021-11-07"
Rev "0.9"
Comp "SofterHardware"
Comment1 "KF7O Steve Haynal"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector:TestPoint_Alt MH1
U 1 1 616C9E01
P 2100 6600
F 0 "MH1" H 2000 6800 50  0000 L CNN
F 1 "TestPoint_Alt" H 2158 6627 50  0001 L CNN
F 2 "TeensyKeyerShield:MH" H 2300 6600 50  0001 C CNN
F 3 "~" H 2300 6600 50  0001 C CNN
	1    2100 6600
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint_Alt MH2
U 1 1 616CA163
P 2300 6600
F 0 "MH2" H 2200 6800 50  0000 L CNN
F 1 "TestPoint_Alt" H 2358 6627 50  0001 L CNN
F 2 "TeensyKeyerShield:MH" H 2500 6600 50  0001 C CNN
F 3 "~" H 2500 6600 50  0001 C CNN
	1    2300 6600
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint_Alt MH3
U 1 1 616CA233
P 2500 6600
F 0 "MH3" H 2400 6800 50  0000 L CNN
F 1 "TestPoint_Alt" H 2558 6627 50  0001 L CNN
F 2 "TeensyKeyerShield:MH" H 2700 6600 50  0001 C CNN
F 3 "~" H 2700 6600 50  0001 C CNN
	1    2500 6600
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint_Alt MH4
U 1 1 616CA2DD
P 2700 6600
F 0 "MH4" H 2600 6800 50  0000 L CNN
F 1 "TestPoint_Alt" H 2758 6627 50  0001 L CNN
F 2 "TeensyKeyerShield:MH" H 2900 6600 50  0001 C CNN
F 3 "~" H 2900 6600 50  0001 C CNN
	1    2700 6600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 6600 2300 6600
Wire Wire Line
	2300 6600 2400 6600
Connection ~ 2300 6600
Wire Wire Line
	2500 6600 2700 6600
Connection ~ 2500 6600
$Comp
L power:GND #PWR0101
U 1 1 61888624
P 2400 6650
F 0 "#PWR0101" H 2400 6400 50  0001 C CNN
F 1 "GND" H 2405 6477 50  0000 C CNN
F 2 "" H 2400 6650 50  0001 C CNN
F 3 "" H 2400 6650 50  0001 C CNN
	1    2400 6650
	1    0    0    -1  
$EndComp
Wire Wire Line
	2400 6650 2400 6600
Connection ~ 2400 6600
Wire Wire Line
	2400 6600 2500 6600
$EndSCHEMATC
