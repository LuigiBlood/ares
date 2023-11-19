struct SmartMedia : Cartridge {
  auto name() -> string override { return "SmartMedia"; }
  auto extensions() -> vector<string> override { return {"sm"}; }
  auto load(string location) -> bool override;
  auto save(string location) -> bool override;
  auto analyze(vector<u8>& data) -> string;
};

auto SmartMedia::load(string location) -> bool {
  vector<u8> input;
  if(directory::exists(location)) {
    append(input, {location, "program.flash"});
  } else if(file::exists(location)) {
    input = Cartridge::read(location);
  }
  if(!input) return false;

  this->sha256   = Hash::SHA256(input).digest();
  this->location = location;
  this->manifest = Medium::manifestDatabase(sha256);
  if(!manifest) manifest = analyze(input);
  auto document = BML::unserialize(manifest);
  if(!document) return false;

  pak = new vfs::directory;
  pak->setAttribute("title", document["game/title"].string());
  pak->append("manifest.bml", manifest);
  pak->append("program.flash", input);

  if(!pak) return false;

  Pak::load("program.flash", ".flash");

  return true;
}

auto SmartMedia::save(string location) -> bool {
  auto document = BML::unserialize(manifest);

  Pak::save("program.flash", ".flash");

  return true;
}

auto SmartMedia::analyze(vector<u8>& flash) -> string {
  if(flash.size() >= 0x1080000) {
    print("[mia] Loading flash failed. Maximum expected flash size is 17301504 (0x1080000) bytes. Flash size: ", flash.size(), " (0x", hex(flash.size()), ") bytes.\n");;
    return {};
  }

  string s;
  s += "game\n";
  s +={"  name:  ", Medium::name(location), "\n"};
  s +={"  title: ", Medium::name(location), "\n"};

  return s;
}
