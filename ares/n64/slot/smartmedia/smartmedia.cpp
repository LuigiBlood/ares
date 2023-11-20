SmartMediaCard& smartmedia1 = smartmediaSlot1.card;
SmartMediaCard& smartmedia2 = smartmediaSlot2.card;
#include "slot.cpp"
#include "flash.cpp"
#include "serialization.cpp"

auto SmartMediaCard::allocate(Node::Port parent) -> Node::Peripheral {
  return node = parent->append<Node::Peripheral>("SmartMedia Card");
}

auto SmartMediaCard::connect() -> void {
  if(!node->setPak(pak = platform->pak(node))) return;
  information = {};
  information.title = pak->attribute("title");

  buffer.allocate(528);

  if(auto fp = pak->read("program.flash")) {
    flash.allocate(fp->size());
    flash.load(fp);
  }

  if(flash) {
    manufacturer = 0x98; //Toshiba
    if(flash.size == 0x108000*1) {
      pageSize = 256+8;
      deviceType = 0xEC;
    }
    if(flash.size == 0x108000*2) {
      pageSize = 256+8;
      deviceType = 0xEA;
    }
    if(flash.size == 0x108000*4) {
      pageSize = 512+16;
      deviceType = 0xE3;
    }
    if(flash.size == 0x108000*8) {
      pageSize = 512+16;
      deviceType = 0xED;
    }
    if(flash.size == 0x108000*16) {
      pageSize = 512+16;
      deviceType = 0x73;
    }
  }
}

auto SmartMediaCard::disconnect() -> void {
  if(!node) return;
  save();
  flash.reset();
  buffer.reset();
  pak.reset();
  node.reset();
}

auto SmartMediaCard::power(bool reset) -> void {
  mode = Mode::Idle;
  pointerMode = PointerMode::Normal;
  address = 0;
  addressClock = 0;
  pageAddress = 0;
  sectorAddress = 0;

  status.fail = 0;
  status.ready = 1;
  status.disabledWriteProtect = 1;
}

auto SmartMediaCard::save() -> void {
  if(!node) return;

  if(auto fp = pak->write("program.flash")) {
    flash.save(fp);
  }
}
