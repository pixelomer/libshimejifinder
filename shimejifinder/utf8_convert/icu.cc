// 
// libshimejifinder - library for finding and extracting shimeji from archives
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

#if SHIMEJIFINDER_USE_ICU

#include "../utf8_convert.hpp"
#include <unicode/unistr.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <iostream>
#include <vector>

namespace shimejifinder {

bool is_valid_utf8(const std::string &str) {
    UChar32 c = 0;
    for (int32_t i=0, len=(int32_t)str.size(); i<len && c>=0; ) {
        U8_NEXT(str.c_str(), i, len, c);
    }
    return c >= 0;
}

bool shift_jis_to_utf8(std::string &str) {
    UErrorCode status = U_ZERO_ERROR;

    // create converter
    UConverter *conv = ucnv_open("Shift_JIS", &status);
    if (U_FAILURE(status)) {
        std::cerr << "shimejifinder: ucnv_open failed: " << status << std::endl;
        return false;
    }

    // get utf-16 length
    int32_t utf16len = ucnv_toUChars(conv, nullptr, 0, str.c_str(), str.size(), &status);
    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        ucnv_close(conv);
        std::cerr << "shimejifinder: ucnv_toUChars: failed to get "
            "utf-16 length: " << status << std::endl;
        return false;
    }

    // convert shift_jis to utf-16
    status = U_ZERO_ERROR;
    std::vector<UChar> utf16(utf16len + 1);
    ucnv_toUChars(conv, &utf16[0], utf16len + 1, str.c_str(), str.size(), &status);
    ucnv_close(conv);
    if (U_FAILURE(status)) {
        std::cerr << "shimejifinder: ucnv_toUChars: utf-16 conversion "
            "failed: " << status << std::endl;
        return false;
    }

    // get utf-8 length
    int32_t utf8len = 0;
    u_strToUTF8(nullptr, 0, &utf8len, &utf16[0], utf16len, &status);
    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        std::cerr << "shimejifinder: u_strToUTF8: failed to get "
            "utf-8 length: " << status << std::endl;
        return false;
    }

    // convert utf-16 to utf-8
    status = U_ZERO_ERROR;
    std::vector<char> utf8(utf8len + 1);
    u_strToUTF8(&utf8[0], utf8len + 1, nullptr, &utf16[0], utf16len, &status);
    if (U_FAILURE(status)) {
        std::cerr << "shimejifinder: u_strToUTF8: utf-8 conversion "
            "failed: " << status << std::endl;
        return false;
    }

    // assign new utf-8 data to string
    str = { &utf8[0], (size_t)utf8len };
    return true;
}

}

#endif
