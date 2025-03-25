/* -*- mode: c++; c-basic-offset: 4 -*- */
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

    if (Pin_PTTin             >= 0) pinMode(Pin_PTTin,             INPUT_PULLUP);
    if (Pin_PTTout            >= 0) {
      pinMode(Pin_PTTout,            OUTPUT);
      digitalWrite(Pin_PTTout, 0);
    }
    if (Pin_CWout             >= 0) {
      pinMode(Pin_CWout,             OUTPUT);
      digitalWrite(Pin_CWout, 0);
    }

    //
    // The following settings will probably very soon be overwritten,
    // but let us start with some 'safe' values
    //

    sine.frequency(800.0F);
    sidetonelevel_target=0.4F;
    sidetonelevel_actual=0.4F;
    sine.amplitude(sidetonelevel_actual);

    masterlevel_actual=0.8F;
    masterlevel_target=0.8F;
    if (wm8960) {
      wm8960->enable();
      wm8960->volume(masterlevel_actual);
      wm8960->inputSelect(0);               // 0 = Mic, 1 = LineIn
      wm8960->enableMicBias(1);
      //
      // DL1YCF comment start
      // ====================
      //
      // Note the level scale is logarithmic. For the mic input jack (left channel)
      // I have measured the input voltage corresponding to full scale input
      // (beyond that, clipping sets in)
      //
      //   Level       Vpp (mV)    Vrms(mV)  dBV
      //   -------------------------------------
      //    0.0         850         300      - 9
      //    0.3         160          57      -25
      //    0.4          95          34      -29
      //    0.5          50          18      -35
      //    0.6          31          11      -39
      //    0.7          17           6      -44
      //
      // so the empirical formula for a full-scale signal (in dBV) is
      // dBV = -10 -50*level
      //
      // Typical input levels are
      //
      // Dynamic Microphone : Vpp =    5 mV,  -55 dBV  ==> Level = 0.9
      // Electret Microphone: Vpp =   50 mV,  -35 dBV  ==> Level = 0.5
      // Line level:          Vpp =  900 mV   -10 dBV  ==> Level = 0.0
      //
      // so with a value of the input level between 0.0 and 1.0, one can cover the whole
      // range from dynamic microphones to line levels (these may occur if using an
      // external microphone pre-amp). A resonable default seems to be 0.5, but if you
      // either connect a dynamic microphone or a line level source you either need
      // to re-compile your KeyerShield firmware or use MIDI comands top re-adjust the level.
      //
      // Whereas the microphone jack only connects to the left audio channel, the
      // signal of the built-in MEMS microphone goes to the right audio channel. My
      // preliminary experiments indicate that a "level" value of about 0.6 is just fine
      // here, since I do not expect people are holding the KeyerShield in their hands
      // and place it just before their mouth!
      //
      // An API in which one can switch between MEMS and MIC such that the chosen signal
      // (occurs on both channels) my be preferred, but then you need to adapt
      // control_wm8960.cpp in the Audio library. This applies in particular when using
      // the Mic input signal, in this case one certainly does not want MEMS signals.
      //
      // Perhaps one should implement a possibility to "switch" between the left and right
      // channel in the sense that the "chosen" signal occurs in both channels.
      //
      // DL1YCF comment end
      // ==================
      //
      wm8960->inputLevel(0.5F, 0.6F);     // volume control for mic input (both mic and MEMS)
    }
    if (sgtl5000) {
      sgtl5000->enable();
      sgtl5000->volume(masterlevel_actual);
      // Note that this sets the Mic Bias voltage to 3.0 Volt and the Mic Bias
      // output impedance to 2 kOhm, and this is "hard-wired" into control_sgtl5000
      // in the audio library.
      sgtl5000->inputSelect(AUDIO_INPUT_MIC);
      // The default microphone setting is 52 dB (40 dB preamp and 12 dB line-gain),
      // the correct value depends on the microphone but here we use some 12 dB less
      sgtl5000->micGain(40);
    }

    AudioInterrupts();

    analogReadRes(12);
    analogReadAveraging(40);

}

