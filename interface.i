interface Reader {
    ssize_t Read(void*, size_t)
}

interface Writer {
    ssize_t Write(const void*, size_t)
}

interface ReadWriter {
    Reader
    Writer
    ssize_t ReadWrite()
}

