#include "gtest/gtest.h"
#include "lightconf/internal/scanner.hpp"

class ScannerTest : public ::testing::Test {
protected:
    lightconf::scanner sc;
};

TEST_F(ScannerTest, ScanString) {
    sc.scan("\"hello\\t\\f\\n\\r\\bworld\"");
    EXPECT_EQ("hello\t\f\n\r\bworld", sc.expect_string());
}

TEST_F(ScannerTest, ScanNumber) {
    sc.scan("1.34 -55 4e6 .4 -.6");
    EXPECT_EQ(1.34, sc.expect_number());
    EXPECT_EQ(-55, sc.expect_number());
    EXPECT_EQ(4e6, sc.expect_number());
    EXPECT_EQ(0.4, sc.expect_number());
    EXPECT_EQ(-0.6, sc.expect_number());
}

TEST_F(ScannerTest, ScanIdentifier) {
    sc.scan("hello there");
    EXPECT_EQ("hello", sc.expect_identifier());
    EXPECT_EQ("there", sc.expect_identifier());
}

TEST_F(ScannerTest, ScanMultipleWithComments) {
    std::vector<std::string> comments;
    sc.scan("{ list = [ 1,  // comment \n  2, \n \n \"\\u2603\" // another comment \n ] }",
        { [&](const std::string& str) { comments.push_back(str); },
        lightconf::allow_comments_flag | lightconf::blank_line_comment_flag });
    EXPECT_NO_THROW(sc.expect('{'));
    EXPECT_EQ("list", sc.expect_identifier());
    EXPECT_NO_THROW(sc.expect('='));
    EXPECT_NO_THROW(sc.expect('['));
    EXPECT_EQ(1, sc.expect_number());
    EXPECT_NO_THROW(sc.expect(','));
    EXPECT_EQ(2, sc.expect_number());
    EXPECT_EQ(1u, comments.size());
    EXPECT_NO_THROW(sc.expect(','));
    EXPECT_EQ(1u, comments.size());
    EXPECT_EQ(u8"☃", sc.expect_string());
    EXPECT_EQ(2u, comments.size());
    EXPECT_NO_THROW(sc.expect(']'));
    EXPECT_EQ(3u, comments.size());
    EXPECT_NO_THROW(sc.expect('}'));

    std::vector<std::string> comments_expected = {"// comment ", "", "// another comment "};
    EXPECT_EQ(comments_expected, comments);
}

TEST_F(ScannerTest, UTF8_OneByte) {
    sc.scan("\"Letter 'a': \\u0061\"");
    EXPECT_EQ("Letter 'a': a", sc.expect_string());
}

TEST_F(ScannerTest, UTF8_TwoByte) {
    sc.scan("\"Cents \\u00A2\"");
    EXPECT_EQ(u8"Cents ¢", sc.expect_string());
}

TEST_F(ScannerTest, UTF8_ThreeByte) {
    sc.scan("\"Euro \\u20ac\"");
    EXPECT_EQ(u8"Euro €", sc.expect_string());
}

TEST_F(ScannerTest, UTF8_FourByte) {
    sc.scan("\"G Clef \\ud834\\udd1e\"");
    EXPECT_EQ(u8"G Clef 𝄞", sc.expect_string());
}

TEST_F(ScannerTest, UTF8_Bogus_NoExcept) {
    sc.scan("\"Bogus \\ud834 \\u12 hello\"");
    EXPECT_EQ(u8"Bogus \ufffd \ufffd hello", sc.expect_string());
}

TEST_F(ScannerTest, UTF8_Bogus_Except) {
    EXPECT_THROW({
        sc.scan("\"Bogus \\ud834\"", { nullptr, lightconf::utf8_exceptions_flag });
        sc.expect_string();
    }, lightconf::utf8_error);
}
