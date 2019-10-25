#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <memory>
#include <fstream>
#include <algorithm>
#define LOG std::cout << __FILE__ << ':' << __LINE__ << '[' << __FUNCTION__ << "] "
#define ERR std::cerr << __FILE__ << ':' << __LINE__ << '[' << __FUNCTION__ << "] "
using std::string;
using std::vector;
using std::stack;
using std::unique_ptr;
using std::shared_ptr;
using std::cin;
using std::cout;
using std::endl;
using std::move;
using std::ifstream;

template<typename T>
using Ptr = unique_ptr<T>;

template<typename T>
using Ref = shared_ptr<T>;

// 各类型关系如下
// Interface -> name + extends[] + Method[]
// Method -> Type + name + Param[] + spec
// Param -> Type + name
// Type -> decl
struct InterfaceDecl;
struct MethodDecl;
struct TypeDecl;
struct ParamDecl;

struct InterfaceDecl {
  string name;
  vector<Ptr<MethodDecl>> method_vec;
  vector<string> extends_vec;
};

struct MethodDecl {
  Ptr<TypeDecl> ret;
  string name;
  vector<Ptr<ParamDecl>> param_vec;
  string spec;
};

struct TypeDecl {
  string decl;
};

struct ParamDecl {
  Ptr<TypeDecl> type;
  string name;
};

// 消耗所有空白符
bool ParseSpace(const string &str, int &off) {
  while (isspace(str[off]) && off < str.size())off++;
  return true;
}

// 匹配str是否以pattern开头，并移动off
bool ParseString(const string &str, const string &pattern, int &off) {
  ParseSpace(str, off);
  if (str.substr(off, pattern.size()) == pattern) {
    off += pattern.size();
    return true;
  }
  return false;
}

// 匹配str是否以pattern开头，不移动off
bool HasPrefix(const string &str, const string &pattern, int &off) {
  ParseSpace(str, off);
  return str.substr(off, pattern.size()) == pattern;
}

// 判断c是否[0-9a-zA-Z_]
bool IsIdentifier(char c) {
  return isalnum(c) || c == '_';
}

// 判断c是否左括号
bool IsLeftBracket(char c) {
  switch (c) {
    case '{':
    case '[':
    case '<':
    case '(':return true;
    default:return false;
  }
}
// 判断c是否右括号
bool IsRightBracket(char c) {
  switch (c) {
    case '}':
    case ']':
    case '>':
    case ')':return true;
    default:return false;
  }
}

// 判断两个括号是否匹配
bool IsBracketMatch(char c1, char c2) {
  if (c1 == '<' && c2 == '>')return true;
  if (c1 == '{' && c2 == '}')return true;
  if (c1 == '[' && c2 == ']')return true;
  return c1 == '(' && c2 == ')';
}

// 解析cvref限定符，并移动off
bool ParseSpec(const string &str, string &spec, int &off) {
  int spec_begin = off;
  while (str[off] != ';')off++;
  int spec_end = off;
  if (spec_begin == spec_end) {
    return false;
  }
  spec = str.substr(spec_begin, spec_end - spec_begin);
  return true;
}

// 解析一个标识符到id中，并移动off
bool ParseIdentifier(const string &str, string &id, int &off) {
  ParseSpace(str, off);
  int begin = off;
  while (IsIdentifier(str[off]))off++;
  int end = off;
  if (begin == end)return false;
  id = str.substr(begin, end - begin);
  return true;
}

