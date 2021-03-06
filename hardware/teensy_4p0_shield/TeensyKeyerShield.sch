EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 1 3
Title "SofterHardware CW Keyer"
Date "2021-11-09"
Rev "0.91"
Comp "SofterHardware"
Comment1 "KF7O Steve Haynal"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 1900 1700 3100 4250
U 60A4A5BC
F0 "Audio" 50
F1 "audio.sch" 50
$EndSheet
$Sheet
S 5550 1700 3100 4250
U 60B41C2D
F0 "Teensy" 50
F1 "teensy.sch" 50
$EndSheet
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
$Comp
L power:GND #PWR?
U 1 1 61889C96
P 2400 6700
F 0 "#PWR?" H 2400 6450 50  0001 C CNN
F 1 "GND" H 2405 6527 50  0000 C CNN
F 2 "" H 2400 6700 50  0001 C CNN
F 3 "" H 2400 6700 50  0001 C CNN
	1    2400 6700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 6600 2100 6650
Wire Wire Line
	2100 6650 2300 6650
Wire Wire Line
	2700 6650 2700 6600
Wire Wire Line
	2300 6650 2300 6600
Connection ~ 2300 6650
Wire Wire Line
	2300 6650 2400 6650
Wire Wire Line
	2500 6650 2500 6600
Connection ~ 2500 6650
Wire Wire Line
	2500 6650 2700 6650
Wire Wire Line
	2400 6700 2400 6650
Connection ~ 2400 6650
Wire Wire Line
	2400 6650 2500 6650
$EndSCHEMATC
