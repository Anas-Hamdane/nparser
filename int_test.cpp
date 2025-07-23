#include <string>
#include <vector>
#include <iostream>

bool starts_with(const std::string &str, const std::string &cmp) {
  return (str.compare(0, cmp.length(), cmp) == 0);
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

struct TestStats {
  int total = 0;
  int passed = 0;
  int failed = 0;
  std::vector<std::string> failures;

  void reset() {
    total = passed = failed = 0;
    failures.clear();
  }

  void print_summary(const std::string &section_name = "") {
    if (!section_name.empty()) {
      std::cout << "\n--- " << section_name << " SUMMARY ---\n";
    }
    std::cout << "Tests: " << passed << "/" << total << " passed";
    if (total > 0) {
      std::cout << " (" << (100.0 * passed / total) << "%)";
    }
    std::cout << '\n';
  }
};

TestStats stats;

void test_integer(const std::string &s, bool expected,
                  const std::string &description = "") {
  bool result = valid_integer(s);
  stats.total++;

  std::cout << "\"" << s << "\"";
  if (!description.empty()) {
    std::cout << " (" << description << ")";
  }
  std::cout << " : " << (result ? "Valid" : "Invalid");

  if (result == expected) {
    std::cout << " ✓\n";
    stats.passed++;
  } else {
    std::cout << " ✗ FAIL\n";
    std::cout << "  Expected: " << (expected ? "Valid" : "Invalid") << '\n';
    stats.failed++;
    stats.failures.push_back(s);
  }
}

int main() {
  std::cout << "=== COMPLETE INTEGER VALIDATION TEST SUITE ===\n";
  std::cout
      << "Testing: Decimal, Hexadecimal, Octal (0o/0O), Binary (0b/0B)\n\n";

  // Test 1: Basic Binary Support
  std::cout << "--- BINARY PREFIX SUPPORT ---\n";
  test_integer("0b0", true, "basic 0b prefix");
  test_integer("0B0", true, "basic 0B prefix");
  test_integer("0b1", true, "0b with digit 1");
  test_integer("0b01", true, "0b with both digits");
  test_integer("0b10", true, "0b binary 2");
  test_integer("0b11", true, "0b binary 3");
  test_integer("0B01010101", true, "0B alternating pattern");
  test_integer("0b11111111", true, "0b all ones");
  test_integer("0B00000000", true, "0B all zeros");
  test_integer("0b1010110011010", true, "0b long pattern");
  stats.print_summary("BASIC BINARY");

  // Test 2: Invalid Binary Digits
  std::cout << "\n--- INVALID BINARY DIGITS ---\n";
  test_integer("0b2", false, "0b with digit 2");
  test_integer("0b3", false, "0b with digit 3");
  test_integer("0b9", false, "0b with digit 9");
  test_integer("0Ba", false, "0B with letter a");
  test_integer("0bF", false, "0b with hex digit F");
  test_integer("0b012", false, "0b with mixed valid/invalid");
  test_integer("0b101234", false, "0b starting valid then invalid");
  test_integer("0b987654321", false, "0b all invalid digits");
  test_integer("0bABCDEF", false, "0b with hex letters");
  stats.print_summary("INVALID BINARY");

  // Test 3: Signed Binary Numbers
  std::cout << "\n--- SIGNED BINARY NUMBERS ---\n";
  test_integer("+0b1010", true, "positive binary");
  test_integer("-0b1010", true, "negative binary");
  test_integer("+0B1111", true, "positive binary uppercase");
  test_integer("-0B0000", true, "negative binary zeros");
  test_integer("+0b0", true, "positive binary zero");
  test_integer("-0b1", true, "negative binary one");
  stats.print_summary("SIGNED BINARY");

  // Test 4: Empty and Malformed Binary
  std::cout << "\n--- EMPTY AND MALFORMED BINARY ---\n";
  test_integer("0b", false, "empty 0b prefix");
  test_integer("0B", false, "empty 0B prefix");
  test_integer("0bb", false, "double b");
  test_integer("0BB", false, "double B");
  test_integer("0bx", false, "0b mixed with x");
  test_integer("0bo", false, "0b mixed with o");
  test_integer("b1010", false, "missing leading 0");
  test_integer("B1010", false, "missing leading 0 uppercase");
  test_integer("00b1010", false, "extra leading 0");
  stats.print_summary("MALFORMED BINARY");

  // Test 5: All Number Systems Side by Side
  std::cout << "\n--- ALL NUMBER SYSTEMS COMPARISON ---\n";
  test_integer("10", true, "decimal 10");
  test_integer("0x10", true, "hex 10 (16 decimal)");
  test_integer("0o10", true, "octal 10 (8 decimal)");
  test_integer("0b10", true, "binary 10 (2 decimal)");
  test_integer("255", true, "decimal 255");
  test_integer("0xFF", true, "hex FF (255 decimal)");
  test_integer("0o377", true, "octal 377 (255 decimal)");
  test_integer("0b11111111", true, "binary 11111111 (255 decimal)");
  stats.print_summary("SYSTEM COMPARISON");

  // Test 6: Cross-System Invalid Digits
  std::cout << "\n--- CROSS-SYSTEM INVALID DIGITS ---\n";
  test_integer("0b8", false, "binary with octal digit");
  test_integer("0b9", false, "binary with decimal digit");
  test_integer("0bA", false, "binary with hex digit");
  test_integer("0o8", false, "octal with decimal digit");
  test_integer("0o9", false, "octal with decimal digit");
  test_integer("0oA", false, "octal with hex digit");
  test_integer("0xG", false, "hex with invalid letter");
  test_integer("0xZ", false, "hex with invalid letter");
  test_integer("123G", false, "decimal with letter");
  stats.print_summary("CROSS-SYSTEM INVALID");

  // Test 7: Decimal with Leading Zeros (Should be Valid)
  std::cout << "\n--- DECIMAL WITH LEADING ZEROS ---\n";
  test_integer("0", true, "single zero");
  test_integer("00", true, "double zero");
  test_integer("000", true, "triple zero");
  test_integer("0123", true, "decimal with leading zero");
  test_integer("0456", true, "decimal with leading zero");
  test_integer("0789", true, "decimal with leading zero");
  test_integer("0123456789", true, "all decimal digits with leading zero");
  stats.print_summary("DECIMAL LEADING ZEROS");

  // Test 8: Case Sensitivity
  std::cout << "\n--- CASE SENSITIVITY ---\n";
  test_integer("0xff", true, "hex lowercase");
  test_integer("0XFF", true, "hex uppercase");
  test_integer("0xAb", true, "hex mixed case");
  test_integer("0b1010", true, "binary lowercase");
  test_integer("0B1010", true, "binary uppercase");
  test_integer("0o777", true, "octal lowercase");
  test_integer("0O777", true, "octal uppercase");
  stats.print_summary("CASE SENSITIVITY");

  // Test 9: Whitespace and Special Characters
  std::cout << "\n--- WHITESPACE AND SPECIAL CHARACTERS ---\n";
  test_integer(" 123", false, "leading space");
  test_integer("123 ", false, "trailing space");
  test_integer("1 23", false, "middle space");
  test_integer("0x 10", false, "space after hex prefix");
  test_integer("0b 10", false, "space after binary prefix");
  test_integer("0o 10", false, "space after octal prefix");
  test_integer("123\t", false, "tab character");
  test_integer("123\n", false, "newline character");
  test_integer("123.", false, "decimal point");
  test_integer("123,456", false, "comma separator");
  stats.print_summary("WHITESPACE");

  // Test 10: Sign Edge Cases
  std::cout << "\n--- SIGN EDGE CASES ---\n";
  test_integer("+", false, "just plus");
  test_integer("-", false, "just minus");
  test_integer("++123", false, "double plus");
  test_integer("--123", false, "double minus");
  test_integer("+-123", false, "plus minus");
  test_integer("-+123", false, "minus plus");
  test_integer("1+23", false, "plus in middle");
  test_integer("12-3", false, "minus in middle");
  test_integer("+0", true, "positive zero");
  test_integer("-0", true, "negative zero");
  stats.print_summary("SIGN EDGE CASES");

  // Test 11: Complex Mixed Patterns
  std::cout << "\n--- COMPLEX MIXED PATTERNS ---\n";
  test_integer("0x0b10", true, "hex prefix with binary-looking number");
  test_integer("0b0x10", false, "binary prefix with hex-looking number");
  test_integer("0o0b10", false, "octal prefix with binary-looking number");
  test_integer("0x0o10", false, "hex prefix with octal-looking number");
  test_integer("0xABCDEF123456", true, "long valid hex");
  test_integer("0b101010101010101010", true, "long valid binary");
  test_integer("0o1234567012345670", true, "long valid octal");
  test_integer("123456789012345678", true, "long valid decimal");
  stats.print_summary("MIXED PATTERNS");

  // Test 12: Boundary and Stress Tests
  std::cout << "\n--- BOUNDARY AND STRESS TESTS ---\n";
  test_integer(std::string(1000, '1'), true, "very long decimal");
  test_integer("0x" + std::string(1000, 'F'), true, "very long hex");
  test_integer("0o" + std::string(1000, '7'), true, "very long octal");
  test_integer("0b" + std::string(1000, '1'), true, "very long binary");
  test_integer("0b" + std::string(10000, '0'), true,
               "extremely long binary zeros");
  stats.print_summary("STRESS TESTS");

  // Test 13: Real-world Examples
  std::cout << "\n--- REAL-WORLD EXAMPLES ---\n";
  test_integer("42", true, "answer to everything");
  test_integer("0xFF", true, "common hex byte");
  test_integer("0o755", true, "Unix file permissions");
  test_integer("0b11010010", true, "8-bit binary value");
  test_integer("2147483647", true, "max 32-bit signed int");
  test_integer("0xDEADBEEF", true, "famous hex constant");
  test_integer("0b1111111111111111", true, "16-bit all ones");
  test_integer("0o37777777777", true, "large octal");
  stats.print_summary("REAL-WORLD");

  // Test 14: Error Cases That Should Definitely Fail
  std::cout << "\n--- DEFINITELY INVALID CASES ---\n";
  test_integer("", false, "empty string");
  test_integer("abc", false, "pure letters");
  test_integer("0xyz", false, "invalid prefix");
  test_integer("123abc", false, "number then letters");
  test_integer("0x", false, "hex prefix only");
  test_integer("0o", false, "octal prefix only");
  test_integer("0b", false, "binary prefix only");
  test_integer("123.456", false, "floating point");
  test_integer("1e10", false, "scientific notation");
  test_integer("∞", false, "infinity symbol");
  stats.print_summary("INVALID CASES");

  // Final comprehensive statistics
  std::cout << "\n=== FINAL COMPREHENSIVE STATISTICS ===\n";
  std::cout << "Total tests run: " << stats.total << '\n';
  std::cout << "Tests passed: " << stats.passed << " ("
            << (stats.total > 0 ? (100.0 * stats.passed / stats.total) : 0)
            << "%)\n";
  std::cout << "Tests failed: " << stats.failed << " ("
            << (stats.total > 0 ? (100.0 * stats.failed / stats.total) : 0)
            << "%)\n";

  if (stats.failed > 0) {
    std::cout << "\n=== FAILED TEST CASES ===\n";
    for (size_t i = 0; i < stats.failures.size(); ++i) {
      std::cout << (i + 1) << ". \"" << stats.failures[i] << "\"\n";
    }

    std::cout << "\n=== ANALYSIS OF FAILURES ===\n";
    std::cout << "Your implementation looks much cleaner now!\n";
    std::cout << "Key improvements:\n";
    std::cout << "✅ Uses string modification instead of index tracking\n";
    std::cout << "✅ Handles signs correctly by erasing them first\n";
    std::cout << "✅ Proper 2-character prefix handling\n";
    std::cout
        << "✅ Supports all 4 number systems (decimal/hex/octal/binary)\n\n";

    std::cout << "If there are still failures, they might be due to:\n";
    std::cout << "- Edge cases in isxdigit() vs custom character checking\n";
    std::cout << "- Locale-specific behavior of character classification\n";
    std::cout << "- Unicode or special character handling\n";
  } else {
    std::cout << "\n🎊 ABSOLUTELY PERFECT! 🎊\n";
    std::cout << "🏆 Your function handles ALL edge cases correctly!\n";
    std::cout << "🚀 This is production-ready integer validation code!\n";
  }

  std::cout << "\n=== SUPPORTED NUMBER SYSTEMS ===\n";
  std::cout << "📊 Decimal:     123, +456, -789, 0123 (leading zeros OK)\n";
  std::cout << "🔢 Hexadecimal: 0x1A, 0XFF, +0xDEAD, -0xBEEF\n";
  std::cout << "🔡 Octal:       0o777, 0O123, +0o456, -0O321\n";
  std::cout << "💻 Binary:      0b1010, 0B1111, +0b0101, -0B1100\n";
  std::cout << "❌ Invalid:     0x, 0o, 0b, spaces, wrong digits for system\n";

  std::cout << "\n=== TEST SUITE COMPLETE! ===\n";
  return 0;
}