void CWKeyerShield::loop(void)
{
    if (enable_pots) { pots(); }
    monitor_ptt();
    midi();
    adjust();
}

void CWKeyerShield::adjust(void)
{
    //
    // There were audible cracks in the side tone if the side
    // tone volume was changed. Therefore we only define
    // the target value when setting the sidetone or the master
    // volume, and here we slowly approach this value
    //
    unsigned long now=millis();
    int update;
    if (now > last_adjust) {
      last_adjust=now;

      update=0;
      if (sidetonelevel_actual < sidetonelevel_target - 0.01F) {
        sidetonelevel_actual += 0.001F;
        update=1;
      } else if (sidetonelevel_actual < sidetonelevel_target - 0.001F) {
        sidetonelevel_actual += 0.0005F;
        update=1;
      } else if (sidetonelevel_actual > sidetonelevel_target + 0.01F) {
        sidetonelevel_actual -= 0.001F;
        update=1;
      } else if (sidetonelevel_actual > sidetonelevel_target + 0.001F) {
        sidetonelevel_actual -= 0.0005F;
        update=1;
      }
      if (update) sine.amplitude(sidetonelevel_actual);

      //
      // Note that depending on the "granularity" of volume
      // control in the hardware, this may offer little
      // improvement
      //
      update=0;
      if (masterlevel_actual < masterlevel_target - 0.01F) {
        masterlevel_actual += 0.001F;
        update=1;
      } else if (masterlevel_actual < masterlevel_target - 0.001F) {
        masterlevel_actual += 0.0005F;
        update=1;
      } else if (masterlevel_actual > masterlevel_target + 0.01F) {
        masterlevel_actual -= 0.001F;
        update=1;
      } else if (masterlevel_actual > masterlevel_target + 0.001F) {
        masterlevel_actual -= 0.0005F;
        update=1;
      } 
      if (update) {
        if (sgtl5000) {
          sgtl5000->volume(masterlevel_actual);
        }
        if (wm8960) {
          wm8960->volume(masterlevel_actual);
        }
      }
    }
}

void CWKeyerShield::monitor_ptt(void)
{
    //
    // do a de-bouncing read of the ptt digital input
    // combine this with the cwptt flag
    // if PTT status changed, send PTT MIDI message
    // and toggle status of hardware PTT out
    //
    unsigned long now=millis();
    int val;

    if (now > (last_ptt_read + 10) && (Pin_PTTin >= 0)) {
      val=!digitalRead(Pin_PTTin);         // input is active-low
      if (val != last_ptt_in) {
        last_ptt_in=val;
        last_ptt_read = now;               // used for debouncing
      }
    }
    val  = (last_ptt_in | cwptt_state) ? 1 : 0;
    if (val != midiptt_state) {
      midiptt_state = val;
      midiptt(val);
    }
    //
    // if micptt_hwptt is not set, this suppresses signalling
    // the PTT-in state to the PTT-out line, but MIDI PTT
    // reporting should still occur.
    //
    // if cwptt_hwptt is not set, this suppresses signalling
    // the cwptt state to the PTT-out line, but MIDI PTT
    // reporting should still occur
    //
    //
    val = ((last_ptt_in & micptt_hwptt) | (cwptt_state & cwptt_hwptt)) ? 1 : 0;
    if (val != hwptt_state) {
      hwptt_state = val;
      hwptt(val);
    }
}