// 解析一个类型，并移动off
Ptr<TypeDecl> ParseType(const string &str, int &off) {
  ParseSpace(str, off);
  stack<char> bracket;
  int ret_begin = off;
  bool space = false;
  while (off < str.size()) {
    auto c = str[off];
    space = isspace(c);
    // 匹配扩号，如果括号不匹配意味着解析仍未完成
    if (IsLeftBracket(c))bracket.push(c);
    if (IsRightBracket(c)) {
      if (IsBracketMatch(bracket.top(), c)) {
        bracket.pop();
      }
    }
    off++;
    // 如果解析到空格，而且括号匹配
    if (space && bracket.empty()) {
      // 如果下一个标识符不是cv、引用、指针，或者当前标识符不是cv，则尝试继续解析
      if (HasPrefix(str, "const", off) ||
          HasPrefix(str, "volatile", off) ||
          HasPrefix(str, "&", off) ||
          HasPrefix(str, "*", off) ||
          str.substr(ret_begin, off - 1 - ret_begin) == "const" ||
          str.substr(ret_begin, off - 1 - ret_begin) == "volatile") {
        continue;
      }
      // 解析到名称的前一个字符
      while (!IsIdentifier(str[off])) off++;
      break;
    }
  }
  int ret_end = off;
  if (ret_begin == ret_end) return nullptr;
  Ptr<TypeDecl> decl(new TypeDecl);
  decl->decl = str.substr(ret_begin, ret_end - ret_begin);
  return decl;
}

// 解析一个参数，并移动off
Ptr<ParamDecl> ParseParam(const string &str, int &off) {
  Ptr<ParamDecl> decl(new ParamDecl);
  // 解析类型
  decl->type = ParseType(str, off);
  if (decl->type == nullptr) {
    ERR << "parse param type error\n";
    return nullptr;
  }
  // 解析名称
  if (!ParseIdentifier(str, decl->name, off)) {
    ERR << "parse param name error\n";
    return nullptr;
  }
  return decl;
}

// 解析一个方法声明，并移动off
Ptr<MethodDecl> ParseMethod(const string &str, int &off) {
  Ptr<MethodDecl> decl(new MethodDecl);
  // 解析返回类型
  decl->ret = ParseType(str, off);
  if (decl->ret == nullptr) {
    ERR << "parse return type error\n";
    return nullptr;
  }
  // 解析名称
  if (!ParseIdentifier(str, decl->name, off)) {
    ERR << "parse method name error\n";
    return nullptr;
  }
  // 解析左小括号
  if (!ParseString(str, "(", off)) {
    ERR << "parse ( error\n";
    return nullptr;
  }
  // 如果解析到右小括号，说明没有参数列表
  if (ParseString(str, ")", off)) {
    ParseSpec(str, decl->spec, off);
    if (!ParseString(str, ";", off)) {
      ERR << "parse ; error\n";
    }
    return decl;
  }
  // 循环解析参数声明
  while (true) {
    auto param = ParseParam(str, off);
    if (param == nullptr) {
      break;
    }
    decl->param_vec.emplace_back(move(param));
    if (!ParseString(str, ",", off)) {
      if (!ParseString(str, ")", off)) {
        ERR << "parse ) error\n";
        break;
      }
      ParseSpec(str, decl->spec, off);
      if (!ParseString(str, ";", off)) {
        ERR << "parse ; error\n";
        break;
      }
      break;
    }
  }
  return decl;
}


// 解析一个继承声明，并移动off
vector<string> ParseExtends(const string &str, int &off) {
  vector<string> extends;
  // 循环解析标识符（基类接口的名称）
  while (true) {
    string id;
    if (ParseIdentifier(str, id, off)) {
      extends.emplace_back(move(id));
    } else {
      ERR << "parse extends name error\n";
      return {};
    }
    // 如果没有解析到','，说明解析完成
    if (!ParseString(str, ",", off)) {
      break;
    }
  }
  return extends;
}

