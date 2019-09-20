#ifndef INTERFACE_I
#define INTERFACE_I
struct Dummy;

class Reader {
  template<typename _T>
        struct _Vtb {
          static _Vtb _vtb;
    ssize_t (_T::* Read)(void*, size_t);
  };
  Dummy* ptr;
  _Vtb<Dummy>* vtb;
 public:
template<typename _T>
  Reader(_T& t): ptr(reinterpret_cast<Dummy*>(&t)), vtb(reinterpret_cast<_Vtb<Dummy>*>(&_Vtb<_T>::_vtb)) {}
  template<typename ..._Args>
  ssize_t Read(_Args&& ..._args) const {
    return (ptr->*(vtb->Read))(std::forward<_Args>(_args)...);
  }
};

template<typename _T>
Reader::_Vtb<_T> Reader::_Vtb<_T>::_vtb = {&_T::Read, };

class Writer {
  template<typename _T>
        struct _Vtb {
          static _Vtb _vtb;
    ssize_t (_T::* Write)(const void*, size_t);
  };
  Dummy* ptr;
  _Vtb<Dummy>* vtb;
 public:
template<typename _T>
  Writer(_T& t): ptr(reinterpret_cast<Dummy*>(&t)), vtb(reinterpret_cast<_Vtb<Dummy>*>(&_Vtb<_T>::_vtb)) {}
  template<typename ..._Args>
  ssize_t Write(_Args&& ..._args) const {
    return (ptr->*(vtb->Write))(std::forward<_Args>(_args)...);
  }
};

template<typename _T>
Writer::_Vtb<_T> Writer::_Vtb<_T>::_vtb = {&_T::Write, };

class ReadWriter : public Reader,public Writer{
  template<typename _T>
        struct _Vtb {
          static _Vtb _vtb;
    ssize_t (_T::* ReadWrite)();
  };
  Dummy* ptr;
  _Vtb<Dummy>* vtb;
 public:
template<typename _T, typename _SFINAE = typename std::enable_if<!std::is_same<_T, ReadWriter>::value>::type>
  ReadWriter(_T& t): Reader(t), Writer(t), ptr(reinterpret_cast<Dummy*>(&t)), vtb(reinterpret_cast<_Vtb<Dummy>*>(&_Vtb<_T>::_vtb)) {}
  template<typename ..._Args>
  ssize_t ReadWrite(_Args&& ..._args) const {
    return (ptr->*(vtb->ReadWrite))(std::forward<_Args>(_args)...);
  }
};

template<typename _T>
ReadWriter::_Vtb<_T> ReadWriter::_Vtb<_T>::_vtb = {&_T::ReadWrite, };

#endif
