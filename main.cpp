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

uint64_t parse_hex(const std::string &str) {
  bool valid = false;
  uint64_t result = 0;

  for (unsigned char c : str) {
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
                                  "' in literal '" + str + "'");

    if (result > (UINT64_MAX >> 4))
      throw std::out_of_range("hex literal out of range");

    result = (result << 4) | digit;
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid hex literal: '" + str + "'\n");

  return result;
}

uint64_t parse_dec(const std::string &str) {
  const static size_t base = 10;

  bool valid = false;
  uint64_t result = 0;

  for (unsigned char c : str) {
    // skip separators
    if (c == '\'')
      continue;

    if (!std::isdigit(c)) {
      throw std::invalid_argument("Invalid decimal digit '" +
                                  std::string(1, c) + "' in literal '" + str +
                                  "'");
    }

    int digit = c - '0';
    if (result > (UINT64_MAX - digit) / base)
      throw std::out_of_range("decimal literal out of range");

    result = (result * base) + digit;
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid decimal literal: '" + str + "'\n");

  return result;
}

uint64_t parse_oct(const std::string &str) {
  bool valid = false;
  uint64_t result = 0;

  for (unsigned char c : str) {
    // skip separators
    if (c == '\'')
      continue;

    if (c < '0' || c > '7') {
      throw std::invalid_argument("Invalid octal digit '" + std::string(1, c) +
                                  "' in literal '" + str + "'");
    }

    int digit = c - '0';
    if (result > (UINT64_MAX >> 3))
      throw std::out_of_range("octal literal out of range");

    result = (result << 3) | digit;
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid octal literal: '" + str + "'\n");

  return result;
}

uint64_t parse_bin(const std::string &str) {
  bool valid = false;
  uint64_t result = 0;

  for (unsigned char c : str) {
    // skip separators
    if (c == '\'')
      continue;

    if (c != '0' && c != '1') {
      throw std::invalid_argument("Invalid binary digit '" + std::string(1, c) +
                                  "' in literal '" + str + "'");
    }

    if (result > (UINT64_MAX >> 1))
      throw std::out_of_range("binary literal out of range");

    result = (result << 1) | (c - '0');
  }

  if (!valid)
    throw std::invalid_argument("Invalid binary literal: '" + str + "'\n");

  return result;
}

uint64_t parse_integer(std::string str) {
  Kind kind = detect_erase_kind(str);
  if (str.length() == 0)
    throw std::invalid_argument("error, invalid integer");

  // clang-format off
  switch (kind) {
    case Kind::Decimal: return parse_dec(str);
    case Kind::Hex:     return parse_hex(str);
    case Kind::Octal:   return parse_oct(str);
    case Kind::Binary:  return parse_bin(str);
  }
  // clang-format on
}

long double parse_floating_point(std::string str) {
  Kind kind = detect_erase_kind(str);

  if (str.length() == 0)
    throw std::invalid_argument("error, invalid float literal");

  if (kind == Kind::Octal || kind == Kind::Binary)
    throw std::invalid_argument("float literals must be either Hex or Decimal");

  if (kind == Kind::Hex ? !std::isxdigit(str.back()) : !std::isdigit(str.back()))
    throw std::invalid_argument("Invalid floating point end: '" + std::string(1, str.back()) + "'");

  size_t base = kind == Kind::Decimal ? 10 : 16;
  size_t exponent_base = kind == Kind::Decimal ? 10 : 2;
  // if we have '.5' it's '0.5' the integer part is 0
  uint64_t integer = 0;
  // if we have '5' it's '5.0' the fraction part is 0
  uint64_t fraction = 0;
  // if we have '1.2' it's '1.2e0' the exponent is 0
  uint64_t exponent = 0;

  char SN = kind == Kind::Decimal ? 'e' : 'p';

  size_t dotpos = str.find('.');

  if (dotpos != std::string::npos && dotpos != 0) {
    std::string intpart = str.substr(0, dotpos);

    size_t snpos = str.find(SN);
    if (kind == Kind::Decimal)
      integer = parse_dec(intpart);
    if (kind == Kind::Hex)
      integer = parse_hex(intpart);

    std::string fracpart;
    if (snpos != std::string::npos)
      fracpart = str.substr(dotpos + 1, snpos);
    else
      fracpart = str.substr(dotpos + 1, str.length());

    if (kind == Kind::Decimal)
      fraction = parse_dec(fracpart);
    if (kind == Kind::Hex)
      fraction = parse_hex(fracpart);

    if (snpos != std::string::npos) {
      bool negative = false;
      if (snpos == (str.length() - 1))
        throw std::invalid_argument("Invalid floating point literal, exponent "
                                    "at the end is not allowed");

      if (str[snpos + 1] == '-' || str[snpos + 1] == '+') {
        snpos++;
        negative = str[snpos + 1] == '-';
      }

      exponent = parse_dec(str.substr(snpos, str.length()));
      exponent *= (negative) ? -1 : 1;
    }

    return (integer + (fraction / pow(kind == Kind::Decimal ? 10 : 16,
                                      fracpart.length()))) *
           pow(kind == Kind::Decimal ? 10 : 2, exponent);
  }

  else if (dotpos == std::string::npos)
    dotpos = 0;

  size_t snpos = str.find(SN);

  std::string fracpart;
  if (snpos != std::string::npos)
    fracpart = str.substr(dotpos + 1, snpos);
  else
    fracpart = str.substr(dotpos + 1, str.length());

  if (kind == Kind::Decimal)
    fraction = parse_dec(fracpart);
  if (kind == Kind::Hex)
    fraction = parse_hex(fracpart);

  if (snpos == std::string::npos)
    return integer + (fraction / pow(base, fracpart.length()));

  if (snpos == (str.length() - 1)) {
    throw std::invalid_argument(
        "Invalid floating point literal, exponent at the end is not allowed");
  }

  bool negative = false;
  if (str[snpos + 1] == '-' || str[snpos + 1] == '+') {
    snpos++;
    negative = str[snpos + 1] == '-';
  }

  exponent = parse_dec(str.substr(snpos, str.length()));
  exponent *= (negative) ? -1 : 1;

  return (integer + fraction / pow(base, fracpart.length()) * pow(exponent_base, exponent));
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

int main() {
  std::string test = "1.265";
  std::cout << test << ": " << parse_floating_point(test) << '\n';
  printf("%Lf\n", parse_floating_point(test));
}
