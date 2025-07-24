#include <string>

bool starts_with(const std::string &str, const std::string &cmp) {
  return (str.compare(0, cmp.length(), cmp) == 0);
}

// <integer>[.<fraction>][e/E[sign]<exponent>]
bool validate_dec(const std::string &str) {
  enum class Section { Integer, Fraction, Exponent };
  size_t section_size = 0;

  if (!std::isdigit(str[0]))
    return false;
  else
    section_size++;

  Section section = Section::Integer;
  for (size_t i = 1; i < str.length(); i++) {
    unsigned char c = str[i];

    if (c == '\'') {
      if (str[i - 1] == '\'')
        return false;

      continue;
    }

    if (std::isdigit(c)) {
      section_size++;
      continue;
    }

    if (c == '.') {
      if (section != Section::Integer || section_size == 0)
        return false;

      section = Section::Fraction;
      section_size = 0;
      continue;
    }

    if (c == 'e' || c == 'E') {
      if (section == Section::Exponent || section_size == 0)
        return false;

      if (i + 1 >= str.length())
        return false;

      if (str[i + 1] == '+' || str[i + 1] == '-')
        i++;

      section = Section::Exponent;
      section_size = 0;
      continue;
    }

    return false;
  }

  if (section_size == 0)
    return false;

  return true;
}

// <integer>[.<fraction>][e/E[sign]<exponent>]
bool validate_hex(const std::string &str) {
  enum class Section { Integer, Fraction, Exponent };
  size_t section_size = 0;

  if (!starts_with(str, "0x") && !starts_with(str, "0X"))
    return false;

  // just prefix
  if (str.length() - 2 == 0)
    return false;

  Section section = Section::Integer;
  for (size_t i = 2; i < str.length(); i++) {
    unsigned char c = str[i];

    if (c == '\'') {
      if (str[i - 1] == '\'')
        return false;

      continue;
    }

    if (section == Section::Exponent ? std::isdigit(c) : std::isxdigit(c)) {
      section_size++;
      continue;
    }

    if (c == '.') {
      if (section != Section::Integer || section_size == 0)
        return false;

      section = Section::Fraction;
      section_size = 0;
      continue;
    }

    if (c == 'p' || c == 'P') {
      if (section == Section::Exponent || section_size == 0)
        return false;

      if (i + 1 >= str.length())
        return false;

      if (str[i + 1] == '+' || str[i + 1] == '-')
        i++;

      section = Section::Exponent;
      section_size = 0;
      continue;
    }

    return false;
  }

  if (section_size == 0)
    return false;

  return true;
}

bool validate_oct(const std::string &str) {
  if (!starts_with(str, "0o") && !starts_with(str, "0O"))
    return false;

  // just prefix
  if (str.length() - 2 == 0)
    return false;

  for (size_t i = 2; i < str.length(); i++) {
    unsigned char c = str[i];

    if (c == '\'') {
      if (str[i - 1] == '\'')
        return false;

      continue;
    }

    if (c >= '0' && c <= '7')
      continue;

    return false;
  }

  return true;
}

bool validate_bin(const std::string &str) {
  if (!starts_with(str, "0b") && !starts_with(str, "0B"))
    return false;

  // just prefix
  if (str.length() - 2 == 0)
    return false;

  for (size_t i = 2; i < str.length(); i++) {
    unsigned char c = str[i];

    if (c == '\'') {
      if (str[i - 1] == '\'')
        return false;

      continue;
    }

    if (c == '0' || c == '1')
      continue;

    return false;
  }

  return true;
}

int main (int argc, char *argv[]) {
  return 0;
}
