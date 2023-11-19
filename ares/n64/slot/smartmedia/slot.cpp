SmartMediaSlot smartmediaSlot1{"SmartMedia Card Slot 1"};
SmartMediaSlot smartmediaSlot2{"SmartMedia Card Slot 2"};

SmartMediaSlot::SmartMediaSlot(string name) : name(name) {
}

auto SmartMediaSlot::load(Node::Object parent) -> void {
  port = parent->append<Node::Port>(name);
  port->setFamily("SmartMedia");
  port->setType("Card");
  port->setAllocate([&](auto name) { return card.allocate(port); });
  port->setConnect([&] { return card.connect(); });
  port->setDisconnect([&] { return card.disconnect(); });
}

auto SmartMediaSlot::unload() -> void {
  card.disconnect();
  port.reset();
}