void CWKeyerShield::process_nrpn(const int16_t nrpn_cc, const int16_t nrpn_val)
{
    switch(nrpn_cc) {

    case MIDI_NRPN_WM8960_ENABLE:
        if (wm8960) {
            if (nrpn_val != 0) wm8960->enable();
            else wm8960->disable();
        }
        break;

    case MIDI_NRPN_WM8960_INPUT_SELECT:
        if (wm8960) wm8960->inputSelect(nrpn_val);
        break;

    case MIDI_NRPN_WM8960_INPUT_LEVEL:
        if (wm8960) {
            float l, r;
            l = (float)(((nrpn_val>>7)&0x7f)/127.0);
            r = (float)((nrpn_val&0x7f)/127.0);
            wm8960->inputLevel(l,r);
        }
        break;

    case MIDI_NRPN_WM8960_VOLUME:
        if (wm8960) {
            float l, r;
            l = (float)(((nrpn_val>>7)&0x7f)/127.0);
            r = (float)((nrpn_val&0x7f)/127.0);
            wm8960->volume(l,r);
        }
        break;

    case MIDI_NRPN_WM8960_HEADPHONE_VOLUME:
        if (wm8960) {
            float l, r;
            l = (float)(((nrpn_val>>7)&0x7f)/127.0);
            r = (float)((nrpn_val&0x7f)/127.0);
            wm8960->headphoneVolume(l,r);
        }
        break;

    case MIDI_NRPN_WM8960_HEADPHONE_POWER:
        if (wm8960) wm8960->headphonePower(nrpn_val);
        break;

    case MIDI_NRPN_WM8960_SPEAKER_VOLUME:
        if (wm8960) {
            float l, r;
            l = (float)(((nrpn_val>>7)&0x7f)/127.0);
            r = (float)((nrpn_val&0x7f)/127.0);
            wm8960->speakerVolume(l,r);
        }
        break;

    case MIDI_NRPN_WM8960_SPEAKER_POWER:
        if (wm8960) wm8960->speakerPower(nrpn_val);
        break;

    case MIDI_NRPN_WM8960_DISABLE_ADCHPF:
        if (wm8960) wm8960->disableADCHPF(nrpn_val);
        break;

    case MIDI_NRPN_WM8960_ENABLE_MICBIAS:
        if (wm8960) wm8960->enableMicBias(nrpn_val);
        break;

    case MIDI_NRPN_WM8960_ENABLE_ALC:
        if (wm8960) wm8960->enableALC(nrpn_val);
        break;

    case MIDI_NRPN_WM8960_MIC_POWER:
        if (wm8960) wm8960->micPower(nrpn_val);
        break;

    case MIDI_NRPN_WM8960_LINEIN_POWER:
        if (wm8960) wm8960->lineinPower(nrpn_val);
        break;

    case MIDI_NRPN_WM8960_RAW_MASK:
        wm8960_raw_mask = nrpn_val & 0x1ff;
        break;

    case MIDI_NRPN_WM8960_RAW_DATA:
        wm8960_raw_data = nrpn_val & 0x1ff;
        break;

    case MIDI_NRPN_WM8960_RAW_WRITE:
        if (wm8960 && wm8960_raw_mask >= 0 && wm8960_raw_data >= 0) {
            wm8960->write(nrpn_val & 0x3f, wm8960_raw_data, wm8960_raw_mask, (nrpn_val & 0x40) != 0);
        }
        wm8960_raw_mask = -1;
        wm8960_raw_data = -1;
        break;

    case MIDI_NRPN_KEYDOWN_NOTE:
        // Use 128-255 to turn note off
        midi_keydown_note = nrpn_val & 0x0ff;
        break;

    case MIDI_NRPN_PTT_NOTE:
        // Use 128-255 to turn note off
        midi_ptt_note = nrpn_val & 0x0ff;
        break;


    default:
        break;

    }
}


