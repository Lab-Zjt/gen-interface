interface Reader {
    R<ssize_t,Error> Read(void * buf, size_t size);
}

interface Writer {
    R<ssize_t,Error> Write(const void * buf, size_t size);
}

interface ReadWriter extends Reader, Writer{}

interface Stringify extends Reader{
  std::string String() const;
}