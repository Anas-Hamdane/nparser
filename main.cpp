#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

#include "Logger.hpp"

Logger logger;

#define FP_FRACTION_MD 18

enum class NumKind { Decimal = 10, Hex = 16, Octal = 8, Binary = 2 };

bool starts_with(const std::string &str, const std::string &cmp) {
  return (str.compare(0, cmp.length(), cmp) == 0);
}

/*
 * Detect the number kind, and erases the part
 * resposible for detection.
 */
NumKind numkind(const std::string &str) {
  if (str.empty())
    throw std::invalid_argument("Invalid argument: empty string");

  if (starts_with(str, "0x") || starts_with(str, "0X"))
    return NumKind::Hex;

  else if (starts_with(str, "0o") || starts_with(str, "0O"))
    return NumKind::Octal;

  else if (starts_with(str, "0b") || starts_with(str, "0B"))
    return NumKind::Binary;

  else if (!std::isdigit(str.front()) && str.front() != '.')
    throw std::invalid_argument("Invalid number start in: " + str);

  return NumKind::Decimal;
}

uint64_t parse_hex(size_t start, const std::string &str, size_t end) {
  if (end > str.length())
    throw std::invalid_argument(
        "end argument is bigger than the string length");

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
      throw std::invalid_argument("Invalid hex digit '" + std::string(1, c) +
                                  "' in literal: " + str);

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
    throw std::invalid_argument(
        "end argument is bigger than the string length");

  const static size_t base = 10;

  bool valid = false;
  uint64_t result = 0;

  for (size_t i = start; i < end; ++i) {
    unsigned char c = str[i];
    // skip separators
    if (c == '\'')
      continue;

    if (!std::isdigit(c))
      throw std::invalid_argument("Invalid decimal digit '" +
                                  std::string(1, c) + "' in literal: " + str);

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
    throw std::invalid_argument(
        "end argument is bigger than the string length");

  bool valid = false;
  uint64_t result = 0;

  for (size_t i = start; i < end; ++i) {
    unsigned char c = str[i];
    // skip separators
    if (c == '\'')
      continue;

    if (c < '0' || c > '7')
      throw std::invalid_argument("Invalid octal digit '" + std::string(1, c) +
                                  "' in literal: " + str);

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
    throw std::invalid_argument(
        "end argument is bigger than the string length");

  bool valid = false;
  uint64_t result = 0;

  for (size_t i = start; i < end; ++i) {
    unsigned char c = str[i];
    // skip separators
    if (c == '\'')
      continue;

    if (c != '0' && c != '1')
      throw std::invalid_argument("Invalid binary digit '" + std::string(1, c) +
                                  "' in literal: " + str);

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

  NumKind kind = numkind(str);
  size_t start = 0;

  if (kind != NumKind::Decimal)
    start += 2;

  if ((str.length() - start) == 0)
    throw std::invalid_argument("error, invalid integer");

  // clang-format off
  switch (kind) {
    case NumKind::Decimal: return parse_dec(start, str, str.length());
    case NumKind::Hex:     return parse_hex(start, str, str.length());
    case NumKind::Octal:   return parse_oct(start, str, str.length());
    case NumKind::Binary:  return parse_bin(start, str, str.length());
  }
  // clang-format on
}

long double parse_floating_point(const std::string &str) {
  if (str.empty())
    throw std::invalid_argument("Invalid floating point literal: empty string");

  NumKind kind = numkind(str);
  size_t current_character = (kind != NumKind::Decimal) ? 2 : 0;

  if ((str.length() - current_character) == 0)
    throw std::invalid_argument("invalid floating point literal: " + str);

  else if (kind == NumKind::Octal || kind == NumKind::Binary)
    throw std::invalid_argument(
        "float literals must be either Hex or Decimal: " + str);

  else if (kind == NumKind::Hex ? !std::isxdigit(str.back())
                                : !std::isdigit(str.back()))
    throw std::invalid_argument("Invalid floating point end: " + str);

  // scientific notation
  char scientific_notation = (kind == NumKind::Decimal) ? 'e' : 'p';

  // search for '.'
  size_t has_dot = str.find('.', current_character);

  // search for the scientific notation
  size_t has_sn = str.find(scientific_notation, current_character);
  if (has_sn == std::string::npos)
    has_sn = str.find(toupper(scientific_notation), current_character);

  // if there's more than one
  if (has_dot != std::string::npos &&
      str.find('.', has_dot + 1) != std::string::npos)
    throw std::invalid_argument("Too many '.' in floating point literal: " +
                                str);
  if (has_sn != std::string::npos &&
      (str.find(scientific_notation, has_sn + 1) != std::string::npos ||
       str.find(toupper(scientific_notation), has_sn + 1) != std::string::npos))
    throw std::invalid_argument(
        "Too many scientific notations in floating point literal: " + str);

  if (has_dot != std::string::npos && has_sn != std::string::npos) {
    if (has_dot + 1 == has_sn)
      throw std::invalid_argument("Scientific notation can't come after a '.'");

    if (has_sn < has_dot)
      throw std::invalid_argument(
          "Scientific notation can't be before the '.'");
  }

  uint64_t integer = 0;
  if (has_dot != 0) {
    size_t integer_end = std::min(std::min(has_dot, has_sn), str.length());

    if (kind == NumKind::Decimal)
      integer = parse_dec(current_character, str, integer_end);
    else if (kind == NumKind::Hex)
      integer = parse_hex(current_character, str, integer_end);

    current_character = integer_end;
  }
  current_character++;

  if (has_dot == std::string::npos && has_sn == std::string::npos)
    return integer;

  uint64_t fraction = 0;
  size_t fraction_size = 0;
  size_t base = (kind == NumKind::Decimal) ? 10 : 16;
  if (has_dot != std::string::npos) {
    size_t fraction_end = std::min(has_sn, str.length());

    size_t end = fraction_end;
    fraction_size = end - current_character;

    if (fraction_size > FP_FRACTION_MD) {
      fraction_size = FP_FRACTION_MD;
      end = fraction_size + current_character;
    }

    if (kind == NumKind::Decimal)
      fraction = parse_dec(current_character, str, end);
    else if (kind == NumKind::Hex)
      fraction = parse_hex(current_character, str, end);

    current_character = fraction_end + 1;
  }

  // doesn't have exponent
  if (current_character >= str.length())
    return (integer + (fraction / pow(base, fraction_size)));

  int64_t exponent = 0;
  size_t exponent_base = (kind == NumKind::Decimal) ? 10 : 2;
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

long double parse_float(const std::string &str) {
  if (str.empty())
    logger.log(Logger::Level::ERROR,
               "Invalid floating point literal: " + str + "\n");

  NumKind kind = numkind(str);
  size_t current_character = (kind != NumKind::Decimal) ? 2 : 0;

  if ((str.length() - current_character) == 0)
    logger.log(Logger::Level::ERROR,
               "invalid floating point literal: " + str + "\n");

  else if (kind == NumKind::Octal || kind == NumKind::Binary)
    logger.log(Logger::Level::ERROR,
               "float literals must be either Hex or Decimal: " + str + "\n");

  else if (kind == NumKind::Hex ? !std::isxdigit(str.back())
                                : !std::isdigit(str.back()))
    logger.log(Logger::Level::ERROR,
               "Invalid floating point end: " + str + "\n");

  enum class Section { Integer, Fraction, Exponent };
  Section section = Section::Integer;

  // scientific notation
  char scientific_notation = (kind == NumKind::Decimal) ? 'e' : 'p';

  // search for '.'
  size_t has_dot = str.find('.', current_character);

  // search for the scientific notation
  size_t has_sn = str.find(scientific_notation, current_character);
  if (has_sn == std::string::npos)
    has_sn = str.find(toupper(scientific_notation), current_character);

  // if there's more than one
  if (has_dot != std::string::npos &&
      str.find('.', has_dot + 1) != std::string::npos)
    logger.log(Logger::Level::ERROR,
               "Too many '.' in floating point literal: " + str + "\n");

  if (has_sn != std::string::npos &&
      (str.find(scientific_notation, has_sn + 1) != std::string::npos ||
       str.find(toupper(scientific_notation), has_sn + 1) != std::string::npos))
    logger.log(Logger::Level::ERROR,
               "Too many scientific notations in floating point literal: " +
                   str + "\n");

  if (has_dot != std::string::npos && has_sn != std::string::npos) {
    if (has_dot + 1 == has_sn)
      logger.log(Logger::Level::ERROR,
                 "Scientific notation can't come after a '.'\n");

    if (has_sn < has_dot)
      logger.log(Logger::Level::ERROR,
                 "Scientific notation can't be before the '.'\n");
  }

  uint64_t integer = 0;
  if (has_dot != 0) {
    size_t integer_end = std::min(std::min(has_dot, has_sn), str.length());

    if (kind == NumKind::Decimal)
      integer = parse_dec(current_character, str, integer_end);
    else if (kind == NumKind::Hex)
      integer = parse_hex(current_character, str, integer_end);

    current_character = integer_end;
  }
  current_character++;

  if (has_dot == std::string::npos && has_sn == std::string::npos)
    return integer;

  uint64_t fraction = 0;
  size_t fraction_size = 0;
  size_t base = (kind == NumKind::Decimal) ? 10 : 16;
  if (has_dot != std::string::npos) {
    size_t fraction_end = std::min(has_sn, str.length());

    size_t end = fraction_end;
    fraction_size = end - current_character;

    if (fraction_size > FP_FRACTION_MD) {
      fraction_size = FP_FRACTION_MD;
      end = fraction_size + current_character;
    }

    if (kind == NumKind::Decimal)
      fraction = parse_dec(current_character, str, end);
    else if (kind == NumKind::Hex)
      fraction = parse_hex(current_character, str, end);

    current_character = fraction_end + 1;
  }

  // doesn't have exponent
  if (current_character >= str.length())
    return (integer + (fraction / pow(base, fraction_size)));

  int64_t exponent = 0;
  size_t exponent_base = (kind == NumKind::Decimal) ? 10 : 2;
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
    logger.log(Logger::Level::ERROR,
               "floating point literal overflow: " + str + "\n");

  return result;
}

long double new_parse_float(const std::string &str) {
  enum class Section { Integer, Fraction, Exponent };

  long double integer = 0;
  long double fraction = 0;
  long double exponent = 0;

  size_t fraction_size = 0;

  NumKind kind = numkind(str);
  char scientific_notation = (kind == NumKind::Hex) ? 'p' : 'e';

  if (kind != NumKind::Decimal && kind != NumKind::Hex)
    throw std::invalid_argument("Float literals must be either Hex or Decimal");

  size_t current_character = (kind == NumKind::Hex) ? 2 : 0;

  Section section = Section::Integer;
  bool negative = false;
  uint64_t tmp = 0;
  for (size_t i = current_character; i < str.length(); ++i) {
    unsigned char c = str[i];

    if (c == '\'') {
      if (str[i - 1] == '\'')
        throw std::invalid_argument("Only one separator alowed at a time");

      continue;
    }

    if (c == '.') {
      if (section != Section::Integer)
        throw std::invalid_argument("Invalid Integer section in float literal");

      integer = tmp;
      tmp = 0;
      section = Section::Fraction;
      continue;
    }

    if (c == scientific_notation || c == toupper(scientific_notation)) {
      if (section == Section::Exponent)
        throw std::invalid_argument(
            "Invalid Exponent Sections in float literal");

      if (section == Section::Fraction)
        fraction = tmp;
      else
        integer = tmp;

      if (i + 1 >= str.length())
        throw std::invalid_argument("Float literals can't end with a scientific notation");

      if (str[i + 1] == '-' || str[i + 1] == '+') {
        negative = (str[i + 1] == '-');
        i++;
      }

      tmp = 0;
      section = Section::Exponent;
      continue;
    }

    if (section == Section::Exponent && !std::isdigit(c))
      throw std::invalid_argument("Exponent must be a valid decimal");

    if (kind == NumKind::Hex && !std::isxdigit(c))
      throw std::invalid_argument("Invalid digit in hex literal");

    if (kind == NumKind::Decimal && !std::isdigit(c))
      throw std::invalid_argument("Invalid digit in decimal literal");

    size_t digit;
    if (c >= '0' && c <= '9')
      digit = c - '0';
    else if (c >= 'a' && c <= 'f')
      digit = c - 'a' + 10;
    else
      digit = c - 'A' + 10;

    if (tmp > (UINT64_MAX - digit) / (uint64_t)kind)
      throw std::out_of_range("float literal overflow: '" + str);

    if (fraction_size >= FP_FRACTION_MD)
      continue;

    if (section == Section::Fraction)
      fraction_size++;

    tmp = (tmp * (uint64_t)kind) + digit;
  }

  switch (section) {
    case Section::Integer: integer = tmp; break;
    case Section::Fraction: fraction = tmp; break;
    case Section::Exponent: exponent = tmp; break;
  }

  if (negative) exponent *= -1;

  size_t exponent_base = (kind == NumKind::Hex) ? 2 : 10;
  long double result =
      (integer + (fraction / pow((uint64_t)kind, fraction_size))) // mantissa
      * pow(exponent_base, exponent);                             // exponent

  if (!std::isfinite(result))
    throw std::invalid_argument("float literal overflow");

  return result;
}

int main() { std::cout << "123: " << new_parse_float("123") << '\n'; }
