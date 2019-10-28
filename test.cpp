#include <cstddef>
#include <array>
#include <cstring>
#include <thread>
#include "interface.h"

struct RW {
  // 实现Reader
  ssize_t Read(void *, size_t) { return 0; }
  // 实现Writer
  ssize_t Write(const void *, size_t) { return 0; }
  // 实现ReadWriter
  ssize_t ReadWrite() { return 0; }
};

void fr(const Reader &reader) {
  reader.Read(nullptr, 0);
}
void fw(const Writer &writer) {
  writer.Write(nullptr, 0);
}

void frw(const ReadWriter &read_writer) {
  fr(read_writer);
  fw(read_writer);
}

int main() {
  auto rw = std::make_shared<RW>();
  fr(rw);
  fw(rw);
  frw(rw);
  return 0;
}
