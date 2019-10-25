# gen-interface

C++非侵入式接口代码生成。

## 接口定义

通过`interface 接口名`声明一个接口，声明体内部有复数个方法定义、继承定义，格式如下（不要在声明体内部写注释，暂时解析不了QAQ）：

```i
interface Reader {
    // 方法定义
    ssize_t Read(void*, size_t);
}

interface Writer {
    // 方法定义
    ssize_t Write(const void*, size_t);
}

// 继承定义
interface ReadWriter extends Reader, Writer {
    // 方法定义
    ssize_t ReadWrite();
}

```

## 接口代码生成

```sh
mkdir build
cd build
cmake ..
make
./gen_interface ../interface.i > ../interface.h
```

## 使用

通过这种方法定义的接口无需显示声明，只要定义了对应的方法就可以当成接口类型使用：

```C++
#include <iostream>
#include "interface.h"
struct Dummy {};

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
  RW rw;
  fr(rw);
  fw(rw);
  frw(rw);
  return 0;
}
```

这种模拟虚函数表的方法带来了额外的开销，一是每个基类都包含ptr指针，使得派生类会有多个该指针；二是编译器无法针对这种间接调用进行优化（相对于虚函数来说）。

调用开销理论上和未优化的虚函数调用相同，需要一次查询虚函数表获得方法指针，然后才能调用方法。

因此这种trick不应该放到任何生产环境下使用，还是应该老老实实地声明纯虚接口QwQ

Enjoy C++ :)

## Reference

[Go-Style Interfaces in C++](http://wall.org/~lewis/2012/07/23/go-style-interfaces-in-cpp.html)
