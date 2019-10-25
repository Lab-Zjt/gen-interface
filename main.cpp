#include <iostream>
#include <vector>
#include <stack>
#include "interface.h"

using std::string;
using std::vector;
using std::stack;

struct Parameter {
  string type;
  string name;
  static Parameter Parse(const std::string &str) {
    Parameter decl;
    decl.type = str;
    decl.name = str.substr(str.rfind(' ') + 1);
    return decl;
  }
};

bool is_left_bracket(char c) {
  return c == '<' || c == '{' || c == '[' || c == '(';
}

bool is_right_bracket(char c) {
  return c == '>' || c == '}' || c == ']' || c == ')';
}

bool is_bracket(char c) { return is_left_bracket(c) || is_right_bracket(c); }

bool match(char c1, char c2) {
  if (c1 == '<' && c2 == '>')return true;
  if (c1 == '{' && c2 == '}')return true;
  if (c1 == '[' && c2 == ']')return true;
  if (c1 == '(' && c2 == ')')return true;
  return false;
}

struct Declaration {
  string return_decl;
  string name;
  vector<Parameter> param_decl;
  string cvr_specifier;
  static Declaration Parse(const string &str) {
    Declaration decl;
    int off = 0;
    bool space = false;
    stack<char> bracket;
    while (isspace(str[off]))off++;
    int ret_begin = off;
    while (off < str.size()) {
      if (isspace(str[off]))space = true;
      else space = false;
      if (is_left_bracket(str[off])) bracket.push(str[off]);
      if (is_right_bracket(str[off])) {
        if (match(bracket.top(), str[off])) {
          bracket.pop();
        }
      }
      if (space && bracket.empty()) {
        break;
      }
      off++;
    }
    if (off == str.size()) {
      throw std::runtime_error("parse failed.");
    }
    decl.return_decl = str.substr(ret_begin, off);
    while (isspace(str[off]))off++;
    auto name_begin = off;
    while (isalnum(str[off]) || str[off] == '_')off++;
    decl.name = str.substr(name_begin, off - name_begin);
    while (isspace(str[off]))off++;
    if (str[off] != '(') {
      throw std::runtime_error("need ( at params begin.");
    }
    off++;
    bool comma = false;
    while (str[off] != ')') {
      int param_begin = off;
      while (off < str.size()) {
        if (str[off] == ',')comma = true;
        else comma = false;
        if (is_left_bracket(str[off])) bracket.push(str[off]);
        if (is_right_bracket(str[off])) {
          if (bracket.empty()) {
            break;
          }
          if (match(bracket.top(), str[off])) {
            bracket.pop();
          }
        }
        if (comma && bracket.empty())break;
        off++;
      }
      decl.param_decl.emplace_back(Parameter::Parse(str.substr(param_begin, off - param_begin)));
      if (str[off] == ')')break;
      else off ++;
    }
    off++;
    decl.cvr_specifier = str.substr(off);
    return decl;
  }
};

void f(void(*f1)()) {

}
using std::cout;
using std::cin;
using std::endl;

struct interface_decl {
  string name;

};

int main() {
  Declaration decl = Declaration::Parse("R<int, Error> Read(void* buf, size_t size) const &");
  cout << decl.return_decl << endl;
  cout << decl.name << endl;
  for (const auto &param: decl.param_decl) {
    cout << param.type << "|" << param.name << endl;
  }
  cout << decl.cvr_specifier << endl;
  cout << decl.return_decl << ' ' << decl.name + '(';
  for (auto&& param: decl.param_decl) {
    cout << param.type << ',';
  }
  cout << ") {\n";
  cout << "return this->" << decl.name << '(';
  for (auto && param: decl.param_decl) {
    cout << "std::forward<decltype(" << param.name << ")>(" << param.name << "),";
  }
  cout << ");\n}";
}