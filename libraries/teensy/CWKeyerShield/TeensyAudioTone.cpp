/* TeensyKeyer for Teensy 4.X
 * Copyright (c) 2021, kf7o, Steve Haynal, steve@softerhardware.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//
// This ifndef allows this module being compiled on an Arduino etc.,
// although it offers no function there. Why do we do this? If this
// module is in the src directory, the Arduino IDE will compile it even
// if it is not used.
//
#ifndef __AVR__

#include <Arduino.h>
#include "TeensyAudioTone.h"
#include "utility/dspinst.h"

#define WINDOW_TABLE_LENGTH 128


#if 0
//
// This "ramp" is the raised cosine function itself.
//
// Mathematica code to generate it:
//
// RC[x_] = 1/2 (1 + Cos[2 Pi x + Pi])
// Tab = Table[Round[2^31  RC[i/258]], {i, 1, 128}];
// Do[{l = Table[PaddedForm[Tab[[j]], 12], {j, 8*i + 1, 8*i + 8}], Print[Row[l, ","], ","]}, {i, 0, 15}]
//
// SciPy code to generate it:
//
//import numpy as np
//import scipy.signal
//a = np.round((2**31)*scipy.signal.windows.hann(259))
//for i in range(1,129):
//  if (i-1)%8 == 0: print()
//  print("{:10d}".format(int(a[i])),end=", ")
const int32_t window_table[WINDOW_TABLE_LENGTH] = {
    318397,    1273398,    2864439,    5090574,    7950483,   11442471,   15564467,   20314026,
  25688331,   31684195,   38298062,   45526009,   53363751,   61806638,   70849664,   80487465,
  90714326,  101524182,  112910621,  124866892,  137385902,  150460228,  164082115,  178243486,
 192935941,  208150767,  223878941,  240111135,  256837722,  274048782,  291734109,  309883213,
 328485332,  347529432,  367004221,  386898147,  407199413,  427895979,  448975571,  470425686,
 492233605,  514386393,  536870912,  559673828,  582781618,  606180576,  629856827,  653796328,
 677984882,  702408144,  727051629,  751900723,  776940687,  802156673,  827533725,  853056793,
 878710740,  904480353,  930350348,  956305383,  982330065, 1008408960, 1034526600, 1060667498,
1086816150, 1112957048, 1139074688, 1165153583, 1191178265, 1217133300, 1243003295, 1268772908,
1294426855, 1319949923, 1345326975, 1370542961, 1395582925, 1420432019, 1445075504, 1469498766,
1493687320, 1517626821, 1541303072, 1564702030, 1587809820, 1610612736, 1633097255, 1655250043,
1677057962, 1698508077, 1719587669, 1740284235, 1760585501, 1780479427, 1799954216, 1818998316,
1837600435, 1855749539, 1873434866, 1890645926, 1907372513, 1923604707, 1939332881, 1954547707,
1969240162, 1983401533, 1997023420, 2010097746, 2022616756, 2034573027, 2045959466, 2056769322,
2066996183, 2076633984, 2085677010, 2094119897, 2101957639, 2109185586, 2115799453, 2121795317,
2127169622, 2131919181, 2136041177, 2139533165, 2142393074, 2144619209, 2146210250, 2147165251,
};
#endif

#if 0
//
// Step function from a RaisedCosine window. Mathematica code:
//
// RC[x_] = 1/2 (1 + Cos[2 Pi x + Pi])
// RCI[y_] = Integrate[RC[x], {x, 0, y}]
// Tab = Table[Round[2^31  RCI[i/129]/RCI[1]], {i, 1, 128}];
// Do[{l = Table[PaddedForm[Tab[[j]], 12], {j, 8*i + 1, 8*i + 8}], Print[Row[l, ","], ","]}, {i, 0, 15}]
//
const int32_t window_table[WINDOW_TABLE_LENGTH] = {
         6581,        52632,       177529,       420460,       820335,      1415690,      2244599,      3344580,
      4752510,      6504535,      8635984,     11181287,     14173892,     17646185,     21629417,     26153625,
     31247563,     36938634,     43252825,     50214644,     57847064,     66171467,     75207593,     84973495,
     95485495,    106758145,    118804192,    131634549,    145258270,    159682527,    174912590,    190951822,
    207801665,    225461638,    243929338,    263200447,    283268743,    304126109,    325762560,    348166263,
    371323562,    395219017,    419835436,    445153916,    471153890,    497813174,    525108021,    553013175,
    581501933,    610546210,    640116599,    670182449,    700711932,    731672120,    763029066,    794747877,
    826792808,    859127335,    891714249,    924515745,    957493504,    990608794,   1023822553,   1057095487,
   1090388161,   1123661095,   1156874854,   1189990144,   1222967903,   1255769399,   1288356313,   1320690840,
   1352735771,   1384454582,   1415811528,   1446771716,   1477301199,   1507367049,   1536937438,   1565981715,
   1594470473,   1622375627,   1649670474,   1676329758,   1702329732,   1727648212,   1752264631,   1776160086,
   1799317385,   1821721088,   1843357539,   1864214905,   1884283201,   1903554310,   1922022010,   1939681983,
   1956531826,   1972571058,   1987801121,   2002225378,   2015849099,   2028679456,   2040725503,   2051998153,
   2062510153,   2072276055,   2081312181,   2089636584,   2097269004,   2104230823,   2110545014,   2116236085,
   2121330023,   2125854231,   2129837463,   2133309756,   2136302361,   2138847664,   2140979113,   2142731138,
   2144139068,   2145239049,   2146067958,   2146663313,   2147063188,   2147306119,   2147431016,   2147477067
};
#endif

// Blackman-Harris Window. Mathematica code.
//
// a0 = 0.3587500000000000000000000000000000000000000000000000;
// a1 = 0.4882900000000000000000000000000000000000000000000000;
// a2 = 0.1412800000000000000000000000000000000000000000000000;
// a3 = 0.0116800000000000000000000000000000000000000000000000;
//
// BH[x_] = a0 - a1 Cos[2 Pi x] + a2 Cos[4 Pi x] - a3 Cos[6 Pi x];
// BHIHI[y_] = Integrate[BH[x], {x, 0, y}]
// Tab = Table[Round[2^31 BHI[i/129]/BHI[1]], {i, 1, 128}];
// Do[{l = Table[PaddedForm[Tab[[j]], 12], {j, 8*i + 1, 8*i + 8}], Print[Row[l, ","], ","]}, {i, 0, 15}]
//
const int32_t window_table[WINDOW_TABLE_LENGTH] = {
         3305,         9778,        22804,        46197,        84420,       142798,       227731,       346909,
       509525,       726474,      1010566,      1376713,      1842122,      2426465,      3152045,      4043938,
      5130118,      6441557,      8012296,      9879493,     12083427,     14667468,     17678014,     21164373,
     25178607,     29775326,     35011439,     40945845,     47639087,     55152951,     63550020,     72893188,
     83245126,     94667721,    107221476,    120964892,    135953824,    152240832,    169874519,    188898887,
    209352690,    231268820,    254673713,    279586805,    306020025,    333977346,    363454400,    394438165,
    426906721,    460829093,    496165178,    532865761,    570872623,    610118743,    650528584,    692018482,
    734497114,    777866051,    822020396,    866849483,    912237655,    958065083,   1004208652,   1050542864,
   1096940784,   1143274996,   1189418565,   1235245993,   1280634165,   1325463252,   1369617597,   1412986534,
   1455465166,   1496955064,   1537364905,   1576611025,   1614617887,   1651318470,   1686654555,   1720576927,
   1753045483,   1784029248,   1813506302,   1841463623,   1867896843,   1892809935,   1916214828,   1938130958,
   1958584761,   1977609129,   1995242816,   2011529824,   2026518756,   2040262172,   2052815927,   2064238522,
   2074590460,   2083933628,   2092330697,   2099844561,   2106537803,   2112472209,   2117708322,   2122305041,
   2126319275,   2129805634,   2132816180,   2135400221,   2137604155,   2139471352,   2141042091,   2142353530,
   2143439710,   2144331603,   2145057183,   2145641526,   2146106935,   2146473082,   2146757174,   2146974123,
   2147136739,   2147255917,   2147340850,   2147399228,   2147437451,   2147460844,   2147473870,   2147480343
};

void TeensyAudioTone::update(void)
{
    audio_block_t *block_sine, *block_inl, *block_inr;
    audio_block_t *block_sidetone;
    int16_t i, t;


    block_inl  = receiveReadOnly(0);
    block_inr  = receiveReadOnly(1);
    block_sine = receiveReadOnly(2);

    //
    // Use block_sidetone as a "flag" for "playing side tone"
    // This guarantees that we do not "hang" in the "window_index > 0" state
    // if allocation of block_sidetone constantly fails.
    //
    block_sidetone=NULL;
    if ((tone || windowindex || mute) && block_sine && sidetone_enabled) {
      block_sidetone=allocate();
    }

    if (block_sidetone){
        if (tone) {
            // Apply ramp up window and/or send tone to both outputs
            for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
                if (windowindex < WINDOW_TABLE_LENGTH) {
                    t = multiply_32x32_rshift32(block_sine->data[i] << 1, window_table[windowindex++]);
                } else {
                    t = block_sine->data[i];
                }
                block_sidetone->data[i]=t;
            }
        } else if (windowindex) {
            // Apply ramp down until 0 window index
            if (windowindex > WINDOW_TABLE_LENGTH) windowindex = WINDOW_TABLE_LENGTH;
            for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
                if (windowindex) {
                    t = multiply_32x32_rshift32(block_sine->data[i] << 1, window_table[--windowindex]);
                } else {
                    t = 0;
                }
                block_sidetone->data[i] = t;
            }
        } else {
          //
          // mute set but not within a "window": send silence
      //
          // NOTE: in the very un-probable case that tone == windowindex == mute == 0,
          //       (since there was a spike on "tone")
          //       we also arrive here and send one block of silence
          //
          for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
            block_sidetone->data[i] = 0;
          }
        }
        // Use same data for both ears
        transmit(block_sidetone,0);
        transmit(block_sidetone,1);
        release(block_sidetone);
    } else {

        windowindex = tone = mute = 0;  // just in case we arrive here because of a failed allocation
        if (block_inl) transmit(block_inl,0);
        if (block_inr) transmit(block_inr,1);
    }

    if (block_sine) release(block_sine);
    if (block_inl)  release(block_inl);
    if (block_inr)  release(block_inr);

}


#undef WINDOW_TABLE_LENGTH
#endif
