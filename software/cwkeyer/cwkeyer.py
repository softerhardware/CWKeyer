import mido  # See https://mido.readthedocs.io/en/latest/


SEND = "Teensy MIDI/Audio:Teensy MIDI/Audio MIDI 1 24:0"
SCH  = 0   ## Send channel, Python starts at 0 for 0-15, Teensy is 1-16
RECV = "Teensy MIDI/Audio:Teensy MIDI/Audio MIDI 1 24:0"
RCH  = 0   ## Receive channel, Python starts at 0 for 0-15, Teensy is 1-16

## Control selection codes, must match those used in TeensyUSBAudioMidi

MIDI_SET_ACCUM           = 0
MIDI_SHIFT_ACCUM         = 1

MIDI_MASTER_VOLUME       = 4
MIDI_SIDETONE_VOLUME     = 5
MIDI_SIDETONE_FREQUENCY  = 6
MIDI_CW_SPEED            = 7
MIDI_ENABLE_POTS         = 8

MIDI_RX_CH               = 16
MIDI_TX_CH               = 17
MIDI_KEYDOWN_NOTE        = 18
MIDI_PTT_MIC_NOTE        = 19
MIDI_PTT_IN_NOTE         = 20
MIDI_CW_HEADTAIL_NOTE    = 21
MIDI_CW_HEADTAIL_ON_RING = 22
MIDI_CW_HEADTAIL_ON_RING = 23



class CWKeyer:
  # CW Keyer object
  def __init__(self,send_name,send_ch,recv_name,recv_ch):
    self.tx = mido.open_output(send_name)
    self.tc = send_ch
    self.rx = mido.open_input(recv_name)
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
  def set_accum(self,v): self.txcc(MIDI_SET_ACCUM,v)

  ## Shift in v as the new lower 7 bits of the accumulator, allows sending large values in multiple steps
  def shift_accum(self,v): self.txcc(MIDI_SHIFT_ACCUM,v)

  ## Set the RX channel for the keyer, keyer responds to this command on any channel
  def set_rx_ch(self,v): self.txcc(MIDI_RX_CH,v)

  ## Set the TX channel the keyer will use for notes and responses
  def set_tx_ch(self,v): self.txcc(MIDI_TX_CH,v)

  def set_keydown_note(self,v): self.txcc(MIDI_KEYDOWN_NOTE,v)
  def set_ptt_mic_note(self,v): self.txcc(MIDI_PTT_MIC_NOTE,v)
  def set_ptt_in_note(self,v): self.txcc(MIDI_PTT_IN_NOTE,v)
  def set_cw_headtail_note(self,v): self.txcc(MIDI_CW_HEADTAIL_NOTE,v)
  def set_cw_headtail_on_ring(self,v): self.txcc(MIDI_CW_HEADTAIL_ON_RING,v)

  ## Enable/disable MIDI responses
  def set_midi_response(self,v): self.txcc(MIDI_RESPONSE,v)

  ## Enable/disable POTs
  def set_enable_pots(self,v): self.txcc(MIDI_ENABLE_POTS,v)


if __name__ == "__main__":
  ## Available Output Ports
  print("Edit the top of this file to use the proper string names for send and receive ports on your system.")
  print("Available send device names:",mido.get_output_names())
  print("Available recv device names:",mido.get_input_names())
  k = CWKeyer(SEND,SCH,RECV,RCH)