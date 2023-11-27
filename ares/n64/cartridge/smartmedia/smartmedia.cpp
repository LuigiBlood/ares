#include "debugger.cpp"
#include "drive.cpp"
#include "error.cpp"

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
  drive1.power(reset);
  drive2.power(reset);

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
