#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "interface.h"

#define ERRNO Error{strerror(errno)}

class File {
 private:
  int fd;
 public:
  static R<Ref<File>, Error> Open(const char *filename) {
    auto f = New<File>();
    f->fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (f->fd < 0) {
      return {nullptr, ERRNO};
    }
    return {f, NoError};
  }
 public:
  R<ssize_t, Error> Read(void *buf, size_t size) {
    auto c = read(fd, buf, size);
    if (c < 0) {
      return {c, ERRNO};
    }
    return {c, NoError};
  }
  R<ssize_t, Error> Write(const void *buf, size_t size) {
    auto c = write(fd, buf, size);
    if (c < 0) {
      return {c, ERRNO};
    }
    return {c, NoError};
  }
};

void ReadAndWrite(ReadWriter rw) {
  char buf[33];
  auto[c1, err1] = rw.Read(buf, 32);
  if (err1 != NoError) {
    printf("read failed. %s\n", err1.desc);
    return;
  }
  printf("read %d\n%.*s", int(c1), int(c1), buf);
  auto[c2, err2] = rw.Write("hello, world", 12);
  if (err2 != NoError) {
    printf("write failed. %s\n", err2.desc);
    return;
  }
  printf("write %d\n", int(c1));
}

int main() {
  auto[f, err] = File::Open("../interface.h");
  if (err != NoError) {
    printf("open file failed. %s\n", err.desc);
    return 0;
  }
  ReadAndWrite(f);
}