// 解析一个interface声明，并移动off
Ptr<InterfaceDecl> ParseInterface(const string &str, int &off) {
  Ptr<InterfaceDecl> decl(new InterfaceDecl);
  ParseSpace(str, off);
  // 到达文件尾
  if (str.size() == off) return nullptr;
  // 解析"interface"
  if (!ParseString(str, "interface", off)) {
    ERR << "parse interface error\n";
    return nullptr;
  }
  // 解析interface名字
  if (!ParseIdentifier(str, decl->name, off)) {
    ERR << "parse interface name error\n";
    return nullptr;
  }
  // 判断是否有继承声明
  if (ParseString(str, "extends", off)) {
    // 有则解析继承声明
    decl->extends_vec = ParseExtends(str, off);
    if (decl->extends_vec.empty()) {
      ERR << "use extends but doesn't extends any interface\n";
      return nullptr;
    }
  }
  // 解析左大括号
  if (!ParseString(str, "{", off)) {
    ERR << "parse { error\n";
    return nullptr;
  }
  // 解析右大括号，如果成功则说明没有方法声明
  if (ParseString(str, "}", off)) {
    return decl;
  }
  // 循环解析方法声明
  while (true) {
    auto method = ParseMethod(str, off);
    if (method == nullptr) {
      break;
    }
    decl->method_vec.emplace_back(move(method));
    // 如果解析到右大括号，说明解析完成
    if (ParseString(str, "}", off)) {
      break;
    }
  }
  return decl;
}

// 解析整个文件
vector<Ptr<InterfaceDecl>> ParseFile(const string &str, int &off) {
  vector<Ptr<InterfaceDecl>> decl_vec;
  while (true) {
    // 不断解析interface直到失败
    auto interface = ParseInterface(str, off);
    if (interface == nullptr) {
      break;
    }
    decl_vec.emplace_back(move(interface));
  }
  return decl_vec;
}

// 对可迭代容器的所有元素调用f1，在两个元素之间调用f2
template<typename Iterable, typename F1, typename F2>
void IterableSplit(const Iterable &iterable, F1 &&f1, F2 &&f2) {
  for (auto it = iterable.begin(); it != iterable.end();) {
    f1(*it);
    it++;
    if (it == iterable.end())break;
    f2();
  }
}