void CWKeyerShield::midi(void)
{
    uint8_t data1, data2;

    //
    // "swallow" incoming MIDI messages on ANY channel,
    // but only process those on MIDI_CONTROL_CHANNEL.
    // This is to prevent overflows if MIDI messages are
    // sent on the "wrong" channel.
    //
    while (usbMIDI.read()) {
        data1 = usbMIDI.getData1();
        data2 = usbMIDI.getData2();

        if (usbMIDI.getType() == usbMIDI.ControlChange) {

            //Serial.print("MIDI ");
            //Serial.print(usbMIDI.getChannel());
            //Serial.print(" ");
            //Serial.print(midi_channel);
            //Serial.print(" ");
            //Serial.print(data1);
            //Serial.print(" ");
            //Serial.println(data2);


            // Accept setting of control channel on any channel
            if (data1 == MIDI_SET_CHANNEL) {
                if (data2 > 16) data2=0;
                set_midi_channel(data2);
            }

            if (usbMIDI.getChannel() == midi_channel) {
                ctrls[data1] = data2; // ctrls[data1] is the definitive value
                switch(data1) {
                    case MIDI_NRPN_CC_MSB: // ctrls[MIDI_NRPN_CC_MSB] is already set
                        break;
                    case MIDI_NRPN_CC_LSB: // ctrls[MIDI_NRPN_CC_LSB] is already set
                        break;
                    case MIDI_NRPN_VAL_MSB: // ctrls[MIDI_NRPN_VAL_MSB] is already set
                        break;
                    case MIDI_NRPN_VAL_LSB: // Writing LSB value triggers NRPN call
                        nrpn_set((ctrls[MIDI_NRPN_CC_MSB]<<7)|ctrls[MIDI_NRPN_CC_LSB],
                             (ctrls[MIDI_NRPN_VAL_MSB]<<7)|ctrls[MIDI_NRPN_VAL_LSB]);
                        break;

                    case MIDI_MASTER_VOLUME:
                        mastervolume(data2);
                        break;

                    case MIDI_SIDETONE_VOLUME:
                        sidetonevolume(data2);
                        break;

                    case MIDI_SIDETONE_FREQUENCY:
                        sidetonefrequency(data2);
                        break;

                    case MIDI_CW_SPEED:
                        if (data2 < 1) data2=1;
                        speed_set(data2);  // report to keyer
                        cwspeed(data2);    // report to radio (and MIDI controller)
                        break;

                    case MIDI_ENABLE_POTS:
                        // Values greater than 63 are on in MIDI standards
                        enable_pots = (data2 > 63);
                        break;

                    case MIDI_RESPONSE:
                        midi_controller_response = (data2 > 63);
                        break;

                    case MIDI_KEYER_AUTOPTT:
                        // auto-PTT by the keyer allowed(data2!=0) or disabled (data2==0)
                        keyer_autoptt_set(data2 > 63);  // report to keyer
                        break;

                    case MIDI_KEYER_LEADIN:
                        // if keyer auto-PTT: lead-in is (10*data2) milliseconds
                        keyer_leadin_set(data2); // report to keyer
                        break;

                    case MIDI_KEYER_HANG:
                        // if keyer auto-PTT: data2=PTT hang time in *dot lengths*
                        keyer_hang_set(data2); // report to keyer
                        break;

                    case MIDI_MUTE_CWPTT:
                        mute_on_cwptt = (data2 > 63);
                        break;

                    case MIDI_MICPTT_HWPTT:
                        micptt_hwptt = (data2 > 63);
                        break;

                    case MIDI_CWPTT_HWPTT:
                        cwptt_hwptt = (data2 > 63);
                        break;

                    default:
                        break;
                }
            }
        } else if (usbMIDI.getType() == usbMIDI.NoteOn) {
            if (data1 == midi_keydown_note) {
                key(data2 != 0);  // Not an on/off value, but velocity information
            } else if (data1 == midi_ptt_note) {
                cwptt(data2 != 0);
            }
        } else if (usbMIDI.getType() == usbMIDI.NoteOff) {
            if (data1 == midi_keydown_note) {
                key(0);  // Ignoring velocity information
            } else if (data1 == midi_ptt_note) {
                cwptt(0);
            }
        }
    }
}

