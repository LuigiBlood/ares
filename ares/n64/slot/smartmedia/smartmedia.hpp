struct SmartMediaCard {
  Node::Peripheral node;
  VFS::Pak pak;
  
  Memory::Writable flash;
  Memory::Writable buffer;

  n1 readonly = 0;

  auto title() const -> string { return information.title; }

  struct Command { enum : u8 {
      ReadPage0          = 0x00,  //read mode (page 0)
      ReadPage1          = 0x01,  //read mode (page 1) (4MB and above only)
      ReadPageInfo       = 0x50,  //read mode (page info)
      ReadId             = 0x90,  //read mode (card id)
      WritePageBuffer    = 0x80,  //start write mode (write to buffer)
      WritePageConfirm   = 0x10,  //confirm write (write buffer to page)
      EraseBlock         = 0x60,  //erase mode (first step)
      EraseBlockConfirm  = 0xd0,  //erase mode (second step, start)
      GetStatus          = 0x70,  //get status
      Reset              = 0xff,  //reset
    };};

  //flash.cpp
  auto read() -> u8;
  auto write(u8 value, bool cmd_latch, bool addr_latch) -> void;

  //smartmedia.cpp
  auto allocate(Node::Port) -> Node::Peripheral;
  auto connect() -> void;
  auto disconnect() -> void;

  auto power(bool reset) -> void;
  auto save() -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  struct {
    string title;
  } information;

private:
  enum class Mode : u32 { Idle, Erase, Write, Read, ReadId, Status };
  Mode mode = Mode::Idle;

  enum class PointerMode : u32 { Normal, PageInfo };
  PointerMode pointerMode = PointerMode::Normal;

  struct {
    n1 fail;
    n1 ready;
    n1 disabledWriteProtect;
  } status;

  u8 manufacturer;
  u8 deviceType;

  u32 address;
  u32 sectorAddress;
  u32 pageAddress;
  u32 bufferAddress;
  u8 addressClock;
  n1 dataReady;

  u16 pageSize;
};

#include "slot.hpp"
extern SmartMediaCard& smartmedia1;
extern SmartMediaCard& smartmedia2;
