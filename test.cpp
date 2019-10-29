#include <cstddef>
#include <array>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include "interface.h"

class File {
  int fd;
 public:
  File() = default;
  ~File() { close(fd); }
  static std::shared_ptr<File> Open(std::string_view filename, int mode = O_RDONLY, int perm = 0666) {
    int fd = open(filename.data(), mode, perm);
    if (fd < 0) return nullptr;
    auto f = std::make_shared<File>();
    f->fd = fd;
    return f;
  }
  ssize_t Read(void *buf, size_t size) { return read(fd, buf, size); }
  ssize_t Write(const void *buf, size_t size) { return write(fd, buf, size); }
};

ssize_t Copy(const Writer &wr, const Reader &rd) {
  std::vector<char> buf(4096);
  for (;;) {
    auto c = rd.Read(buf.data(), buf.size());
    if (c <= 0) return c;
    c = wr.Write(buf.data(), c);
    if (c <= 0) return c;
    if (c == buf.size()) { buf = std::vector<char>(buf.size() * 2); }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  auto f1 = File::Open(argv[1]);
  auto f2 = File::Open(argv[2], O_WRONLY | O_CREAT | O_TRUNC);
  Copy(f2, f1);
  return 0;
}
