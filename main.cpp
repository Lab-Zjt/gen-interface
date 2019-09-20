#include <iostream>
#include "interface.h"
struct Dummy {};

struct RW {
  ssize_t Read(void *, size_t) { return 0; }
  ssize_t Write(const void *, size_t) { return 0; }
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
  RW rw;
  fr(rw);
  fw(rw);
  frw(rw);
  return 0;
}