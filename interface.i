#include <string>
// Reader provide a general Read method to read bytes from device, file, socket, etc.
interface Reader {
    long Read(void * buf, size_t size);
}

// Writer provide a general Write method to write bytes to device, file, socket, etc.
interface Writer {
    long Write(const void * buf, size_t size);
}

// ReadWriter means it can Read and Write.
interface ReadWriter extends Reader, Writer{}

// Stringify means it can be serialized to string.
interface Stringify extends Reader{
  std::string String() const;
}