auto Cartridge::SmartMedia::Debugger::io(bool mode, u32 address, u32 data) -> void {
  address = (address & 0x1ff) >> 2;
  if(address >= 0x40) address -= 0x3B;

  static const vector<string> registerNames = {
    "PTP_ENC_HW4",
    "PTP_ENC_HW3",
    "PTP_ENC_HW2",
    "PTP_ENC_HW1",
    "PTP_ENC_CMD",
    "PTP_DRIVE1_STATUS",
    "PTP_DRIVE1_CMD",
    "PTP_DRIVE1_ADDR",
    "PTP_DRIVE1_DATA",
    "PTP_DRIVE1_ECC_ADDR_HI",
    "PTP_DRIVE1_ECC_ADDR_LO",
    "PTP_DRIVE1_ECC_BIT_HI",
    "PTP_DRIVE1_ECC_BIT_LO",
    "PTP_DRIVE1_MAGIC_LO",
    "PTP_DRIVE1_MAGIC_HI",
    "PTP_DRIVE1_UNKNOWN",
    "PTP_DRIVE1_UNKNOWN",
    "PTP_DRIVE1_UNKNOWN",
    "PTP_DRIVE1_UNKNOWN",
    "PTP_DRIVE1_UNKNOWN",
    "PTP_DRIVE1_UNLOCK",
    "PTP_DRIVE2_STATUS",
    "PTP_DRIVE2_CMD",
    "PTP_DRIVE2_ADDR",
    "PTP_DRIVE2_DATA",
    "PTP_DRIVE2_ECC_ADDR_HI",
    "PTP_DRIVE2_ECC_ADDR_LO",
    "PTP_DRIVE2_ECC_BIT_HI",
    "PTP_DRIVE2_ECC_BIT_LO",
    "PTP_DRIVE2_MAGIC_LO",
    "PTP_DRIVE2_MAGIC_HI",
    "PTP_DRIVE2_UNKNOWN",
    "PTP_DRIVE2_UNKNOWN",
    "PTP_DRIVE2_UNKNOWN",
    "PTP_DRIVE2_UNKNOWN",
    "PTP_DRIVE2_UNKNOWN",
    "PTP_DRIVE2_UNLOCK",
  };

  if(unlikely(tracer->enabled())) {
    string message;
    string name = registerNames(address, "PTP_UNKNOWN");
    if(mode == Read) {
      message = {name.split("|").first(), " => ", hex(data, 8L)};
    }
    if(mode == Write) {
      message = {name.split("|").last(), " <= ", hex(data, 8L)};
    }
    tracer->notify(message);
  }
}
