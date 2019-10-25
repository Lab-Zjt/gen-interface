#ifndef _INTERFACE_BASIC_
#define _INTERFACE_BASIC_
#include <memory>
#include <tuple>
template<typename T, typename ...Args>
inline std::shared_ptr<T> New(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}
template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T>
using Ptr = std::unique_ptr<T>;
template<typename T>
using WeakRef = std::weak_ptr<T>;
template<typename ...Ts>
using R = std::tuple<Ts...>;
struct Error {
  const char *desc = nullptr;
};
bool operator==(const Error &lhs, const Error &rhs) { return lhs.desc == rhs.desc; }
bool operator!=(const Error &lhs, const Error &rhs) { return lhs.desc != rhs.desc; }
Error NoError;
#endif
#ifndef ___INTERFACE_I
#define ___INTERFACE_I
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
class Reader{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    R<ssize_t,Error>  (_T::* Read)(void *  buf, size_t  size);
  };
  Ref<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  Reader(Ref<_T> t): ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  R<ssize_t,Error>  Read(void *  buf, size_t  size) {
    return ((ptr.get())->*(vtb->Read))(buf, size);
  }
};
template<typename _T>
Reader::_Vtb<_T> Reader::_Vtb<_T>::_vtb = {&_T::Read};
class Writer{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    R<ssize_t,Error>  (_T::* Write)(const void *  buf, size_t  size);
  };
  Ref<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  Writer(Ref<_T> t): ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  R<ssize_t,Error>  Write(const void *  buf, size_t  size) {
    return ((ptr.get())->*(vtb->Write))(buf, size);
  }
};
template<typename _T>
Writer::_Vtb<_T> Writer::_Vtb<_T>::_vtb = {&_T::Write};
class ReadWriter:public Reader, public Writer{
 public:
  template<typename _T>
  ReadWriter(Ref<_T> t): Reader(t),Writer(t) {}
};
class Stringify:public Reader{
  struct _Dummy{};
  template<typename _T>
  struct _Vtb {
    static _Vtb _vtb;
    std::string  (_T::* String)();
  };
  Ref<_Dummy> ptr;
  _Vtb<_Dummy>* vtb;
 public:
  template<typename _T>
  Stringify(Ref<_T> t): Reader(t), ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb)) {}
  std::string  String() {
    return ((ptr.get())->*(vtb->String))();
  }
};
template<typename _T>
Stringify::_Vtb<_T> Stringify::_Vtb<_T>::_vtb = {&_T::String};
#endif
