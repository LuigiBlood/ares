//Debugger
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
    "PTP_DRIVE1_ECC_PAGE_HI",
    "PTP_DRIVE1_ECC_PAGE_LO",
    "PTP_DRIVE1_ECC_ERROR_HI",
    "PTP_DRIVE1_ECC_ERROR_LO",
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
    "PTP_DRIVE2_ECC_PAGE_HI",
    "PTP_DRIVE2_ECC_PAGE_LO",
    "PTP_DRIVE2_ECC_ERROR_HI",
    "PTP_DRIVE2_ECC_ERROR_LO",
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

//Drive
auto Cartridge::SmartMedia::Drive::setMagic(u16 seed) -> void {
  magic_seed = seed;

  u32 j, k, v;
  j = seed << 15;
  k = 0x47D78000;
  v = 0x40000000;
  while(v > 0x4001) {
    if (j&v) j ^= k;
    v >>= 1;
    k >>= 1;
  }
  j <<= 1;
  k = 1;
  v = 0x8000;
  while(v >= 2) {
    if (j&v) k = 1 - k;
    v >>= 1;
  }
  j |= k;

  magic_response = j & 0xffff;
}

auto Cartridge::SmartMedia::Drive::read(u32 address) -> u16 {
  n16 data = 0;

  if(unlock == 3) {
  if(address == 0) {
    data = 0x43;
    data.bit(2) = card.flash ? 0 : 1;
    data.bit(3) = card.readonly;
    data.bit(4) = status.busy_rdwr;
    data.bit(5) = status.busy_serial;
    data.bit(7) = status.busy_command;
  }
  if(address == 1) {
    data.bit(2) = card.flash ? 1 : 0;
  }
  if(address == 2) {
    data = 0xff;
  }
  if(address == 3) {
    data = card.read();
  }
  if(address == 4) {
  }
  if(address == 5) {
  }
  if(address == 6) {
  }
  if(address == 7) {
  }
  }
  if(address == 8) {
    if(magic_unlock == 2) data.bit(0,7) = magic_seed.bit(0,7);
  }
  if(address == 9) {
    if(magic_unlock == 2) data.bit(0,7) = magic_seed.bit(8,15);
  }
  if(address == 15) {
  }

  return (u16)data;
}

auto Cartridge::SmartMedia::Drive::write(u32 address, u16 data) -> void {
  n16 value = data;

  if(unlock == 3) {
  if(address == 0) {
    status.disableEcc = value.bit(5);
    status.cardBig =    value.bit(6);
    status.cardSmall =  value.bit(7);
  }
  if(address == 1) {
    card.write((u8)data, true, false);
  }
  if(address == 2) {
    card.write((u8)data, false, true);
  }
  if(address == 3) {
    card.write((u8)data, false, false);
  }
  if(address == 4) {
  }
  if(address == 5) {
  }
  if(address == 6) {
  }
  if(address == 7) {
  }
  }
  if(address == 8) {
    //magic_response
    if(magic_unlock == 2 && magic_response.bit(0,7) == value.bit(0,7)) unlock.bit(0) = 1;
  }
  if(address == 9) {
    //magic_response
    if(magic_unlock == 2 && magic_response.bit(8,15) == value.bit(0,7)) unlock.bit(1) = 1;
  }
  if(address == 15) {
    if(value == 1 && magic_unlock == 0) magic_unlock++;
    else if(value == 0 && magic_unlock == 1) magic_unlock++;
    else magic_unlock = 0;
  }
}

//SmartMedia
auto Cartridge::SmartMedia::load() -> void {
  if(self.pak->attribute("sm").boolean()) {
    self.has.SmartMediaCard = true;

    smartmediaSlot1.load(self.node);
    smartmediaSlot2.load(self.node);

    debugger.tracer = self.node->append<Node::Debugger::Tracer::Notification>("SmartMedia", "Cartridge");
    debugger.tracer->setAutoLineBreak(true);
    debugger.tracer->setTerminal(false);
    debugger.tracer->setFile(true);
    debugger.tracer->setPrefix(true);
  }
}

auto Cartridge::SmartMedia::unload() -> void {
  smartmediaSlot1.unload();
  smartmediaSlot2.unload();
}

auto Cartridge::SmartMedia::save() -> void {
  smartmedia1.save();
  smartmedia2.save();
}

auto Cartridge::SmartMedia::power(bool reset) -> void {
  drive1.card.power(reset);
  drive2.card.power(reset);

  drive1.magic_unlock = 0;
  drive2.magic_unlock = 0;
  drive1.unlock = 0;
  drive2.unlock = 0;

  drive1.setMagic(0xFFF0);
  drive2.setMagic(0xC000);
}

//read/write
auto Cartridge::SmartMedia::readHalf(u32 address) -> u16 {
  if((address&2)==0) return 0;
  address = (address & 0x1ff) >> 2;
  u16 data = 0;

  if ((address >= 0x00) && (address < 0x05)) data = 0; //encryption hardware (TODO)
  if ((address >= 0x40) && (address < 0x50)) data = drive1.read(address - 0x40);
  if ((address >= 0x50) && (address < 0x60)) data = drive2.read(address - 0x50);

  return data;
}

auto Cartridge::SmartMedia::readWord(u32 address) -> u32 {
  address = (address & 0xffff);
  n32 data;
  data.bit(16,31) = readHalf(address+0);
  data.bit( 0,15) = readHalf(address+2);
  debugger.io(Read, address, data);
  return (u32)data;
}

auto Cartridge::SmartMedia::writeHalf(u32 address, u16 data) -> void {
  if((address&2)==0) return;
  address = (address & 0x1ff) >> 2;

  if ((address >= 0x00) && (address < 0x05)) return; //encryption hardware (TODO)
  if ((address >= 0x40) && (address < 0x50)) drive1.write(address - 0x40, data);
  if ((address >= 0x50) && (address < 0x60)) drive2.write(address - 0x50, data);
}

auto Cartridge::SmartMedia::writeWord(u32 address, u32 data) -> void {
  address = (address & 0xffff);
  writeHalf(address+0, data >> 16);
  writeHalf(address+2, data & 0xffff);
  debugger.io(Write, address, data);
}
