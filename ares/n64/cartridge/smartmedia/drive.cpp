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
    if(!error.disableEcc && error.command_clock) {
      //do ecc calc
      if(error.address < 256) {
        auto temp = error.ecc_table[(u8)data];
        error.ecc_lo_calc.bit(2,7) ^= (temp & 0x3f);
        if((temp & 0x40) != 0) {
          error.ecc_lo_calc.bit(8,15) ^= error.address;
          error.ecc_lo_calc.bit(16,23) ^= ~error.address;
        }
      }

      if((error.address >= error.cardSmall ? 256+8 : 256)
      && (error.address <  error.cardSmall ? 512+8 : 512)) {
        auto temp = error.ecc_table[(u8)data];
        error.ecc_hi_calc.bit(2,7) ^= (temp & 0x3f);
        if((temp & 0x40) != 0) {
          error.ecc_hi_calc.bit(8,15) ^= (error.address - error.cardSmall ? 256+8 : 256);
          error.ecc_hi_calc.bit(16,23) ^= ~(error.address - error.cardSmall ? 256+8 : 256);
        }
      }

      if(error.address == 520) error.ecc_hi_compare.bit(8,15) = data;
      if(error.address == 521) error.ecc_hi_compare.bit(16,23) = data;
      if(error.address == 522) error.ecc_hi_compare.bit(0,7) = data;

      if(error.address == 525) error.ecc_lo_compare.bit(8,15) = data;
      if(error.address == 526) error.ecc_lo_compare.bit(16,23) = data;
      if(error.address == 527) error.ecc_lo_compare.bit(0,7) = data;

      error.address++;

      if(error.address == 528) {
        error.address = 0;

        //do compare
        n24 temp_hi, temp_lo = 0;
        //convert
        for(int i : range(8)) {
          temp_hi.bit(23 - (i * 2) - 0) = ~error.ecc_hi_calc.bit(15 - i);
          temp_hi.bit(23 - (i * 2) - 1) = ~error.ecc_hi_calc.bit(23 - i);

          temp_lo.bit(23 - (i * 2) - 0) = ~error.ecc_lo_calc.bit(15 - i);
          temp_lo.bit(23 - (i * 2) - 1) = ~error.ecc_lo_calc.bit(23 - i);
        }
        temp_hi.bit(0,7) = ~error.ecc_hi_calc.bit(0,7);
        temp_lo.bit(0,7) = ~error.ecc_lo_calc.bit(0,7);

        //error.ecc_lo_addr = temp_lo.bit(16,23);
        //error.ecc_hi_addr = temp_lo.bit(8,15);
        
        //attempt ecc (hi)
        temp_hi ^= error.ecc_hi_compare;
        if(temp_hi == 0) {
          //no problem
          error.ecc_hi_addr = 0;
          error.ecc_hi_bit = 0;
        } else if(((temp_hi ^ (temp_hi>>1)) & 0x555554) == 0x555554) {
          //correctable
          n8 addr = 0;
          for (int i : range(8))
            if(temp_hi.bit(23 - (i * 2))) addr.bit(7 - i) = 1;
          n8 bit = 0;
          for (int i : range(3))
            if(temp_hi.bit(7 - (i * 2))) bit.bit(2 - i) = 1;
          error.ecc_hi_addr = (u8)addr;
          error.ecc_hi_bit = (u8)bit;
        } else {
          error.ecc_hi_addr = 0xff;
          error.ecc_hi_bit = 0xff;
        }

        //attempt ecc (lo)
        temp_lo ^= error.ecc_lo_compare;
        if(temp_lo == 0) {
          //no problem
          error.ecc_lo_addr = 0;
          error.ecc_lo_bit = 0;
        } else if(((temp_lo ^ (temp_lo>>1)) & 0x555554) == 0x555554) {
          //correctable
          n8 addr = 0;
          for (int i : range(8))
            if(temp_lo.bit(23 - (i * 2))) addr.bit(7 - i) = 1;
          n8 bit = 0;
          for (int i : range(3))
            if(temp_lo.bit(7 - (i * 2))) bit.bit(2 - i) = 1;
          error.ecc_lo_addr = (u8)addr;
          error.ecc_lo_bit = (u8)bit;
        } else {
          error.ecc_lo_addr = 0xff;
          error.ecc_lo_bit = 0xff;
        }
      }
    }
  }
  if(address == 4) {
    data = error.ecc_hi_addr;
  }
  if(address == 5) {
    data = error.ecc_lo_addr;
  }
  if(address == 6) {
    data = error.ecc_hi_bit;
  }
  if(address == 7) {
    data = error.ecc_lo_bit;
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
    error.disableEcc = value.bit(5);
    error.cardBig =    value.bit(6);
    error.cardSmall =  value.bit(7);

    error.ecc_lo_addr = 0;
    error.ecc_hi_addr = 0;
    error.ecc_lo_bit = 0;
    error.ecc_hi_bit = 0;
  }
  if(address == 1) {
    card.write((u8)data, true, false);
    error.command_clock = 0;
    if(!error.disableEcc && data == 0) {
      error.command_clock = 1;
      error.address_clock = 0;
      error.address = 0;
      error.ecc_lo_calc = 0;
      error.ecc_hi_calc = 0;
      error.ecc_lo_addr = 0;
      error.ecc_hi_addr = 0;
      error.ecc_lo_bit = 0;
      error.ecc_hi_bit = 0;
    }
  }
  if(address == 2) {
    card.write((u8)data, false, true);
    if(!error.disableEcc && error.command_clock) {
      if(error.address_clock == 0) address = (u8)data;
      //if(error.address_clock == 1) address += ((u8)data & 1) * error.cardSmall ? (256+8) : (512+16);
      if(error.address_clock != 3) error.address_clock++;
    }
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

auto Cartridge::SmartMedia::Drive::power(bool reset) -> void {
  card.power(reset);
  magic_unlock = 0;
  unlock = 0;

  error.command_clock = 0;
  error.address_clock = 0;
  error.address = 0;
  error.ecc_lo_addr = 0;
  error.ecc_hi_addr = 0;
  error.ecc_lo_bit = 0;
  error.ecc_hi_bit = 0;
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