void CWKeyerShield::nrpn_set(const int16_t nrpn, const int16_t value) {
    if ( ! nrpn_is_valid(nrpn)) return;
    nrpns[nrpn] = value;
    switch (nrpn) {
    case NRPN_ID_KEYER:
        nrpns[nrpn] = NRPNV_ID_KEYER; // correct nrpn value
        nrpn_send(nrpn);
        break;
    case NRPN_ID_VERSION:
        nrpns[nrpn] = NRPNV_ID_VERSION; // correct nrpn value
        nrpn_send(nrpn);
        break;
    case NRPN_NNRPN:
        nrpns[nrpn] = NNRPN; // correct nrpn value
        nrpn_send(nrpn);
        break;
    case NRPN_NRPN_QUERY:
        if ( ! nrpn_is_valid(value)) return;
        if ( ! nrpn_is_set(value)) return;
        nrpn_send(value);
        break;
    case NRPN_NRPN_UNSET:
        if ( ! nrpn_is_valid(value)) return;
        nrpns[value] = NRPNV_NOTSET;
        break;

    default:
        process_nrpn(nrpn, value);
        break;
   }
}

void CWKeyerShield::pots()
{
    uint16_t analog_data;
    unsigned long m = millis();
    int val;

    // Call every 10 ms, will handle rollover as both m and last_analog_read are unsigned long
    if ((m - last_analog_read) > 10) {
        //
        // Note on analog debouncing:
        // The value obtained by analogRead is a 12-bit value, that is converted
        // to a 13-bit moving average (0...8191). Changes of the input value are
        // 'accepted' if they deviate more than a given threshold (64 for volume,
        // 128 for freq and 256 for speed) from the previous 'nominal' value
        // (this is referred to as 'discretization').
        //
        // Now there is a problem:
        // Let the "discretization" be 64 (as for the volume pots). Then, if the
        // "last analog value" was 64 (mapping to 1 on the range 0-127), you can never
        // reach zero. Likewise, if the "last analog value" was 63 (mapping to 0),
        // then it is impossible to reach the next value and you have to jump 2 steps.
        // Therefore, the last analog value to be remembered will be placed
        // at the mid-point of the interval corresponding to the new setting.
        // (in our example:  63 ==> 32; 64 ==> 96).
        //
        // Note on the soldering of the pots:
        // In the CW Keyer Shield, the pots are soldered such that turning them clockwise
        // *decreases* the analogRead value downto zero. Therefore the max analog value
        // is a "zero reading" and a zero analog value is a "max reading".
        //
        if (last_analog_line == 0) {
            // Master volume
            analog_data = analogRead(Pin_MasterVolume);
            Analog_MasterVol = (Analog_MasterVol >> 1) + analog_data;

            if (abs(Analog_MasterVol - last_mastervol) > 64) {
                val=(Analog_MasterVol >> 6) & 0x7f;              // 0...127
                mastervolume(127-val);                           // correct soldering
                last_mastervol = (val << 6) + 32;                // new "old" value
                //Serial.print("MV ");
                //Serial.println(analog_data);
            }
        } else if (last_analog_line == 1) {

            // Sidetone volume
            analog_data = analogRead(Pin_SideToneVolume);
            Analog_SideVol = (Analog_SideVol >> 1) + analog_data;
            if (abs(Analog_SideVol - last_sidevol) > 64) {
                val=(Analog_SideVol >> 6) & 0x7F;                // 0...127
                sidetonevolume(127-val);                         // correct soldering
                last_sidevol = (val << 6) + 32;                  // new "old" value
                //Serial.print("SV ");
                //Serial.println(analog_data);
            }
        } else if (last_analog_line == 2) {

            // Sidetone frequency
            analog_data = analogRead(Pin_SideToneFrequency);
            Analog_SideFreq = (Analog_SideFreq >> 1) + analog_data;
            if (abs(Analog_SideFreq - last_sidefreq) > 128) {
                val=(Analog_SideFreq >> 7) & 0x3F;               // val 0...63, mapped to 40 ... 103 (400 to 1030 Hz)
                sidetonefrequency(103-val);                      // correct soldering
                last_sidefreq = (val << 7) + 64;                 // new "old" value
                //Serial.print("SF ");
                //Serial.println(analog_data);
            }
        } else if (last_analog_line == 3) {

            // Speed
            analog_data = analogRead(Pin_Speed);
            Analog_Speed = (Analog_Speed >> 1) + analog_data;
            if (abs(Analog_Speed - last_speed) > 256) {
                val=(Analog_Speed >> 8) & 0x1f;                  // 0...31, mapped to 0 ... 127 through SpeedTab
                speed_set(SpeedTab[31-val]);                     // report to keyer
                cwspeed(SpeedTab[31-val]);                       // report to radio
                last_speed = (val << 8) + 128;                   // new "old" value
                //Serial.print("SP ");
                //Serial.println(analog_data);
            }
        }

        last_analog_read = m;
        last_analog_line = (last_analog_line + 1) & 0x3;
    }
}

