#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

enum class Kind { Decimal, Hex, Octal, Binary };

bool starts_with(const std::string &str, const std::string &cmp) {
  return (str.compare(0, cmp.length(), cmp) == 0);
}

/*
 * Detect the number kind, and erases the part
 * resposible for detection.
 */
Kind detect_erase_kind(std::string &str) {
  Kind kind = Kind::Decimal;

  if (starts_with(str, "0x") || starts_with(str, "0X")) {
    kind = Kind::Hex;
    str.erase(0, 2);
  }

  else if (starts_with(str, "0o") || starts_with(str, "0O")) {
    kind = Kind::Octal;
    str.erase(0, 2);
  }

  else if (starts_with(str, "0b") || starts_with(str, "0B")) {
    kind = Kind::Binary;
    str.erase(0, 2);
  }

  return kind;
}

uint64_t parse_hex(const std::string& origin, const std::string &copy) {
  bool valid = false;
  uint64_t result = 0;

  for (unsigned char c : copy) {
    // skip separators
    if (c == '\'')
      continue;

    int digit;

    if (c >= '0' && c <= '9')
      digit = c - '0';
    else if (c >= 'a' && c <= 'f')
      digit = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      digit = c - 'A' + 10;
    else
      throw std::invalid_argument("Invalid hex digit '" + std::string(1, c) +
                                  "' in literal '" + origin + "'");

    if (result > (UINT64_MAX >> 4))
      throw std::out_of_range("hex literal overflow: '" + origin + "'");

    result = (result << 4) | digit;
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid hex literal: '" + origin + "'\n");

  return result;
}

uint64_t parse_dec(const std::string& origin, const std::string &copy) {
  const static size_t base = 10;

  bool valid = false;
  uint64_t result = 0;

  for (unsigned char c : copy) {
    // skip separators
    if (c == '\'')
      continue;

    if (!std::isdigit(c)) {
      throw std::invalid_argument("Invalid decimal digit '" +
                                  std::string(1, c) + "' in literal '" + origin +
                                  "'");
    }

    int digit = c - '0';
    if (result > (UINT64_MAX - digit) / base)
      throw std::out_of_range("decimal literal overflow: '" + origin + "'");

    result = (result * base) + digit;
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid decimal literal: '" + origin + "'\n");

  return result;
}

uint64_t parse_oct(const std::string& origin, const std::string &copy) {
  bool valid = false;
  uint64_t result = 0;

  for (unsigned char c : copy) {
    // skip separators
    if (c == '\'')
      continue;

    if (c < '0' || c > '7') {
      throw std::invalid_argument("Invalid octal digit '" + std::string(1, c) +
                                  "' in literal '" + origin + "'");
    }

    int digit = c - '0';
    if (result > (UINT64_MAX >> 3))
      throw std::out_of_range("octal literal overflow: '" + origin + "'");

    result = (result << 3) | digit;
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid octal literal: '" + origin + "'\n");

  return result;
}

uint64_t parse_bin(const std::string& origin, const std::string &copy) {
  bool valid = false;
  uint64_t result = 0;

  for (unsigned char c : copy) {
    // skip separators
    if (c == '\'')
      continue;

    if (c != '0' && c != '1') {
      throw std::invalid_argument("Invalid binary digit '" + std::string(1, c) +
                                  "' in literal '" + origin + "'");
    }

    if (result > (UINT64_MAX >> 1))
      throw std::out_of_range("binary literal overflow: '" + origin + "'");

    result = (result << 1) | (c - '0');
  }

  if (!valid)
    throw std::invalid_argument("Invalid binary literal: '" + origin + "'\n");

  return result;
}

int64_t parse_integer(const std::string& origin) {
  std::string copy = origin;
  Kind kind = detect_erase_kind(copy);
  if (copy.length() == 0)
    throw std::invalid_argument("error, invalid integer");

  // clang-format off
  switch (kind) {
    case Kind::Decimal: return parse_dec(origin, copy);
    case Kind::Hex:     return parse_hex(origin, copy);
    case Kind::Octal:   return parse_oct(origin, copy);
    case Kind::Binary:  return parse_bin(origin, copy);
  }
  // clang-format on
}

long double parse_floating_point(const std::string& origin) {
  std::string copy = origin;
  Kind kind = detect_erase_kind(copy);

  if (copy.length() == 0)
    throw std::invalid_argument("error, invalid float literal");

  if (kind == Kind::Octal || kind == Kind::Binary)
    throw std::invalid_argument("float literals must be either Hex or Decimal");

  if (kind == Kind::Hex ? !std::isxdigit(copy.back())
                        : !std::isdigit(copy.back()))
    throw std::invalid_argument("Invalid floating point end: '" +
                                std::string(1, copy.back()) + "'");

  size_t base = kind == Kind::Decimal ? 10 : 16;
  size_t exponent_base = kind == Kind::Decimal ? 10 : 2;
  // if we have '.5' it's '0.5' the integer part is 0
  uint64_t integer = 0;
  // if we have '5' it's '5.0' the fraction part is 0
  uint64_t fraction = 0;
  // if we have '1.2' it's '1.2e0' the exponent is 0
  uint64_t exponent = 0;

  char SN = kind == Kind::Decimal ? 'e' : 'p';

  size_t dotpos = copy.find('.');
  size_t snpos = copy.find(SN);
  if (snpos == std::string::npos)
    snpos = copy.find(std::toupper(SN));

  std::string integer_part;
  if (dotpos == 0)
    integer_part = "0";
  else if (dotpos != std::string::npos)
    integer_part = copy.substr(0, dotpos);
  else
    integer_part = copy.substr(0, snpos > 0 ? snpos : copy.length());

  if (kind == Kind::Decimal)
    integer = parse_dec(origin, integer_part);
  if (kind == Kind::Hex)
    integer = parse_hex(origin, integer_part);

  // doesn't have fraction or exponent
  if (dotpos == std::string::npos && snpos == std::string::npos)
    return integer;

  if (dotpos == std::string::npos)
    dotpos = 0;

  else if (snpos == std::string::npos)
    snpos = copy.length();

  std::string fracpart;
  fracpart = copy.substr(dotpos + 1, snpos - (dotpos + 1));

  if (kind == Kind::Decimal)
    fraction = parse_dec(origin, fracpart);
  if (kind == Kind::Hex)
    fraction = parse_hex(origin, fracpart);

  // doesn't have exponent
  if (snpos == copy.length())
    return integer + (fraction / pow(base, fracpart.length()));

  if (snpos == (copy.length() - 1)) {
    throw std::invalid_argument(
        "Invalid floating point literal, exponent at the end is not allowed");
  }

  bool negative = false;
  if (copy[snpos + 1] == '-' || copy[snpos + 1] == '+') {
    snpos++;
    negative = copy[snpos + 1] == '-';
  }

  exponent = parse_dec(origin, copy.substr(snpos + 1, copy.length() - (snpos + 1)));
  exponent *= (negative) ? -1 : 1;

  long double result = (integer + (fraction / pow(base, fracpart.length()))) * pow(exponent_base, exponent);

  if (!std::isfinite(result))
    throw std::out_of_range("floating point literal overflow: '" + copy + "'");

  return result;
}

bool valid_integer(std::string str) {
  enum Kind { Decimal, Hex, Octal, Binary };

  if (str.empty() || str.find(' ') != std::string::npos)
    return false;

  Kind kind = Decimal;

  if (starts_with(str, "-") || starts_with(str, "+"))
    str.erase(0, 1);

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
    return false;

  for (unsigned char c : str) {
    if (kind == Hex ? !isxdigit(c) : !isdigit(c))
      return false;

    if (kind == Octal && (c == '8' || c == '9'))
      return false;

    if (kind == Binary && c != '0' && c != '1')
      return false;
  }

  return true;
}

bool valid_float(std::string str) {
  enum Kind { Decimal, Hex };

  if (str.empty() || str.find(' ') != std::string::npos)
    return false;

  Kind kind = Decimal;

  if (starts_with(str, "-") || starts_with(str, "+"))
    str.erase(0, 1);

  if (starts_with(str, "0x") || starts_with(str, "0X")) {
    kind = Hex;
    str.erase(0, 2);
  }

  if (str.length() == 0)
    return false;

  char corr_sn = kind == Hex ? 'P' : 'E';

  if (str.back() == '.' || str.back() == corr_sn || str.back() == '-' ||
      str.back() == '+')
    return false;

  if (str.front() == corr_sn)
    return false;

  // has a '.', has a scientific notation E/P
  bool dot = false;
  bool sn = false;

  for (size_t i = 0; i < str.length(); ++i) {
    unsigned char c = (unsigned char)str[i];

    if (c == '.') {
      if (dot || sn)
        return false;

      dot = true;
      continue;
    }

    if (c == corr_sn) {
      if (sn || (dot && str[i - 1] == '.'))
        return false;

      if (i + 1 < str.length() && (str[i + 1] == '-' || str[i + 1] == '+'))
        i++;

      sn = true;
      continue;
    }

    if (kind == Hex ? !isxdigit(c) : !isdigit(c))
      return false;
  }

  return true;
}

void test_floating_point(std::string test) {
  std::cout << test << ": " << parse_floating_point(test) << '\n';
}

int main() {
  test_floating_point("1");
  test_floating_point("1.2");
  test_floating_point(".2");
  test_floating_point("0.2");
  test_floating_point("0");

  test_floating_point("1.2e2");

  test_floating_point("0x0");
  test_floating_point("0x10");
  test_floating_point("0x3f99999a");
}
