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
    /*pageSize = 528;
    if(flash.size() <= 0x210000) pageSize = 264;*/
    pageSize = 264;
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
