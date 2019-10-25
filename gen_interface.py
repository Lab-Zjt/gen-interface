#!/usr/bin/python3.6
# -*- coding: utf-8 -*-

import re
import pathlib
import sys

# 读取接口定义文件
interface_declare = pathlib.Path(sys.argv[1]).read_text()

# 共有定义

print('''
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
''')

# 头文件重复包含保护
protected = str(sys.argv[1]).replace('/', '_').replace('.', '_').upper()
print('#ifndef %s' % protected)
print('#define %s' % protected)
print('#include <utility>\n#include <memory>')
print('''
#if __cplusplus < 201703ul
template <typename T, typename U>
std::shared_ptr<T> reinterpret_pointer_cast(const std::shared_ptr<U>& r) noexcept  {
  auto p = reinterpret_cast<typename std::shared_ptr<T>::element_type *>(r.get());
  return std::shared_ptr<T>(r, p);
}
#else
using std::reinterpret_pointer_cast;
#endif
''')
pos = 0
# 搜索定义
reg = re.compile(r'interface\s*([^{]+)\{([^\}]+)\}\s*')
method_reg = re.compile(r'([^\s]+)\s+(\w+)(.*)')
while True:
    res = reg.search(interface_declare, pos)
    # 重复搜索直到结果为空
    if res is None:
        break
    pos = res.span()[1]
    interface_name = res.group(1).strip()
    methods = res.group(2)
    method_list = methods.strip().split('\n')
    declare_list = []
    inherit_list = []
    # 搜索方法
    for method in method_list:
        r = method_reg.search(method)
        if r is not None:
            declare_list.append((r.group(1), r.group(2), r.group(3)))
        else:
            inherit_list.append(method.strip())
    # 定义接口类，根据是否有继承定义决定是否继承
    if len(inherit_list) == 0:
        print('class %s {' % interface_name)
    else:
        decl = 'class %s : ' % interface_name
        for i in 0, len(inherit_list) - 1:
            decl += 'public %s' % inherit_list[i]
            if i is not len(inherit_list) - 1:
                decl += ','
        decl += '{'
        print(decl)
    print('  struct _Dummy{};')
    # 如果有方法定义，则需要增加一个模拟虚函数表、一个对象指针、一个模拟虚函数表指针
    init = ""
    if len(declare_list) != 0:
        print('  template<typename _T>')
        print('  struct _Vtb {')
        print('    static _Vtb _vtb;')
        init += '{'
        for declare in declare_list:
            print('    %s (_T::* %s)%s;' % declare)
            init += "&_T::%s, " % declare[1]
        init += '};'
        print('  };')
        print('  std::shared_ptr<_Dummy> ptr;')
        print('  _Vtb<_Dummy>* vtb;')
    # 构造函数
    print(' public:')
    construct = ""
    # 如果有继承定义，先初始化基类
    if len(inherit_list) == 0:
        construct += "  template<typename _T>\n"
    else:
        construct += "  template<typename _T, typename _SFINAE = typename std::enable_if<!std::is_same<_T, %s>::value>::type>\n" % interface_name
    construct += "  %s(const std::shared_ptr<_T>& t): " % interface_name
    if len(inherit_list) != 0:
        for i in 0, len(inherit_list) - 1:
            construct += '%s(t)' % inherit_list[i]
            if i != len(inherit_list) - 1:
                construct += ', '
    # 如果有方法定义，初始化对象指针、模拟虚函数表指针
    if len(declare_list) != 0:
        if len(inherit_list) != 0:
            construct += ', '
        construct += 'ptr(reinterpret_pointer_cast<_Dummy>(t)), vtb(reinterpret_cast<_Vtb<_Dummy>*>(&_Vtb<_T>::_vtb))'
    construct += ' {}'
    print(construct)
    # 通过模拟虚函数表转发函数调用
    for declare in declare_list:
        print('  template<typename ..._Args>')
        print('  %s %s(_Args&& ..._args) const {' % (declare[0], declare[1]))
        print('    return ((ptr.get())->*(vtb->%s))(std::forward<_Args>(_args)...);' % declare[1])
        print('  }')
    print('};')
    print()
    # 初始化模拟虚函数表
    if len(declare_list) != 0:
        print('template<typename _T>')
        print('%s::_Vtb<_T> %s::_Vtb<_T>::_vtb = %s' % (interface_name, interface_name, init))
        print()
print('#endif')
