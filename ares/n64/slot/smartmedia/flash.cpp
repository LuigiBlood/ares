auto SmartMediaCard::read() -> u8 {
  u8 data = 0xFF;
  switch(mode) {
    case Mode::Read:
      if(addressClock!=3) break;
      data = flash.read<Byte>(address++);
      if(pointerMode == PointerMode::PageInfo) {
        if((address % pageSize) == 0) {
          // go to next page info
          address += 256;
          if(pageSize == 528) address += 256;
        }
      }
      break;
    case Mode::ReadId:
      if(addressClock!=1) break;
      if(address==0) data = 0x98; //Manufacturer: Toshiba
      if(address==1) data = 0xEA; //Device Type:  2MB 3.3v (To be changed)
      if(address <2) address++;
      break;
    case Mode::Status:
      if(addressClock!=0) break;
      data = 0;
      if(status.fail)                 data |= 0x01;
      if(status.ready)                data |= 0x40;
      if(status.disabledWriteProtect) data |= 0x80;
      break;
  }
  return data;
}

auto SmartMediaCard::write(u8 value, bool cmd_latch, bool addr_latch) -> void {
  if(cmd_latch) {
    switch(value) {
      case Command::Reset:
        power(true);
        break;
      case Command::ReadPage0:
        pointerMode = PointerMode::Normal;
        sectorAddress = 0;
        address = sectorAddress;
        addressClock = 0;
        mode = Mode::Read;
        break;
      case Command::ReadPage1:
        pointerMode = PointerMode::Normal;
        sectorAddress = 256;
        address = sectorAddress;
        addressClock = 0;
        mode = Mode::Read;
        break;
      case Command::ReadPageInfo:
        pointerMode = PointerMode::PageInfo;
        sectorAddress = 256;
        if(pageSize==528) sectorAddress = 512;
        address = sectorAddress;
        addressClock = 0;
        mode = Mode::Read;
        break;
      case Command::ReadId:
        address = 0;
        addressClock = 0;
        mode = Mode::ReadId;
        break;
      case Command::WritePageBuffer:
        address = sectorAddress;
        addressClock = 0;
        bufferAddress = 0;
        mode = Mode::Write;
        break;
      case Command::WritePageConfirm:
        if(mode == Mode::Write) {
          for(u32 i : range(bufferAddress)) {
            flash.write<Byte>(address++, buffer.read<Byte>(i));

            if(pointerMode == PointerMode::PageInfo) {
              if((address % pageSize) == 0) {
                // go to next page info
                address += 256;
                if(pageSize == 528) address += 256;
              }
            } else {
              sectorAddress = 0;
            }
          }
          mode = Mode::Idle;
        }
        break;
      case Command::EraseBlock:
        address = 0;
        addressClock = 0;
        mode = Mode::Erase;
        break;
      case Command::EraseBlockConfirm:
        if(mode == Mode::Erase) {
          for(u32 i : range(pageSize * 16)) {
            flash.write<Byte>(address + i, 0xff);
          }
          if(pointerMode == PointerMode::Normal) {
            sectorAddress = 0;
          }
          mode = Mode::Idle;
        }
        break;
      case Command::GetStatus:
        address = 0;
        addressClock = 0;
        mode = Mode::Status;
        break;
    }
    return;
  }
  if(addr_latch) {
    switch(mode) {
      case Mode::Erase:
        if(addressClock==0) pageAddress = value;
        if(addressClock==1) {
          pageAddress += (value << 8);
          pageAddress &= ~0xF;
          address += pageAddress * pageSize;
        }
        if(addressClock==2) return;
        break;
      case Mode::Write:
        if(addressClock==0) address += value;
        if(addressClock==1) pageAddress = value;
        if(addressClock==2) {
          pageAddress += (value << 8);
          address += pageAddress * pageSize;
        }
        if(addressClock==3) return;
        break;
      case Mode::Read:
        if(addressClock==0) address += value;
        if(addressClock==1) pageAddress = value;
        if(addressClock==2) {
          pageAddress += (value << 8);
          address += pageAddress * pageSize;
        }
        if(addressClock==3) return;
        break;
      case Mode::ReadId:
        if(addressClock==1) return;
        break;
      case Mode::Status:
        if(addressClock==0) return;
        break;
    }
    addressClock++;
    return;
  }
  //data latch
  switch(mode) {
    case Mode::Write:
      //write to buffer
      if(addressClock != 3) return;
      if(bufferAddress >= pageSize) return;
      buffer.write<Byte>(bufferAddress++, value);
      break;
  }
}
