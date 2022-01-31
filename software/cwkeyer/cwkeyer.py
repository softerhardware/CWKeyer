import mido  # See https://mido.readthedocs.io/en/latest/


SEND = "Teensy MIDI/Audio:Teensy MIDI/Audio MIDI 1 24:0"
SCH  = 1   ## Send channel, Python starts at 0 for 0-15, Teensy is 1-16
RECV = "Teensy MIDI/Audio:Teensy MIDI/Audio MIDI 1 24:0"
RCH  = 0   ## Receive channel, Python starts at 0 for 0-15, Teensy is 1-16

## Control selection codes, must match those used in TeensyUSBAudioMidi


MIDI_SET_A                    = 0    ## Set 7-bit accumulator A
MIDI_SET_B                    = 1    ## Set 7-bit accumulator B
MIDI_SET_C                    = 2    ## Set 7-bit accumulator C

MIDI_MASTER_VOLUME            = 4    ## set master volume
MIDI_SIDETONE_VOLUME          = 5    ## set sidetone volume
MIDI_SIDETONE_FREQUENCY       = 6    ## set sidetone frequency
MIDI_CW_SPEED                 = 7    ## set CW speed
MIDI_ENABLE_POTS              = 8    ## enable/disable potentiometers
MIDI_KEYER_AUTOPTT            = 9    ## enable/disable auto-PTT from CW keyer
MIDI_KEYER_LEADIN             = 10   ## set Keyer lead-in time (if auto-PTT active)
MIDI_KEYER_HANG               = 11   ## set Keyer hang time (if auto-PTT active)
MIDI_RESPONSE                 = 12   ## enable/disable reporting back to MIDI controller
MIDI_MUTE_CWPTT               = 13   ## enable/disable muting of RX audio during auto-PTT
MIDI_MICPTT_HWPTT             = 14   ## enable/disable that MICIN triggers the hardware PTT output

MIDI_RX_CH                    = 16   ## set MIDI channel to/from controller
MIDI_TX_CH                    = 17   ## set MIDI channel to radio

MIDI_KEYDOWN_NOTE             = 18   ## set MIDI note for key-down (to radio)
MIDI_PTT_MIC_NOTE             = 19
MIDI_PTT_IN_NOTE              = 20
MIDI_CWPTT_NOTE               = 21   ## set MIDI note for PTT activation (to radio)
MIDI_SPEED_CTRL               = 22   ## set MIDI controller for cw speed (to radio)
MIDI_FREQ_CTRL                = 23   ## set MIDI controller for side tone frequency (to radio)

MIDI_WM8960_ENABLE            = 40
MIDI_WM8960_INPUT_LEVEL       = 41
MIDI_WM8960_INPUT_SELECT      = 42
MIDI_WM8960_VOLUME            = 43
MIDI_WM8960_HEADPHONE_VOLUME  = 44
MIDI_WM8960_HEADPHONE_POWER   = 45
MIDI_WM8960_SPEAKER_VOLUME    = 46
MIDI_WM8960_SPEAKER_POWER     = 47
MIDI_WM8960_DISABLE_ADCHPF    = 48
MIDI_WM8960_ENABLE_MICBIAS    = 49
MIDI_WM8960_ENABLE_ALC        = 50
MIDI_WM8960_MIC_POWER         = 51
MIDI_WM8960_LINEIN_POWER      = 52
MIDI_WM8960_RAW_WRITE         = 53



def midi_callback(message):
  print(message)


