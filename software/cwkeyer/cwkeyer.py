import mido  # See https://mido.readthedocs.io/en/latest/


SEND = "Teensy MIDI/Audio:Teensy MIDI/Audio MIDI 1 24:0"
SCH  = 0   ## Send channel, Python starts at 0 for 0-15, Teensy is 1-16
RECV = "Teensy MIDI/Audio:Teensy MIDI/Audio MIDI 1 24:0"
RCH  = 0   ## Receive channel, Python starts at 0 for 0-15, Teensy is 1-16

## Control selection codes, must match those used in TeensyUSBAudioMidi


MIDI_SET_ACCUM           = 0    ## initiate multi-byte value
MIDI_SHIFT_ACCUM         = 1    ## update multi-byte value

MIDI_MASTER_VOLUME       = 4    ## set master volume
MIDI_SIDETONE_VOLUME     = 5    ## set sidetone volume
MIDI_SIDETONE_FREQUENCY  = 6    ## set sidetone frequency
MIDI_CW_SPEED            = 7    ## set CW speed
MIDI_ENABLE_POTS         = 8    ## enable/disable potentiometers
MIDI_KEYER_AUTOPTT       = 9    ## enable/disable auto-PTT from CW keyer
MIDI_KEYER_LEADIN        = 10   ## set Keyer lead-in time (if auto-PTT active)
MIDI_KEYER_HANG          = 11   ## set Keyer hang time (if auto-PTT active)
MIDI_RESPONSE            = 12   ## enable/disable reporting back to MIDI controller
MIDI_MUTE_CWPTT          = 13   ## enable/disable muting of RX audio during auto-PTT
MIDI_MICPTT_HWPTT        = 14   ## enable/disable that MICIN triggers the hardware PTT output

MIDI_RX_CH               = 16   ## set MIDI channel to/from controller
MIDI_TX_CH               = 17   ## set MIDI channel to radio

MIDI_KEYDOWN_NOTE        = 18   ## set MIDI note for key-down (to radio)
MIDI_PTT_MIC_NOTE        = 19
MIDI_PTT_IN_NOTE         = 20
MIDI_CWPTT_NOTE          = 21   ## set MIDI note for PTT activation (to radio)
MIDI_SPEED_CTRL          = 22   ## set MIDI controller for cw speed (to radio)
MIDI_FREQ_CTRL           = 23   ## set MIDI controller for side tone frequency (to radio)


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
  def write_accum(self,v): self.txcc(MIDI_SET_ACCUM,v)

  ## Shift in v as the new lower 7 bits of the accumulator, allows sending large values in multiple steps
  def shift_accum(self,v): self.txcc(MIDI_SHIFT_ACCUM,v)

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

  ## piHPSDR specfici
  def speed_ctrl(self,v): self.txcc(MIDI_SPEED_CTRL,v)
  def freq_ctrl(self,v): self.txcc(MIDI_FREQ_CTRL,v)



if __name__ == "__main__":
  ## Available Output Ports
  print("Edit the top of this file to use the proper string names for send and receive ports on your system.")
  print("Available send device names:",mido.get_output_names())
  print("Available recv device names:",mido.get_input_names())
  k = CWKeyer(SEND,SCH,RECV,RCH)