struct Cartridge {
  Node::Peripheral node;
  VFS::Pak pak;
  Memory::Readable16 rom;
  Memory::Writable16 ram;
  Memory::Writable16 eeprom;
  struct Flash : Memory::Writable {
    template<u32 Size>
    auto read(u32 address) -> u64 {
      if constexpr(Size == Byte) return readByte(address);
      if constexpr(Size == Half) return readHalf(address);
      if constexpr(Size == Word) return readWord(address);
      if constexpr(Size == Dual) return readDual(address);
      unreachable;
    }

    template<u32 Size>
    auto write(u32 address, u64 data) -> void {
      if constexpr(Size == Byte) return writeByte(address, data);
      if constexpr(Size == Half) return writeHalf(address, data);
      if constexpr(Size == Word) return writeWord(address, data);
      if constexpr(Size == Dual) return writeDual(address, data);
    }

    //flash.cpp
    auto readByte(u32 adddres) -> u64;
    auto readHalf(u32 address) -> u64;
    auto readWord(u32 address) -> u64;
    auto readDual(u32 address) -> u64;
    auto writeByte(u32 address, u64 data) -> void;
    auto writeHalf(u32 address, u64 data) -> void;
    auto writeWord(u32 address, u64 data) -> void;
    auto writeDual(u32 address, u64 data) -> void;

    enum class Mode : u32 { Idle, Erase, Write, Read, Status };
    Mode mode = Mode::Idle;
    u64  status = 0;
    u32  source = 0;
    u32  offset = 0;
  } flash;
  struct ISViewer : Memory::PI<ISViewer> {
    Memory::Writable ram;  //unserialized
    Node::Debugger::Tracer::Notification tracer;

    //isviewer.cpp
    auto messageChar(char c) -> void;
    auto readHalf(u32 address) -> u16;
    auto writeHalf(u32 address, u16 data) -> void;
    auto readWord(u32 address) -> u32;
    auto writeWord(u32 address, u32 data) -> void;
  } isviewer;

  struct RTC {
    Cartridge& self;
    RTC(Cartridge &self) : self(self) {}

    Memory::Writable ram;
    n1 present;
    n8 status;
    n3 writeLock;

    // rtc.cpp
    auto power(bool reset) -> void;
    auto run(bool run) -> void;
    auto running() -> bool;
    auto load() -> void;
    auto save() -> void;
    auto tick(int nsec=1) -> void;
    auto advance(int nsec) -> void;
    auto serialize(serializer& s) -> void;
    auto read(u2 block, n8 *data) -> void;
    auto write(u2 block, n8 *data) -> void;
  } rtc{*this};

  struct SmartMedia : Memory::PI<SmartMedia> {
    Cartridge& self;
    SmartMedia(Cartridge &self) : self(self) {}

    struct Debugger {
      Node::Debugger::Tracer::Notification tracer;

      auto io(bool mode, u32 address, u32 data) -> void;
    } debugger;

    struct Drive {
      ares::Nintendo64::SmartMediaCard& card;
      Drive(ares::Nintendo64::SmartMediaCard &card) : card(card) {}

      struct {
        n1 busy_rdwr;
        n1 busy_serial;
        n1 busy_command;
      } status;

      struct {
        n1 disableEcc;
        n1 cardBig;     //64MB and above
        n1 cardSmall;   //2MB and below

        n1 command_clock;
        u8 address_clock;
        u16 address;

        n24 ecc_lo_compare;
        n24 ecc_hi_compare;

        //result
        u8 ecc_lo_addr;
        u8 ecc_hi_addr;
        u8 ecc_lo_bit;
        u8 ecc_hi_bit;
      } error;