int main(int argc, char *argv[]) {
//  if (argc < 2) {
//    ERR << "input file unspecified!\n";
//    return -1;
//  }
  string filename = argv[1];
//  string filename = "../interface.i";
  string filename_upper;
  std::transform(filename.begin(), filename.end(), std::back_inserter(filename_upper), [](char c) -> char {
    if (islower(c)) return c + 'A' - 'a';
    if (c == '.' || c == '/')return '_';
    return c;
  });
  ifstream ifs(filename);
  string str(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>{});
  int off = 0;
  auto vec = ParseFile(str, off);
  auto &ofs = cout;
  // 共有部分 interface基础代码
  ofs << "#ifndef _INTERFACE_BASIC_\n"
         "#define _INTERFACE_BASIC_\n"
         "#include <memory>\n"
         "#include <tuple>\n"
         "#include <cstring>\n"
         "#define ERRNO Error{strerror(errno)}\n"
         "template<typename T, typename ...Args>\n"
         "inline std::shared_ptr<T> New(Args &&...args) {\n"
         "  return std::make_shared<T>(std::forward<Args>(args)...);\n"
         "}\n"
         "template<typename T>\n"
         "using Ref = std::shared_ptr<T>;\n"
         "template<typename T>\n"
         "using Ptr = std::unique_ptr<T>;\n"
         "template<typename T>\n"
         "using WeakRef = std::weak_ptr<T>;\n"
         "template<typename ...Ts>\n"
         "using R = std::tuple<Ts...>;\n"
         "struct Error {\n"
         "  const char *desc = nullptr;\n"
         "};\n"
         "bool operator==(const Error &lhs, const Error &rhs) { return lhs.desc == rhs.desc; }\n"
         "bool operator!=(const Error &lhs, const Error &rhs) { return lhs.desc != rhs.desc; }\n"
         "Error NoError;\n"
         "#endif\n";
  ofs << "#ifndef " << filename_upper << "\n"
      << "#define " << filename_upper << "\n"
      << "#include <utility>\n"
      << "#if __cplusplus < 201703ul\n"
         "template <typename T, typename U>\n"
         "std::shared_ptr<T> reinterpret_pointer_cast(const std::shared_ptr<U>& r) noexcept  {\n"
         "  auto p = reinterpret_cast<typename std::shared_ptr<T>::element_type *>(r.get());\n"
         "  return std::shared_ptr<T>(r, p);\n"
         "}\n"
         "#else\n"
         "using std::reinterpret_pointer_cast;\n"
         "#endif\n";
  // 遍历所有interface
  for (auto &interface : vec) {
    ofs << "class " << interface->name;
    // 如果该interface是对其它接口extends而来的，需要加上继承声明
    if (!interface->extends_vec.empty()) {
      ofs << ":";
      IterableSplit(interface->extends_vec,
                    [&ofs](const string &s) { ofs << "public " << s; },
                    [&ofs]() { ofs << ", "; });
    }
    ofs << "{\n";
    // 如果interface的方法列表不为空，需要增加模拟虚函数表，对象指针
    if (!interface->method_vec.empty()) {
      ofs << "  struct _Dummy{};\n"
          << "  template<typename _T>\n"
          << "  struct _Vtb {\n"
          << "    static _Vtb _vtb;\n";
      for (auto &method : interface->method_vec) {
        ofs << "    " << method->ret->decl << " (_T::* " << method->name << ")(";
        IterableSplit(method->param_vec,
                      [&ofs](const Ptr<ParamDecl> &p) { ofs << p->type->decl << " " << p->name; },
                      [&ofs]() { ofs << ", "; });
        ofs << ");\n";
      }
      ofs << "  };\n";
      ofs << "  Ref<_Dummy> ptr;\n"
          << "  _Vtb<_Dummy>* vtb;\n";
    }
    // 构造函数声明
    ofs << " public:\n"
        << "  template<typename _T>\n"
        << "  " << interface->name << "(Ref<_T> t): ";
    // 如果有继承声明，需要初始化基类
    if (!interface->extends_vec.empty()) {
      IterableSplit(interface->extends_vec,
                    [&ofs](const string &s) { ofs << s << "(t)"; },
                    [&ofs]() { ofs << ","; });
    }
    if (!interface->extends_vec.empty() && !interface->method_vec.empty()) {
      ofs << ", ";
    }
    // 如果有方法声明，需要初始化成员
    if (!interface->method_vec.empty()) {
      ofs << "ptr(reinterpret_pointer_cast<_Dummy>(std::move(t))), "
          << "vtb(reinterpret_cast<_Vtb<_Dummy> *>(&_Vtb<_T>::_vtb))";
    }
    ofs << " {}\n";
    // 对于该interface内的所有方法
    for (auto &method : interface->method_vec) {
      // 将参数转发给虚函数表中的函数
      ofs << "  " << method->ret->decl << " " << method->name << "(";
      IterableSplit(method->param_vec,
                    [&ofs](const Ptr<ParamDecl> &p) { ofs << p->type->decl << " " << p->name; },
                    [&ofs]() { ofs << ", "; });
      ofs << ") {\n";
      ofs << "    return ((ptr.get())->*(vtb->" << method->name << "))(";
      IterableSplit(method->param_vec,
                    [&ofs](const Ptr<ParamDecl> &p) { ofs << p->name; },
                    [&ofs]() { ofs << ", "; });
      ofs << ");\n";
      ofs << "  }\n";
    }
    ofs << "};\n";
    // 初始化虚函数表
    if (!interface->method_vec.empty()) {
      ofs << "template<typename _T>\n";
      ofs << interface->name << "::_Vtb<_T> " << interface->name << "::_Vtb<_T>::_vtb = {";
      IterableSplit(interface->method_vec,
                    [&ofs](const Ptr<MethodDecl> &p) { ofs << "&_T::" << p->name; },
                    [&ofs]() { ofs << ", "; });
      ofs << "};\n";
    }
  }
  ofs << "#endif\n";
}
