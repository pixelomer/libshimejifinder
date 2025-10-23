#include <shimejifinder/utf8_convert.hpp>
#include <gtest/gtest.h>

// <is_utf8, original, converted_utf8>
static const std::vector<std::tuple<bool, std::string, std::string>> test_cases = {
    { true,  "Привет", "" },
    { true,  "こんにちは", "" },
    { true,  "Merhaba dünya", "" },
    { false, "\x82\xb1\x82\xf1\x82\xc9\x82\xbf\x82\xcd", "こんにちは" }
};

TEST(UTF8ConvertTest, ValidateUTF8) {
    for (size_t i=0; i<test_cases.size(); ++i) {
        auto const& [is_utf8, original, converted] = test_cases[i];
        EXPECT_EQ(shimejifinder::is_valid_utf8(original), is_utf8) <<
            "Expected string #" << i << " to be " <<
            (is_utf8 ? "valid" : "invalid") << " UTF-8";
    }
}

TEST(UTF8ConvertTest, ConvertShiftJIS) {
    for (size_t i=0; i<test_cases.size(); ++i) {
        auto const& [is_utf8, original, converted] = test_cases[i];
        if (is_utf8) {
            continue;
        }
        std::string output = original;
        bool ret = shimejifinder::shift_jis_to_utf8(output);
        EXPECT_EQ(ret, true) << "Expected string #" << i <<
            " conversion to succeed";
        EXPECT_EQ(output, converted) << "Converted string #" << i <<
            " value is incorrect";
    }
}