      struct EccHardware {
        const u8 ecc_table[256] = {
          0x00, 0x55, 0x56, 0x03, 0x59, 0x0C, 0x0F, 0x5A, 0x5A, 0x0F, 0x0C, 0x59, 0x03, 0x56, 0x55, 0x00,
          0x65, 0x30, 0x33, 0x66, 0x3C, 0x69, 0x6A, 0x3F, 0x3F, 0x6A, 0x69, 0x3C, 0x66, 0x33, 0x30, 0x65,
          0x66, 0x33, 0x30, 0x65, 0x3F, 0x6A, 0x69, 0x3C, 0x3C, 0x69, 0x6A, 0x3F, 0x65, 0x30, 0x33, 0x66,
          0x03, 0x56, 0x55, 0x00, 0x5A, 0x0F, 0x0C, 0x59, 0x59, 0x0C, 0x0F, 0x5A, 0x00, 0x55, 0x56, 0x03,
          0x69, 0x3C, 0x3F, 0x6A, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6A, 0x3F, 0x3C, 0x69,
          0x0C, 0x59, 0x5A, 0x0F, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0F, 0x5A, 0x59, 0x0C,
          0x0F, 0x5A, 0x59, 0x0C, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0C, 0x59, 0x5A, 0x0F,
          0x6A, 0x3F, 0x3C, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3C, 0x3F, 0x6A,
          0x6A, 0x3F, 0x3C, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3C, 0x3F, 0x6A,
          0x0F, 0x5A, 0x59, 0x0C, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0C, 0x59, 0x5A, 0x0F,
          0x0C, 0x59, 0x5A, 0x0F, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0F, 0x5A, 0x59, 0x0C,
          0x69, 0x3C, 0x3F, 0x6A, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6A, 0x3F, 0x3C, 0x69,
          0x03, 0x56, 0x55, 0x00, 0x5A, 0x0F, 0x0C, 0x59, 0x59, 0x0C, 0x0F, 0x5A, 0x00, 0x55, 0x56, 0x03,
          0x66, 0x33, 0x30, 0x65, 0x3F, 0x6A, 0x69, 0x3C, 0x3C, 0x69, 0x6A, 0x3F, 0x65, 0x30, 0x33, 0x66,
          0x65, 0x30, 0x33, 0x66, 0x3C, 0x69, 0x6A, 0x3F, 0x3F, 0x6A, 0x69, 0x3C, 0x66, 0x33, 0x30, 0x65,
          0x00, 0x55, 0x56, 0x03, 0x59, 0x0C, 0x0F, 0x5A, 0x5A, 0x0F, 0x0C, 0x59, 0x03, 0x56, 0x55, 0x00
          };
        
        u16 address;
        n24 calc;

        u8 eccAddr;
        u8 eccBit;
        
        auto reset() -> void;
        auto input(u8 data) -> void;
        auto eccArea() -> n24;

        auto correct(n24 eccCompare) -> void;
        auto addr() -> u8;
        auto bit() -> u8;
      };

      struct EccHardware ecc_lo{};
      struct EccHardware ecc_hi{};

      n2 magic_unlock;
      n16 magic_seed;
      n16 magic_response;

      n2 unlock;

      auto setMagic(u16 seed) -> void;
      auto read(u32 address) -> u16;
      auto write(u32 address, u16 data) -> void;
      auto power(bool reset) -> void;
    };

    struct Drive drive1{smartmedia1};
    struct Drive drive2{smartmedia2};

    auto load() -> void;
    auto unload() -> void;
    auto save() -> void;
    auto power(bool reset) -> void;

    auto readHalf(u32 address) -> u16;
    auto writeHalf(u32 address, u16 data) -> void;
    auto readWord(u32 address) -> u32;
    auto writeWord(u32 address, u32 data) -> void;
  } smartmedia{*this};

  struct Debugger {
    //debugger.cpp
    auto load(Node::Object) -> void;
    auto unload(Node::Object) -> void;

    struct Memory {
      Node::Debugger::Memory rom;
      Node::Debugger::Memory ram;
      Node::Debugger::Memory eeprom;
      Node::Debugger::Memory flash;
      Node::Debugger::Memory smartmedia;
    } memory;
  } debugger;

  struct Has {
    boolean SmartMediaCard;
  } has;


  auto title() const -> string { return information.title; }
  auto region() const -> string { return information.region; }
  auto cic() const -> string { return information.cic; }

  //cartridge.cpp
  auto allocate(Node::Port) -> Node::Peripheral;
  auto connect() -> void;
  auto disconnect() -> void;
  auto save() -> void;
  auto power(bool reset) -> void;

  //joybus.cpp
  auto joybusComm(n8 send, n8 recv, n8 input[], n8 output[]) -> n2;

  //serialization.cpp
  auto serialize(serializer&) -> void;

private:
  struct Information {
    string title;
    string region;
    string cic;
  } information;
};

#include "slot.hpp"
extern Cartridge& cartridge;