void CWKeyerShield::hwptt(int state)
{
    //
    // Set the hardware PTT line
    //
    if (Pin_PTTout > 0) {
      digitalWrite(Pin_PTTout, state ? 1 : 0);
    }
}

void CWKeyerShield::midiptt(int state)
{
    //
    // send MIDI PTT message to radio
    //
    if ((midi_channel > 0) && (midi_ptt_note < 128)) {
        usbMIDI.sendNoteOn(midi_ptt_note, state ? 127 : 0, midi_channel);
    }
}

void CWKeyerShield::key(int state)
{
    //
    // Interface to the keyer, to trigger key-up and key-down
    //
    // a) switch side tone on/off
    // b) send MIDI message
    // c) set hardware line
    //
    teensyaudiotone.setTone(state);
    if ((midi_channel > 0) && (midi_keydown_note < 128)) {
        usbMIDI.sendNoteOn(midi_keydown_note, state ? 127 : 0, midi_channel);
        usbMIDI.send_now();
    }
    if (Pin_CWout >= 0) {
      digitalWrite(Pin_CWout, state? 1 : 0);
    }
}

void CWKeyerShield::cwptt(int state)
{
    //
    // Interface to the keyer. The keyer calls this function
    // if it wants to activate PTT
    //
    if (mute_on_cwptt || state == 0) {
      //
      // possibly mute the audio from the PC (but not the side tone)
      //
      teensyaudiotone.muteAudioIn(state);
    }
    cwptt_state = state;  // Actual PTT setting is done in monitor_ptt()
}


void CWKeyerShield::mastervolume(uint8_t level)  // input level from 0 ... 127
{
    if (midi_controller_response && midi_channel > 0) {
        usbMIDI.sendControlChange(MIDI_MASTER_VOLUME, level, midi_channel);
    }
    masterlevel_target=(float)level/127.0;
}

void CWKeyerShield::sidetonevolume(uint8_t level)    // input level from 0 ... 127
{
    //
    // The input value (level) is in the range 0-31 and converted to
    // an amplitude using VolTab, such that a logarithmic pot is
    // simulated.
    //
    if (midi_controller_response && midi_channel > 0) {
        usbMIDI.sendControlChange(MIDI_SIDETONE_VOLUME, level, midi_channel);
    }
    level = level >> 2;                  // reduce to 0...31
    sidetonelevel_target=VolTab[level];
}

void CWKeyerShield::sidetonefrequency(uint8_t freq)   // input freq from 0 ... 127, maps to 0 ... 1270 Hz
{
    sine.frequency( (float)(10*freq) );

    // Code provides a unified on/off switch for control change responses, there is no distinction between controller and SDR
    if (midi_controller_response && midi_channel > 0) {
        usbMIDI.sendControlChange(MIDI_SIDETONE_FREQUENCY, freq, midi_channel);
    }
}

void CWKeyerShield::cwspeed(uint8_t speed)   // input speed from 0 ... 127
{
    if (speed == 0) speed = 1;  // even more paranoia

    if (midi_controller_response && midi_channel> 0) {
        usbMIDI.sendControlChange(MIDI_CW_SPEED, speed, midi_channel);
    }
 }

#endif
