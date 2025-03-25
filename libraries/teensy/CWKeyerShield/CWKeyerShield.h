/* -*- mode: c++; c-basic-offset: 4 -*- */
/* SofterHardwareCWKeyerShield for Teensy 4.X
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

#ifndef CWKeyerShield_h_
#define CWKeyerShield_h_

#include "Arduino.h"
#include "Audio.h"
#include "AudioStream.h"
#include "arm_math.h"
#include "TeensyAudioTone.h"

//
// External functions, to be implemented in the keyer
// (at least as dummies)
//
void speed_set(int speed);          // set CW speed in keyer
void keyer_autoptt_set(int enable); // enable/disable PTT activation by keyer
void keyer_leadin_set(int leadin);  // set keyer PTT lead-in time (leadin milli-seconds) (if using auto-PTT)
void keyer_hang_set(int hang);      // set keyer PTT hang time (in *dotlengths*) (if using auto-PTT)

//
// avoid certain "continuous controllers"
//
// No.  0, 32           (Bank select)
// No.  6, 38, 98--101  (Registered and non-Registered Parameters)
// No. 88               (High-resolution velocity prefix)
// No. 96, 97           (Increment/Decrement)
// No. 120-127          (Channel mode messages)
//
//

enum midi_control_selection {

    MIDI_NRPN_CC_MSB              = 99,
    MIDI_NRPN_CC_LSB              = 98,
    MIDI_NRPN_VAL_MSB             = 6,
    MIDI_NRPN_VAL_LSB             = 38,

    MIDI_MASTER_VOLUME            = 7,      // set master volume
    MIDI_MASTER_BALANCE           = 8,      //TODO: stereo balance
    MIDI_MASTER_PAN               = 10,     //TODO: stereo position of CW tone

    MIDI_SIDETONE_VOLUME          = 12,     // set sidetone volume
    MIDI_SIDETONE_FREQUENCY       = 13,     // set sidetone frequency

    MIDI_INPUT_LEVEL              = 16,     //TODO:

    MIDI_ENABLE_POTS              = 64,     // enable/disable potentiometers
    MIDI_KEYER_AUTOPTT            = 65,     // enable/disable auto-PTT from CW keyer
    MIDI_RESPONSE                 = 66,     // enable/disable reporting back to SDR and MIDI controller
    MIDI_MUTE_CWPTT               = 67,     // enable/disable muting of RX audio during auto-PTT
    MIDI_MICPTT_HWPTT             = 68,     // enable/disable that MICIN triggers the hardware PTT output
    MIDI_CWPTT_HWPTT              = 69,     // enable/disable that CWPTT triggers the hardware PTT output

    MIDI_KEYER_HANG               = 72,     // set Keyer hang time (if auto-PTT active)
    MIDI_KEYER_LEADIN             = 73,     // set Keyer lead-in time (if auto-PTT active)
    MIDI_CW_SPEED                 = 74,     // set CW speed
    MIDI_INPUT_SELECT             = 75,     //TODO:
    MIDI_SET_CHANNEL              = 119     // Change the default channel to use
};

enum midi_nrpn_values {
    NRPNV_NOTSET = -1,          // initialized value of NRPNs, not a legal value
    NRPNV_ID_KEYER = 0x50F,     // SOFterharderware, only 14 bits
    NRPNV_ID_VERSION = 101
};

enum midi_nrpn_selection {
    NRPN_NOTHING                       = 0,   // not a nrpn nrpn value, where a null pointer is needed
    NRPN_ID_KEYER                      = 1,   // identify this keyer for the correspondent
    NRPN_ID_VERSION                    = 2,   // identify this keyer version for the correspondent
    NRPN_NNRPN                         = 3,   // return how many NRPNs are allocated
    NRPN_NRPN_QUERY                    = 4,   // take the value as a nrpn number and send that nrpns value, no response if no value set
    NRPN_NRPN_UNSET                    = 5,   // take the value as a nrpn number and make that nrpn NRPNV_NOTSET
    MIDI_NRPN_WM8960_ENABLE            = 11,
    MIDI_NRPN_WM8960_INPUT_LEVEL       = 12,
    MIDI_NRPN_WM8960_INPUT_SELECT      = 13,
    MIDI_NRPN_WM8960_VOLUME            = 14,
    MIDI_NRPN_WM8960_HEADPHONE_VOLUME  = 15,
    MIDI_NRPN_WM8960_HEADPHONE_POWER   = 16,
    MIDI_NRPN_WM8960_SPEAKER_VOLUME    = 17,
    MIDI_NRPN_WM8960_SPEAKER_POWER     = 18,
    MIDI_NRPN_WM8960_DISABLE_ADCHPF    = 19,
    MIDI_NRPN_WM8960_ENABLE_MICBIAS    = 20,
    MIDI_NRPN_WM8960_ENABLE_ALC        = 21,
    MIDI_NRPN_WM8960_MIC_POWER         = 22,
    MIDI_NRPN_WM8960_LINEIN_POWER      = 23,
    MIDI_NRPN_WM8960_RAW_MASK          = 24,
    MIDI_NRPN_WM8960_RAW_DATA          = 25,
    MIDI_NRPN_WM8960_RAW_WRITE         = 26,
    MIDI_NRPN_KEYDOWN_NOTE             = 27,
    MIDI_NRPN_PTT_NOTE                 = 28
};

//
// The hardware setup (digital and analog I/O lines, which audio system to use)
// is passed as a parameter to the constructor.
// All other parameters can in principle be changed any time (either through
// an interface, through turning a pot, or through an incoming MIDI message)
//
class CWKeyerShield
{
public:
    // User must be able to see good default values without referring to any other code, for example
    // there should be no need to look at TeensyWinkeyEmulator
    CWKeyerShield (int i2s              = 1,
                   int pin_sidevol      = A2,
                   int pin_sidefreq     = A3,
                   int pin_mastervol    = A1,
                   int pin_speed        = A8,
                   int pin_ptt_in       = 3,
                   int pin_ptt_out      = 4,
                   int pin_cw_out       = 5,
                   // Below values can be changed later by accessors
                   int midi_ch          = 10,
                   int midi_keydown_nt  = 17,
                   int midi_ptt_nt      = 18)
                   :
    sine(),
    usbaudioinput(),
    teensyaudiotone(),
    patchinl (usbaudioinput,   0, teensyaudiotone, 0),
    patchinr (usbaudioinput,   1, teensyaudiotone, 1),
    patchwav (sine,            0, teensyaudiotone, 2)
    {
      nrpn_init();              // because any of these could be aliases to nrpn values
      Pin_SideToneFrequency = pin_sidefreq;
      Pin_SideToneVolume    = pin_sidevol;
      Pin_MasterVolume      = pin_mastervol;
      Pin_Speed             = pin_speed;

      Pin_PTTin             = pin_ptt_in;
      Pin_PTTout            = pin_ptt_out;
      Pin_CWout             = pin_cw_out;

      midi_ptt_note = midi_ptt_nt;
      midi_keydown_note = midi_keydown_nt;
      midi_channel = midi_ch;

      //
      // Audio output. The audio output method is encoded in the i2s variable:
      //
      // i2s = 0:   MQS audio output, no master volume control
      // i2s = 1:   I2S audio output, assuming a WM8960   device
      // i2s = 2:       I2S audio output, assuming a SGTL5100 device
      //
      // use MQS as the default if an illegal value has been given
      // (no audio input in this case)
      //
      switch (i2s) {
        case 0:
        default:
            audioout = new AudioOutputMQS;
            break;
        case 1:
            audioout = new AudioOutputI2S;
            audioin  = new AudioInputI2S;
            wm8960   = new AudioControlWM8960;
            break;
        case 2:
            audioout = new AudioOutputI2S;
            audioin  = new AudioInputI2S;
            sgtl5000 = new AudioControlSGTL5000;
            break;
      }
      //
      // Connect teensyaudiotone to audio output
      //
      if (audioout) {
        patchoutl = new AudioConnection(teensyaudiotone, 0, *audioout,        0);
        patchoutr = new AudioConnection(teensyaudiotone, 1, *audioout,        1);
      }
      if (audioin) {
        //
        // Connect I2S audio input to USB audio out
        //
        patchusboutl = new AudioConnection(*audioin, 0, usbaudiooutput, 0);
        patchusboutr = new AudioConnection(*audioin, 1, usbaudiooutput, 1);
      }
    }

    int8_t ctrls[128];          // current values of controls
    static const unsigned NNRPN = 128;  // number of NRPNs maintained
    int16_t nrpns[NNRPN];         // current values of NRPNs
    void nrpn_init(void) {
        for (unsigned nrpn = 0; nrpn < NNRPN; nrpn += 1) nrpns[nrpn] = NRPNV_NOTSET;
    }
    void nrpn_set(const int16_t nrpn, const int16_t value);
    void nrpn_send(const int16_t nrpn) {
        usbMIDI.beginNrpn(nrpn, midi_channel);
        usbMIDI.sendNrpnValue(nrpns[nrpn], midi_channel);
    }
    bool nrpn_is_valid(const int16_t nrpn) { // nrpn number is in range
        return ((unsigned)nrpn) < NNRPN;
    }
    bool nrpn_is_set(const int16_t nrpn) { // nrpn value has been set
        return nrpn_is_valid(nrpn) && nrpns[nrpn] != NRPNV_NOTSET;
    }

    void setup(void);                                           // to be executed once upon startup
    void loop(void);                                            // to be executed at each heart beat
    void key(int state);                                        // CW Key up/down event
    void cwptt(int state);                                      // PTT open/close event triggered by keyer
    void hwptt(int state);                                      // set hardware PTT
    void midiptt(int state);                                    // send MIDI PTT event
    void mastervolume(uint8_t level);                           // set master volume
    void sidetonevolume(uint8_t level);                         // Change side tone volume
    void sidetonefrequency(uint8_t freq);                       // Change side tone frequency
    void cwspeed(uint8_t speed);                                // send CW speed event
    void sidetoneenable(int onoff) {                            // enable/disable side tone
       teensyaudiotone.sidetoneenable(onoff);
    }

    void set_cwptt_mute_option(int v)    { mute_on_cwptt = v; }

    void set_midi_channel(int v)         { midi_channel = v; }
    int  get_midi_channel(void)          { return midi_channel; }

    void set_midi_ptt_note(int v)        { midi_ptt_note = v; }
    int  get_midi_ptt_note(void)         { return midi_ptt_note; }

    void set_midi_keydown_note(int v)    { midi_keydown_note = v; }
    int  get_midi_keydown_note(void)     { return midi_keydown_note; }


private:
    void monitor_ptt(void);                                     // monitor PTT-in line, do PTT
    void midi(void);                                            // MIDI loop
    void pots(void);                                            // Potentiometer loop
    void adjust(void);                                          // slowly adjust SideTone/Master volume
    void process_nrpn(const int16_t nrpn_cc, const int16_t nrpn_val); // Process NRPN midi messages
    AudioSynthWaveformSine  sine;               // free-running side tone oscillator
    AudioInputUSB           usbaudioinput;      // Audio in from Computer
    AudioOutputUSB          usbaudiooutput;     // Audio out to Computer
    TeensyAudioTone         teensyaudiotone;    // Side tone mixer
    AudioConnection         patchinl;           // Cable "L" from Audio-in to side tone mixer
    AudioConnection         patchinr;           // Cable "R" from Audio-in to side tone mixer
    AudioConnection         patchwav;           // Mono-Cable from Side tone oscillator to side tone mixer
    AudioConnection         *patchusboutl=NULL;
    AudioConnection         *patchusboutr=NULL;
    //
    // These are dynamically created, since they depend on the actual
    // audio output device
    //
    AudioStream             *audioout=NULL;     // Audio output to headphone
    AudioStream             *audioin=NULL;      // Audio output to computer
    AudioControlSGTL5000    *sgtl5000=NULL;     // SGTL5000 output controller
    AudioControlWM8960      *wm8960=NULL;       // WM8960 output controller
    AudioConnection         *patchoutl=NULL;    // Cable "L" from side tone mixer to headphone
    AudioConnection         *patchoutr=NULL;    // Cable "R" from side tone mixer to headphone

    //
    // MIDI channel to use for communication with the controller
    // *and* with the radio
    // ATTN: the 16 midi channels are numbered 1-16 (not 0-15!),
    //   since this was designed for musicians not computer scientists.
    //   So a channel value of 0 means "no communication"
    //
    uint8_t midi_channel;

    uint8_t midi_ptt_note;
    uint8_t midi_keydown_note;

    // Enable/disable  that MICPTT/CWPTT triggers the hardware PTT output.
    // (both will trigger a MIDI message in either case)
    uint8_t micptt_hwptt      = 1;
    uint8_t cwptt_hwptt       = 1;

    // PTT state from keyer. This flag is set if the keyer wants to
    // activate PTT. The actual PTT switching is done in monitor_ptt()
    uint8_t cwptt_state = 0;

    // PTT state of the MIDI and hardware PTT "lines"
    uint8_t hwptt_state=0;
    uint8_t midiptt_state=0;

    // Enable/disable MIDI control change responses
    uint8_t midi_controller_response     = 1;

    // Enable/disable POTS
    uint8_t enable_pots       = 1;

    //
    // (Digital) inputs to monitor / (Digital) output lines
    // A negative value indicates 'do not use'
    // Default values refer to the SofterHardware shield
    //
    int Pin_PTTin              = -1;
    int Pin_PTTout             = -1;
    int Pin_CWout              = -1;

    //
    // (Analog) inputs to monitor. A negative value indicates "do not use this feature"
    //
    int Pin_SideToneFrequency = -1;
    int Pin_SideToneVolume    = -1;
    int Pin_MasterVolume      = -1;
    int Pin_Speed             = -1;

    //
    // current states of the analog input lines,
    // kept for de-noising.
    //
    uint16_t Analog_SideFreq  = 0;
    uint16_t Analog_SideVol   = 0;
    uint16_t Analog_MasterVol = 0;
    uint16_t Analog_Speed     = 0;

    uint16_t last_sidefreq         = 0;
    uint16_t last_sidevol          = 0;
    uint16_t last_mastervol        = 0;
    uint16_t last_speed            = 0;

    int mute_on_cwptt  = 0;                 // If set, Audio from PC is muted while CWPTT is active

    unsigned long last_analog_read = 0;     // time of last analog read
    unsigned int  last_analog_line=0;       // which line was read last time

    unsigned long last_ptt_read = 0;        // time of last PTT-in reading
    uint8_t       last_ptt_in = 0;          // state of PTT-in line
    uint8_t       ptt_state = 0;            // PTT state

    // Accumulators for MIDI commands with multiple data
    int16_t wm8960_raw_mask = -1;
    int16_t wm8960_raw_data = -1;

    //
    // Variables for the "continuous" adjustment of side tone and master volume.
    // This is meant to reduce audible "cracks" when changing the volume
    //
    float sidetonelevel_target;
    float sidetonelevel_actual;
    float masterlevel_target;
    float masterlevel_actual;
    unsigned long last_adjust=0;

    //
    // Side tone level (amplitude), in 32 steps from zero to one, covering 40 dB
    // alltogether.  Set first entry (nominally: -40 dB, amplitude 0.0100) to zero
    // to allow for "complete muting"
    //
    float VolTab[32] = {0.0000, 0.0116, 0.0135, 0.0156, 0.0181, 0.0210, 0.0244, 0.0283,
                        0.0328, 0.0381, 0.0442, 0.0512, 0.0595, 0.0690, 0.0800, 0.0928,
                        0.1077, 0.1250, 0.1450, 0.1682, 0.1951, 0.2264, 0.2626, 0.3047,
                        0.3535, 0.4101, 0.4758, 0.5520, 0.6404, 0.7430, 0.8620, 1.0000};

    //
    // CW speed table (wpm), in 32 steps from 5 to 52 wpm.
    // differences between adjacent steps increase at the
    // top of the scale. With the pot in center position we
    // have about 20 wpm.
    //
    uint8_t SpeedTab[32] = {  5,  6,  7,  8,  9, 10, 11, 12,
                             13, 14, 15, 16, 17, 18, 19, 20,
                             21, 22, 23, 24, 26, 28, 30, 32,
                             34, 36, 38, 40, 43, 46, 49, 52};

};

#endif
