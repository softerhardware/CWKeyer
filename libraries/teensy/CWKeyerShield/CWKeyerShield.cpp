/* SofterHardwareCWKeyerShield for Teensy 4.X
 * Copyright (c) 2021-2022, kf7o, Steve Haynal, steve@softerhardware.com
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
#include "CWKeyerShield.h"

void CWKeyerShield::setup(void)
{
    AudioMemory(32);
    AudioNoInterrupts();

    if (Pin_SideToneFrequency >= 0) pinMode(Pin_SideToneFrequency, INPUT);
    if (Pin_SideToneVolume    >= 0) pinMode(Pin_SideToneVolume,    INPUT);
    if (Pin_MasterVolume      >= 0) pinMode(Pin_MasterVolume,      INPUT);
    if (Pin_Speed             >= 0) pinMode(Pin_Speed,             INPUT);

    sine.frequency(default_freq);
    sine_level=default_level;
    sine.amplitude(sine_level);

    if (wm8960) {
      wm8960->enable();
      wm8960->volume(0.8F);
    }
    if (sgtl5000) {
      sgtl5000->enable();
      sgtl5000->volume(0.8F);
    }

    AudioInterrupts();

#ifndef DL1YCF_POTS
    analogReadRes(12);
    analogReadAveraging(40);
#endif

}

void CWKeyerShield::loop(void)
{
    if (enable_pots) { pots(); }
    midi();
}


void CWKeyerShield::midi(void)
{
    uint cmd, data;

    //
    // "swallow" incoming MIDI messages on ANY channel,
    // but only process those on MIDI_CONTROL_CHANNEL.
    // This is to prevent overflows if MIDI messages are
    // sent on the "wrong" channel.
    //
    while (usbMIDI.read()) {
        if (usbMIDI.getType() == usbMIDI.ControlChange) {
            cmd  = usbMIDI.getData1();
            data = usbMIDI.getData2();

        Serial.print("MIDI ");
        Serial.print(usbMIDI.getChannel());
        Serial.print(" ");
        Serial.print(midi_rx_ch);
        Serial.print(" ");
        Serial.print(cmd);
        Serial.print(" ");
        Serial.println(data);


        // Accept setting of control channel on any channel
        if (cmd == MIDI_RX_CH) {
            midi_rx_ch = data & 0x0f;
        }

        if (usbMIDI.getChannel() == midi_rx_ch) {
            switch(cmd) {
                case MIDI_SET_A:
                    accum_a = data;
                    break;

                case MIDI_SET_B:
                    accum_b = data;
                    break;

                case MIDI_SET_C:
                    accum_c = data;
                    break;

                case MIDI_MASTER_VOLUME:
                    mastervolume((data << 6)+31);
                    break;

                case MIDI_SIDETONE_VOLUME:
                    sidetonevolume((data << 6)+31);
                    break;

                case MIDI_SIDETONE_FREQUENCY:
                    sidetonefrequency((data << 6)+31);
                    break;

                case MIDI_CW_SPEED:
                    if (data < 1) data=1;
                    speed_set(data);  // report to keyer
                    cwspeed(data);    // report to radio (and MIDI controller)
                    break;

                case MIDI_ENABLE_POTS:
                    enable_pots = (data != 0);
                    break;

                case MIDI_KEYER_AUTOPTT:
                    // auto-PTT by the keyer allowed(data!=0) or disabled (data==0)
                    keyer_autoptt_set(data != 0);  // report to keyer
                    break;

                case MIDI_KEYER_LEADIN:
                    // if keyer auto-PTT: lead-in is (10*data) milliseconds
                    keyer_leadin_set(data); // report to keyer
                    break;

                case MIDI_KEYER_HANG:
                    // if keyer auto-PTT: data=PTT hang time in *dot lengths*
                    keyer_hang_set(data); // report to keyer
                    break;

                case MIDI_MUTE_CWPTT:
                    mute_on_cwptt = (data != 0);
                    break;

                case MIDI_MICPTT_HWPTT:
                    micptt_hwptt = (data != 0);
                    break;

                case MIDI_TX_CH:
                    midi_tx_ch = data & 0x0f;
                    break;

                case MIDI_KEYDOWN_NOTE:
                    midi_keydown_note = data;
                    break;

                case MIDI_PTT_MIC_NOTE:
                    midi_ptt_mic_note = data;
                    break;

                case MIDI_PTT_IN_NOTE:
                    midi_ptt_in_note = data;
                    break;

                case MIDI_CWPTT_NOTE:
                    midi_cwptt_note = data;
                    break;

                case MIDI_SPEED_CTRL:
                    midi_speed_ctrl = data;
                    break;

                case MIDI_FREQ_CTRL:
                    midi_freq_ctrl = data;
                    break;

                case MIDI_RESPONSE:
                    midi_response = data;
                    break;

                case MIDI_WM8960_ENABLE:
                    if (wm8960) {
                        if (data != 0) wm8960->enable();
                        else wm8960->disable();
                    }
                    break;

                case MIDI_WM8960_INPUT_LEVEL:
                    if (wm8960) {
                        float v;
                        v = (float)data/127.0;
                        // accumulator A is right if set, otherwise value is for both
                        if (accum_a >= 0) wm8960->inputLevel( v, (float)accum_a/127.0);
                        else wm8960->inputLevel(v,v);
                    }
                    break;

                case MIDI_WM8960_INPUT_SELECT:
                    if (wm8960) wm8960->inputSelect(data);
                    break;

                case MIDI_WM8960_VOLUME:
                    if (wm8960) {
                        float v;
                        v = (float)data/127.0;
                        // accumulator A is right if set, otherwise value is for both
                        if (accum_a >= 0) wm8960->volume( v, (float)accum_a/127.0);
                        else wm8960->volume(v,v);
                    }
                    break;

                case MIDI_WM8960_HEADPHONE_VOLUME:
                    if (wm8960) {
                        float v;
                        v = (float)data/127.0;
                        // accumulator A is right if set, otherwise value is for both
                        if (accum_a >= 0) wm8960->headphoneVolume( v, (float)accum_a/127.0);
                        else wm8960->headphoneVolume(v,v);
                    }
                    break;

                case MIDI_WM8960_HEADPHONE_POWER:
                    if (wm8960) wm8960->headphonePower(data);
                    break;

                case MIDI_WM8960_SPEAKER_VOLUME:
                    if (wm8960) {
                        float v;
                        v = (float)data/127.0;
                        // accumulator A is right if set, otherwise value is for both
                        if (accum_a >= 0) wm8960->speakerVolume( v, (float)accum_a/127.0);
                        else wm8960->speakerVolume(v,v);
                    }
                    break;

                case MIDI_WM8960_SPEAKER_POWER:
                    if (wm8960) wm8960->speakerPower(data);
                    break;

                case MIDI_WM8960_DISABLE_ADCHPF:
                    if (wm8960) wm8960->disableADCHPF(data);
                    break;

                case MIDI_WM8960_ENABLE_MICBIAS:
                    if (wm8960) wm8960->enableMicBias(data);
                    break;

                case MIDI_WM8960_ENABLE_ALC:
                    if (wm8960) wm8960->enableALC(data);
                    break;

                case MIDI_WM8960_MIC_POWER:
                    if (wm8960) wm8960->micPower(data);
                    break;

                case MIDI_WM8960_LINEIN_POWER:
                    if (wm8960) wm8960->lineinPower(data);
                    break;

                case MIDI_WM8960_RAW_WRITE:
                    if (wm8960 && accum_a >= 0 && accum_b >= 0 && accum_c >= 0) {
                        uint16_t val;
                        uint16_t mask;

                        val = accum_a | ((accum_c & 0b011) << 7);
                        mask = accum_b | ((accum_c & 0b01100) << 5);

                        // reg is never more than 6bits so use data
                        wm8960->write(data, val, mask, (accum_c & 0b010000) != 0);

                    }
                    break;

                default:
                    break;
            }

            if (cmd > MIDI_SET_C) {
                // Reset accumulators
                accum_a = -1;
                accum_b = -1;
                accum_c = -1;
            }
        }
    }
}
}


#ifdef DL1YCF_POTS

void CWKeyerShield::pots() {
    unsigned long now;
    //
    // handle analog lines, but only one analogRead every 5 msec
    // in case of overflow, trigger read.
    // Read all four input lines in round-robin fashion
    //
    now = millis();
    if (now < last_analog_read) last_analog_read = now; // overflow recovery
    if (now > last_analog_read +5) {
      last_analog_read = now;
      switch (last_analog_line++) {
        case 0:  // SideToneFrequency
          if (Pin_SideToneFrequency >= 0) {
            if (analogDenoise(Pin_SideToneFrequency, &Analog_SideFreq, &last_sidefreq)) {
              sidetonefrequency(400+30*last_sidefreq);  // 400 ... 1000 Hz in 30 Hz steps
            }
          }
          break;
        case 1: // SideToneVolume
          if (Pin_SideToneVolume >= 0) {
            if (analogDenoise(Pin_SideToneVolume, &Analog_SideVol, &last_sidevol)) {
              sidetonevolume(last_sidevol);
            }
          }
          break;
        case 2: // Master Volume
          if (Pin_MasterVolume >= 0) {
            if (analogDenoise(Pin_MasterVolume, &Analog_MasterVol, &last_mastervol)) {
              mastervolume(last_mastervol);
            }
          }
          break;
        case 3: // Speed
          if (Pin_Speed >= 0) {
            if (analogDenoise(Pin_Speed, &Analog_Speed, &last_speed)) {
              speed_set(10+last_speed); // report to keyer
              cwspeed(10+last_speed);   // report to radio
            }
          }
          last_analog_line=0;   // roll over
          break;
      }
    }
}

bool CWKeyerShield::analogDenoise(int pin, uint16_t *value, uint8_t *old) {
  //
  // Read analog input from pin #pin, convert value to a scale 0-20.
  // "old" and "value" need to be conserved between calls.
  // "value" is needed for low-passing, and "old" is given the new
  // reading in the range 0-20 if the analog input value has changed.
  // This function returns "true" if the value has changed, and "false"
  // if there is no update.
  //
  // Here comes DL1YCF's "black magic" to de-noise the analog input:
  //
  // The result of analogRead() is low-passed through a moving exponential average.
  // The result of the low-passing is stored in "value" and is in the range
  // (0, 163368) for 10-bit analog reads (16368 = 16*1023).
  //
  // The value is then converted to the scale 0-20 and the new scale value
  // is stored in *old. The conversion is done as follows:
  //
  // value =     0 ...   779   ==> reading =  0
  // value =   780 ...  1559   ==> reading =  1
  // ...
  // value = 15600 ... 16368   ==> reading = 20
  //
  // In this section: VALUE is the low-passed analog reading in the
  // range 0...16386, while READING is the returned value in the range
  // 0..20.
  // While the exponential averaging implements a low-pass filter so we get
  // rid of high-frequency oscillations in VALUE, we have to guard
  // against small-amplitude low-frequency oscillations as well. These can
  // occur if VALUE is close to a multiple of 780 (that is, close to the
  // border of two intervals leading to different READINGs).
  // So we remember the previous READING, compute the mid-point of the
  // interval of VALUEs that possibly lead to this READING, and require that
  // the new VALUE differs from that midpoint by at least 600.
  // This means, if the pot is close to the borderline, we have to move the pot
  // away from this borderline before reporting a new READING.
  //
  // We have to do this, because slowly varying READINGs for the same
  // pot position are quite unpleasant, especially if it affects the
  // side tone frequency.
  //

  uint16_t newval, midpoint;

  if (pin < 0) return false; // paranoia

  newval = (15 * *value) / 16 + analogRead(pin);  // range 0 - 16368
  if (newval > 16368) newval=16368; // pure paranoia
  *value = newval;

  midpoint = 390 + 780 * *old;  // midpoint of interval corresponding to "old" value

  if (newval > midpoint + 600 || (midpoint > 780 && newval < midpoint - 600)) {
    *old = newval / 780; // range 0 ... 20
    return true;
  } else {
    return false;
  }
}

#else

void CWKeyerShield::pots()
{
    uint16_t analog_data;
    unsigned long m = millis();

    // Call every 10 ms, will handle rollover as both m and last_analog_read are unsigned long
    if ((m - last_analog_read) > 10) {

        if (last_analog_line == 0) {
            // Master volume
            analog_data = analogRead(Pin_MasterVolume);
            // At 12 bit this averaging will produce a value in the 0 to 8191 range, 13 bits
            Analog_MasterVol = (Analog_MasterVol >> 1) + analog_data;

            // Volume is 80 values, so only care if more than about 2**7 bit change
            if (abs(Analog_MasterVol - last_mastervol) > 64) {
                mastervolume(8191-Analog_MasterVol);
                last_mastervol = Analog_MasterVol;
                //Serial.print("MV ");
                //Serial.println(analog_data);
            }
        } else if (last_analog_line == 1) {

            // Sidetone volume
            analog_data = analogRead(Pin_SideToneVolume);
            Analog_SideVol = (Analog_SideVol >> 1) + analog_data;
            // Sidetone volume has 32 entries so 5 bits
            if (abs(Analog_SideVol - last_sidevol) > 256) {
                sidetonevolume(8191-Analog_SideVol);
                last_sidevol = Analog_SideVol;
                //Serial.print("SV ");
                //Serial.println(analog_data);
            }
        } else if (last_analog_line == 2) {

            // Sidetone frequency
            analog_data = analogRead(Pin_SideToneFrequency);
            Analog_SideFreq = (Analog_SideFreq >> 1) + analog_data;
            // Range between 250 and 1274, 10 bits
            if (abs(Analog_SideFreq - last_sidefreq) > 64) {
                // Range between 100 and 2147 hz
                sidetonefrequency(8191-Analog_SideFreq);
                last_sidefreq = Analog_SideFreq;
                //Serial.print("SF ");
                //Serial.println(analog_data);
            }
        } else if (last_analog_line == 3) {

            // Speed
            analog_data = analogRead(Pin_Speed);
            Analog_Speed = (Analog_Speed >> 1) + analog_data;
            // range between 1 and 65 WPM via pot, 6 bits
            if (abs(Analog_Speed - last_speed) > 128) {
                uint16_t spd=(8191-Analog_Speed)>>7;
                // never report a "zero speed"
                if (spd < 1) spd=1;
                speed_set(spd); // report to keyer
                cwspeed(spd);   // report to radio
                last_speed = Analog_Speed;
                //Serial.print("SP ");
                //Serial.println(pot_speed);
                //Serial.print(" ");
                //Serial.println(analog_data);
            }
        }

        last_analog_read = m;
        last_analog_line = (last_analog_line + 1) & 0x3;
    }
}

#endif


void CWKeyerShield::key(int state)
{
    //
    // Interface to the keyer. The keyer calls this if it wants
    // to do key-down or key-up
    //
    teensyaudiotone.setTone(state);
    if (midi_keydown_note >= 0 && midi_tx_ch > 0) {
        usbMIDI.sendNoteOn(midi_keydown_note, state ? 127 : 0, midi_tx_ch);
        usbMIDI.send_now();
    }
}

void CWKeyerShield::hwptt(int state)
{
    //
    // Trigger hardware PTT line
    // TODO. HW digital pin not yet known!
    //
}

void CWKeyerShield::midiptt(int state)
{
    //
    // Send MIDI "PTT" event
    //
    if (midi_cwptt_note >= 0 && midi_tx_ch > 0) {
        usbMIDI.sendNoteOn(midi_cwptt_note, state ? 127 : 0, midi_tx_ch);
    }
}

void CWKeyerShield::micptt(int state)
{
    //
    // This function is called if the MICPTT input line state changes
    //
    if (micptt_hwptt) hwptt(state);
    midiptt(state);
}

void CWKeyerShield::cwptt(int state)
{
    //
    // Interface to the keyer. The keyer calls this function
    // if it wants to activate PTT
    //
    if (mute_on_cwptt || state == 0) {
      //
      // This mutes the audio from the PC but not the side tone
      //
      teensyaudiotone.muteAudioIn(state);
    }
    hwptt(state);   // if already done in the keyer, this does no harm
    midiptt(state);
}


// Expected level is 0 to 8191
void CWKeyerShield::mastervolume(uint16_t level)
{
    // Ease reaching max and minimum values
    if (level > 8158) level = 8191;
    if (level < 33) level = 0;

    if (midi_response) {
        usbMIDI.sendControlChange(MIDI_MASTER_VOLUME, (level>>6)&0x7f, midi_rx_ch); // send to MIDI controller
    }
    if (sgtl5000) {
        sgtl5000->volume(((float)level)/8191.0);
    }
    if (wm8960) {
        wm8960->volume(((float)level)/8191.0);
    }
}

// Expected level is 0 to 8191
void CWKeyerShield::sidetonevolume(uint16_t level)
{
    //
    // The input value (level) is in the range 0-31 and converted to
    // an amplitude using VolTab, such that a logarithmic pot is
    // simulated.
    //
    if (midi_response) {
        usbMIDI.sendControlChange(MIDI_SIDETONE_VOLUME, (level>>6)&0x7f, midi_rx_ch); // send to MIDI controller
    }
    // Reduce to 5 bits
    level = (level >> 8) & 0x1f;
    sine.amplitude(VolTab[level]);
}

// Expected input value is 0 to 8191, maps to range 250 to about 1270Hz
void CWKeyerShield::sidetonefrequency(uint16_t freq)
{
    sine.frequency( 250+((float)freq)/8.0 );

    if (midi_freq_ctrl >= 0 && midi_tx_ch > 0) {
        //
        // In the radio, the frequency has to be calculated as
        // freq = 250 + 8*val, where val is the controller value 0<= val <= 127
        //
        usbMIDI.sendControlChange(midi_freq_ctrl, (freq>>6)&0x7f, midi_tx_ch); // send  to radio
    }
    if (midi_response) {
        usbMIDI.sendControlChange(MIDI_SIDETONE_FREQUENCY, (freq>>6)&0x7f, midi_rx_ch); // send to MIDI controller
    }
}

// Expected speed is 1 to 127, don't use high values if not desired, no fractional CW speeds
void CWKeyerShield::cwspeed(uint16_t speed)
{
    if (speed == 0) speed = 1;
    if (speed > 127) speed = 127;

    if (midi_speed_ctrl >= 0 && midi_tx_ch > 0) {
        //
        // In the radio, the controller value (1 <= val <= 127) encodes the speed
        //
        usbMIDI.sendControlChange(midi_speed_ctrl, speed&0x7f, midi_tx_ch);  // send to radio
    }
    if (midi_response) {
        usbMIDI.sendControlChange(MIDI_CW_SPEED, speed&0x7f, midi_rx_ch);    // send to MIDI controller
    }
 }



#endif
