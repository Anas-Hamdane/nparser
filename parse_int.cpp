#include <cstdint>
#include <string>
#include <stdexcept>

bool starts_with(const std::string &str, const std::string &cmp) {
  return (str.compare(0, cmp.length(), cmp) == 0);
}

uint64_t parse_integer(std::string str) {
  enum Kind { Decimal = 10, Hex = 16, Octal = 8, Binary = 2 };
  Kind kind = Decimal;

  if (starts_with(str, "0x") || starts_with(str, "0X")) {
    kind = Hex;
    str.erase(0, 2);
  }

  else if (starts_with(str, "0o") || starts_with(str, "0O")) {
    kind = Octal;
    str.erase(0, 2);
  }

  else if (starts_with(str, "0b") || starts_with(str, "0B")) {
    kind = Binary;
    str.erase(0, 2);
  }

  if (str.length() == 0)
    throw std::invalid_argument("error, invalid integer");

  uint64_t result = 0;

  // check for empty strings like "0x''"
  bool valid = false;
  for (char c : str) {
    int digit;

    if (c == '\'')
      continue;

    if (kind == Octal && (c == '8' || c == '9'))
      throw std::invalid_argument(
          "invalid character in octal integer expression: '" +
          std::string(1, c) + "'\n");

    if (kind == Binary && c != '0' && c != '1')
      throw std::invalid_argument(
          "invalid character in binary integer expression: '" +
          std::string(1, c) + "'\n");

    if (std::isdigit(c))
      digit = c - '0';

    else if (kind == Hex && std::isxdigit(c))
      digit = (c >= 'a' && c <= 'f') ? (c - 'a' + 10) : (c - 'A' + 10);

    else
      throw std::invalid_argument(
          "invalid character in integer literal expression: '" +
          std::string(1, c) + "'\n");

    if (result > (UINT64_MAX - digit) / kind)
      throw std::out_of_range("Integer literal overflow for string: '" + str +
                              "'\n");

    result = (result * kind) + digit;

    // has at least one valid letter
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid inetger literal: '" + str + "'");

  return result;
}
