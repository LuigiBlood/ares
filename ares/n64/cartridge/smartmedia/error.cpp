auto Cartridge::SmartMedia::Drive::EccHardware::reset() -> void {
  address = 0;
  calc = 0;

  eccAddr = 0;
  eccBit = 0;
}

auto Cartridge::SmartMedia::Drive::EccHardware::input(u8 data) -> void {
  auto temp = ecc_table[(u8)data];
  calc.bit(2,7) ^= (temp & 0x3f);
  if((temp & 0x40) != 0) {
    calc.bit(8,15) ^= address;
    calc.bit(16,23) ^= ~address;
  }
  address++;
}

auto Cartridge::SmartMedia::Drive::EccHardware::eccArea() -> n24 {
  n24 temp = 0;

  for(int i : range(8)) {
    temp.bit(23 - (i * 2) - 0) = ~calc.bit(15 - i);
    temp.bit(23 - (i * 2) - 1) = ~calc.bit(23 - i);
  }
  temp.bit(0,7) = ~calc.bit(0,7);

  return temp;
}

auto Cartridge::SmartMedia::Drive::EccHardware::correct(n24 eccCompare) -> void {
  n24 temp = eccArea();
  temp ^= eccCompare;
  if(temp == 0) {
    //no problem
    eccAddr = 0;
    eccBit = 0;
  } else if(((temp ^ (temp>>1)) & 0x555554) == 0x555554) {
    //correctable
    n8 addr = 0;
    for (int i : range(8)) {
      if(temp.bit(23 - (i * 2))) addr.bit(7 - i) = 1;
    }
    n8 bit = 0;
    for (int i : range(3)) {
      if(temp.bit(7 - (i * 2))) bit.bit(2 - i) = 1;
    }
    eccAddr = (u8)addr;
    eccBit = (u8)bit;
  } else {
    eccAddr = 0xff;
    eccBit = 0xff;
  }
}

auto Cartridge::SmartMedia::Drive::EccHardware::addr() -> u8 {
  return eccAddr;
}

auto Cartridge::SmartMedia::Drive::EccHardware::bit() -> u8 {
  return eccBit;
}
