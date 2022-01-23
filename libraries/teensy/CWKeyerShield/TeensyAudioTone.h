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

#ifndef TeensyAudioTone_h_
#define TeensyAudioTone_h_

#include "Arduino.h"
#include "Audio.h"
#include "AudioStream.h"
#include "arm_math.h"

class TeensyAudioTone : public AudioStream
{
public:
    TeensyAudioTone() : AudioStream(3, inputQueueArray) {
        sidetone_enabled = 1;
        tone = 0;
        mute = 0;
        windowindex = 0;
    }

    virtual void update(void);

    void setTone(uint8_t state) {
        tone = state;
    }
    void sidetoneenable(uint8_t state) {
      sidetone_enabled = state;
    }

    void muteAudioIn(uint8_t state) {
        //
        // mute/unmute audio from PC
        //
        mute = state;
    }

private:
    audio_block_t *inputQueueArray[3];

    uint8_t  sidetone_enabled;
    uint8_t  tone;         // tone on/off flag
    uint8_t  mute;         // mute on/off flag
    uint8_t  windowindex;  // pointer into the "ramp"
};

#endif