class CWKeyer:
  # CW Keyer object
  def __init__(self,send_name,send_ch,recv_name,recv_ch):
    self.tx = mido.open_output(send_name)
    self.tc = send_ch
    self.rx = mido.open_input(recv_name,callback=midi_callback)
    self.rc = recv_ch

  ## Send a control change message
  ## c is the control selecton: 0 to 127
  ## v is the value: 0 to 127 (Larger values may be accumulated)
  def txcc(self,c,v):
    v = int(v)
    msg = mido.Message('control_change', channel=self.tc, control=c&0x7f, value=v&0x7f)
    self.tx.send(msg)

  ## v is float 0 to 1.0
  def txccf(self,c,v):
    if v < 0:   v = 0
    if v > 1.0: v = 1.0
    v = int(v*127.5)
    self.txcc(c,v)

  ## 0.0 to 1.0
  def master_volume(self,v): self.txccf(MIDI_MASTER_VOLUME,v)

  ## 0.0 to 1.0
  def sidetone_volume(self,v): self.txccf(MIDI_SIDETONE_VOLUME,v)

  ## 250Hz to ~1270Hz
  def sidetone_frequency(self,v):
    if v < 250:  v = 250
    if v > 1270: v = 1270
    v = (v - 250) >> 3
    self.txcc(MIDI_SIDETONE_FREQUENCY,v)

  ## 1 to 127 WPM
  def cw_speed(self,v): self.txcc(MIDI_CW_SPEED,v)

  ## Set the accumulator with v, overwrites any accumulated value
  def set_accum_a(self,v): self.txcc(MIDI_SET_A,v)
  def set_accum_b(self,v): self.txcc(MIDI_SET_B,v)
  def set_accum_c(self,v): self.txcc(MIDI_SET_C,v)

  ## Set the RX channel for the keyer, keyer responds to this command on any channel
  def rx_ch(self,v): self.txcc(MIDI_RX_CH,v)

  ## Set the TX channel the keyer will use for notes and responses
  def tx_ch(self,v): self.txcc(MIDI_TX_CH,v)

  def keydown_note(self,v): self.txcc(MIDI_KEYDOWN_NOTE,v)
  def cwptt_note(self,v): self.txcc(MIDI_CWPTT_NOTE, v)
  def ptt_mic_note(self,v): self.txcc(MIDI_PTT_MIC_NOTE,v)
  def ptt_in_note(self,v): self.txcc(MIDI_PTT_IN_NOTE,v)

  ## Enable/disable MIDI responses
  def response(self,v): self.txcc(MIDI_RESPONSE,v)

  ## Enable/disable POTs
  def enable_pots(self,v): self.txcc(MIDI_ENABLE_POTS,v)

  ## Enable/disable CW PTT
  def keyer_autoptt(self,v): self.txcc(MIDI_KEYER_AUTOPTT,v)

  def keyer_leadin(self,v): self.txcc(MIDI_KEYER_LEADIN,v)
  def keyer_hang(self,v): self.txcc(MIDI_KEYER_HANG,v)

  def mute_cwptt(self,v): self.txcc(MIDI_MUTE_CWPTT,v)

  ## enable/disable that MICIN triggers the hardware PTT output
  def micptt_hwptt(self,v): self.txcc(MIDI_MICPTT_HWPTT,v)

  ## piHPSDR specfic
  def speed_ctrl(self,v): self.txcc(MIDI_SPEED_CTRL,v)
  def freq_ctrl(self,v): self.txcc(MIDI_FREQ_CTRL,v)

  def wm8960_enable(self,v): self.txcc(MIDI_WM8960_ENABLE,v)

  def wm8960_input_level(self,l,r=None):
    if r:
      ## Send right as accumulator first
      self.txccf(MIDI_SET_A,r)
    self.txccf(MIDI_WM8960_INPUT_LEVEL,l)

  def wm8960_input_select(self,v): self.txcc(MIDI_WM8960_INPUT_SELECT,v)

  def wm8960_volume(self,l,r=None):
    if r:
      ## Send right as accumulator first
      self.txccf(MIDI_SET_A,r)
    self.txccf(MIDI_WM8960_VOLUME,l)

  def wm8960_headphone_volume(self,l,r=None):
    if r:
      ## Send right as accumulator first
      self.txccf(MIDI_SET_A,r)
    self.txccf(MIDI_WM8960_HEADPHONE_VOLUME,l)

  def wm8960_headphone_power(self,l,r):
    v = 0
    if l: v = 2
    if r: v = v | 1
    self.txcc(MIDI_WM8960_HEADPHONE_POWER,v)

  def wm8960_speaker_volume(self,l,r=None):
    if r:
      ## Send right as accumulator first
      self.txccf(MIDI_SET_A,r)
    self.txccf(MIDI_WM8960_SPEAKER_VOLUME,l)

  def wm8960_speaker_power(self,l,r):
    v = 0
    if l: v = 2
    if r: v = v | 1
    self.txcc(MIDI_WM8960_SPEAKER_POWER,v)

  def wm8960_disable_adchpf(self,v): self.txcc(MIDI_WM8960_DISABLE_ADCHPF,v)
  def wm8960_enable_micbias(self,v): self.txcc(MIDI_WM8960_ENABLE_MICBIAS,v)

  def wm8960_enable_alc(self,l,r):
    v = 0
    if l: v = 2
    if r: v = v | 1
    self.txcc(MIDI_WM8960_ENABLE_ALC,v)

  def wm8960_enable_mic_power(self,l,r):
    v = 0
    if l: v = 2
    if r: v = v | 1
    self.txcc(MIDI_WM8960_ENABLE_MIC_POWER,v)

  def wm8960_enable_linein_power(self,l,r):
    v = 0
    if l: v = 2
    if r: v = v | 1
    self.txcc(MIDI_WM8960_ENABLE_MIC_POWER,v)

  def wm8960_raw_write(self,reg,val,mask,force=False):
    ## Repacking as midi has 7-bit values but wm8960 uses 9-bit values
    a = val   & 0x7f          ## a is lower 7 bits of value
    b = mask & 0x7f           ## b is lower 7 bits of mask
    c = 0x10 if force else 0  ## force is bit 4 of c
    c = c | ((mask >> 5) & 0b01100) ## mask upper bits are c[3:2]
    c = c | ((val >> 7) & 0b011)    ## value upper bits are c[1:0]
    self.txcc(MIDI_SET_A,a)
    self.txcc(MIDI_SET_B,b)
    self.txcc(MIDI_SET_C,c)
    self.txcc(MIDI_WM8960_RAW_WRITE,reg)


if __name__ == "__main__":
  ## Available Output Ports
  print("Edit the top of this file to use the proper string names for send and receive ports on your system.")
  print("Available send device names:",mido.get_output_names())
  print("Available recv device names:",mido.get_input_names())
  k = CWKeyer(SEND,SCH,RECV,RCH)