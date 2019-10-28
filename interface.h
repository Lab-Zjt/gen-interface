#ifndef INTERFACE_I
#define INTERFACE_I
#include <utility>
#if __cplusplus < 201703ul
template <typename T, typename U>
std::shared_ptr<T> reinterpret_pointer_cast(const std::shared_ptr<U>& r) noexcept  {
  auto p = reinterpret_cast<typename std::shared_ptr<T>::element_type *>(r.get());
  return std::shared_ptr<T>(r, p);
}
#else
using std::reinterpret_pointer_cast;
#endif
#include <string>
// Reader provide a general Read method to read bytes from device, file, socket, etc.
class Reader{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    long  (_T::* Read)(void *  buf, size_t  size);
  };
  std::shared_ptr<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  Reader(const std::shared_ptr<_T>& t): ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  long  Read(void *  buf, size_t  size) const {
    return ((ptr.get())->*(vtb->Read))(buf, size);
  }
};
template<typename _T>
Reader::_Vtb<_T> Reader::_Vtb<_T>::_vtb = {&_T::Read};


// Writer provide a general Write method to write bytes to device, file, socket, etc.
class Writer{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    long  (_T::* Write)(const void *  buf, size_t  size);
  };
  std::shared_ptr<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  Writer(std::shared_ptr<_T> t): ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  long  Write(const void *  buf, size_t  size) const {
    return ((ptr.get())->*(vtb->Write))(buf, size);
  }
};
template<typename _T>
Writer::_Vtb<_T> Writer::_Vtb<_T>::_vtb = {&_T::Write};


// ReadWriter means it can Read and Write.
class ReadWriter:public Reader, public Writer{
 public:
  template<typename _T>
  ReadWriter(std::shared_ptr<_T> t): Reader(t),Writer(t) {}
};


// Stringify means it can be serialized to string.
class Stringify:public Reader{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    std::string  (_T::* String)();
  };
  std::shared_ptr<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  Stringify(std::shared_ptr<_T> t): Reader(t), ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  std::string  String() const {
    return ((ptr.get())->*(vtb->String))();
  }
};
template<typename _T>
Stringify::_Vtb<_T> Stringify::_Vtb<_T>::_vtb = {&_T::String};
#endif
