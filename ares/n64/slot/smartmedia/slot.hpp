struct SmartMediaSlot {
  Node::Port port;
  SmartMediaCard card;

  //slot.cpp
  SmartMediaSlot(string name);
  auto load(Node::Object) -> void;
  auto unload() -> void;

  const string name;
};

extern SmartMediaSlot smartmediaSlot1;
extern SmartMediaSlot smartmediaSlot2;
