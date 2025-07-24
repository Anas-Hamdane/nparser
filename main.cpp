#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

#define MAX_FRACTION_DIGITS 18

enum class Kind { Decimal, Hex, Octal, Binary };

bool starts_with(const std::string &str, const std::string &cmp) {
  return (str.compare(0, cmp.length(), cmp) == 0);
}

/*
 * Detect the number kind, and erases the part
 * resposible for detection.
 */
Kind numkind(const std::string &str) {
  if (str.empty())
    throw std::invalid_argument("Invalid argument: empty string");

  if (starts_with(str, "0x") || starts_with(str, "0X"))
    return Kind::Hex;

  else if (starts_with(str, "0o") || starts_with(str, "0O"))
    return Kind::Octal;

  else if (starts_with(str, "0b") || starts_with(str, "0B"))
    return Kind::Binary;

  else if (!std::isdigit(str.front()) && str.front() != '.')
    throw std::invalid_argument("Invalid number start in: " + str);

  return Kind::Decimal;
}

uint64_t parse_hex(size_t start, const std::string &str, size_t end) {
  if (end > str.length())
    throw std::invalid_argument("end argument is bigger than the string length");

  bool valid = false;
  uint64_t result = 0;

  for (size_t i = start; i < end; ++i) {
    unsigned char c = str[i];

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
      throw std::invalid_argument("Invalid hex digit '" + std::string(1, c) + "' in literal: " + str);

    if (result > (UINT64_MAX >> 4))
      throw std::out_of_range("hex literal overflow: " + str);

    result = (result << 4) | digit;
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid hex literal: " + str);

  return result;
}

uint64_t parse_dec(size_t start, const std::string &str, size_t end) {
  if (end > str.length())
    throw std::invalid_argument("end argument is bigger than the string length");

  const static size_t base = 10;

  bool valid = false;
  uint64_t result = 0;

  for (size_t i = start; i < end; ++i) {
    unsigned char c = str[i];
    // skip separators
    if (c == '\'')
      continue;

    if (!std::isdigit(c))
      throw std::invalid_argument("Invalid decimal digit '" + std::string(1, c) + "' in literal: " + str);

    int digit = c - '0';
    if (result > (UINT64_MAX - digit) / base)
      throw std::out_of_range("decimal literal overflow: '" + str);

    result = (result * base) + digit;
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid decimal literal: '" + str);

  return result;
}

uint64_t parse_oct(size_t start, const std::string &str, size_t end) {
  if (end > str.length())
    throw std::invalid_argument("end argument is bigger than the string length");

  bool valid = false;
  uint64_t result = 0;

  for (size_t i = start; i < end; ++i) {
    unsigned char c = str[i];
    // skip separators
    if (c == '\'')
      continue;

    if (c < '0' || c > '7')
      throw std::invalid_argument("Invalid octal digit '" + std::string(1, c) + "' in literal: " + str);

    int digit = c - '0';
    if (result > (UINT64_MAX >> 3))
      throw std::out_of_range("octal literal overflow: '" + str);

    result = (result << 3) | digit;
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid octal literal: '" + str);

  return result;
}

uint64_t parse_bin(size_t start, const std::string &str, size_t end) {
  if (end > str.length())
    throw std::invalid_argument("end argument is bigger than the string length");

  bool valid = false;
  uint64_t result = 0;

  for (size_t i = start; i < end; ++i) {
    unsigned char c = str[i];
    // skip separators
    if (c == '\'')
      continue;

    if (c != '0' && c != '1')
      throw std::invalid_argument("Invalid binary digit '" + std::string(1, c) + "' in literal: " + str);

    if (result > (UINT64_MAX >> 1))
      throw std::out_of_range("binary literal overflow: " + str);

    result = (result << 1) | (c - '0');
    valid = true;
  }

  if (!valid)
    throw std::invalid_argument("Invalid binary literal: '" + str);

  return result;
}

int64_t parse_integer(const std::string &str) {
  if (str.empty())
    throw std::invalid_argument("Invalid argument: empty string");

  Kind kind = numkind(str);
  size_t start = 0;

  if (kind != Kind::Decimal)
    start += 2;

  if ((str.length() - start) == 0)
    throw std::invalid_argument("error, invalid integer");

  // clang-format off
  switch (kind) {
    case Kind::Decimal: return parse_dec(start, str, str.length());
    case Kind::Hex:     return parse_hex(start, str, str.length());
    case Kind::Octal:   return parse_oct(start, str, str.length());
    case Kind::Binary:  return parse_bin(start, str, str.length());
  }
  // clang-format on
}

long double parse_floating_point(const std::string &str) {
  if (str.empty())
    throw std::invalid_argument("Invalid floating point literal: empty string");

  Kind kind = numkind(str);
  size_t current_character = (kind != Kind::Decimal) ? 2 : 0;

  if ((str.length() - current_character) == 0)
    throw std::invalid_argument("invalid floating point literal: " + str);

  else if (kind == Kind::Octal || kind == Kind::Binary)
    throw std::invalid_argument("float literals must be either Hex or Decimal: " + str);

  else if (kind == Kind::Hex ? !std::isxdigit(str.back())
                             : !std::isdigit(str.back()))
    throw std::invalid_argument("Invalid floating point end: " + str);

  // scientific notation
  char scientific_notation = (kind == Kind::Decimal) ? 'e' : 'p';

  // search for '.'
  size_t has_dot = str.find('.', current_character);

  // search for the scientific notation
  size_t has_sn = str.find(scientific_notation, current_character);
  if (has_sn == std::string::npos) has_sn = str.find(toupper(scientific_notation), current_character);

  // if there's more than one
  if (has_dot != std::string::npos && str.find('.', has_dot + 1) != std::string::npos)
    throw std::invalid_argument("Too many '.' in floating point literal: " + str);
  if (has_sn != std::string::npos && (str.find(scientific_notation, has_sn + 1) != std::string::npos
                                   || str.find(toupper(scientific_notation), has_sn + 1) != std::string::npos))
    throw std::invalid_argument("Too many scientific notations in floating point literal: " + str);

  if (has_dot != std::string::npos && has_sn != std::string::npos) {
    if (has_dot + 1 == has_sn)
      throw std::invalid_argument("Scientific notation can't come after a '.'");

    if (has_sn < has_dot)
      throw std::invalid_argument("Scientific notation can't be before the '.'");
  }

  uint64_t integer = 0;
  if (has_dot != 0) {
    size_t integer_end = std::min(std::min(has_dot, has_sn), str.length());

    if (kind == Kind::Decimal)
      integer = parse_dec(current_character, str, integer_end);
    else if (kind == Kind::Hex)
      integer = parse_hex(current_character, str, integer_end);

    current_character = integer_end;
  }
  current_character++;

  if (has_dot == std::string::npos && has_sn == std::string::npos)
    return integer;

  uint64_t fraction = 0;
  size_t fraction_size = 0;
  size_t base = (kind == Kind::Decimal) ? 10 : 16;
  if (has_dot != std::string::npos) {
    size_t fraction_end = std::min(has_sn, str.length());

    size_t end = fraction_end;
    fraction_size = end - current_character;

    if (fraction_size > MAX_FRACTION_DIGITS) {
      fraction_size = MAX_FRACTION_DIGITS;
      end = fraction_size + current_character;
    }

    if (kind == Kind::Decimal)
      fraction = parse_dec(current_character, str, end);
    else if (kind == Kind::Hex)
      fraction = parse_hex(current_character, str, end);

    current_character = fraction_end + 1;
  }

  // doesn't have exponent
  if (current_character >= str.length())
    return (integer + (fraction / pow(base, fraction_size)));

  int64_t exponent = 0;
  size_t exponent_base = (kind == Kind::Decimal) ? 10 : 2;
  size_t len = str.length();

  bool negative = false;
  if (str[current_character] == '-' || str[current_character] == '+') {
    negative = (str[current_character] == '-');
    current_character++;
  }

  exponent = parse_dec(current_character, str, len);
  exponent *= (negative) ? -1 : 1;

  long double result =
      (integer + (fraction / pow(base, fraction_size))) // mantissa
      * pow(exponent_base, exponent);                   // exponent

  if (!std::isfinite(result))
    throw std::out_of_range("floating point literal overflow: " + str);

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

int main() { std::cout << parse_floating_point("0x") << '\n'; }
