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

#if SHIMEJIFINDER_USE_ICONV

#include "../utf8_convert.hpp"
#include <iconv.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <array>

namespace shimejifinder {

bool is_valid_utf8(const std::string &str) {
    iconv_t cd = iconv_open("UTF-8", "UTF-8");
    if (cd == (iconv_t)-1) {
        std::cerr << "shimejifinder: iconv_open failed: " << strerror(errno) << std::endl;
        return false;
    }

    std::vector<char> inbuf(str.begin(), str.end());
    std::array<char, 256> outbuf;
    char *outbufPt, *inbufPt = &inbuf[0];
    size_t outleft, inleft = inbuf.size();

    size_t result;
    do {
        errno = 0;
        outbufPt = &outbuf[0];
        outleft = outbuf.size();
        result = iconv(cd, &inbufPt, &inleft, &outbufPt, &outleft);
    }
    while (errno == E2BIG);

    iconv_close(cd);
    bool valid = result != (size_t)-1;
    return valid;
}

bool shift_jis_to_utf8(std::string &str) {
    iconv_t cd = iconv_open("UTF-8", "SHIFT-JIS");
    if (cd == (iconv_t)-1) {
        std::cerr << "shimejifinder: iconv_open failed: " << strerror(errno) << std::endl;
        return false;
    }

    std::vector<char> inbuf(str.begin(), str.end());
    std::vector<char> outbuf(str.size() * 4);
    std::string outstr;
    char *outbufPt = &outbuf[0], *inbufPt = &inbuf[0];
    size_t outleft = outbuf.size(), inleft = inbuf.size();

    size_t result = iconv(cd, &inbufPt, &inleft, &outbufPt, &outleft);
    iconv_close(cd);

    if (result == (size_t)-1) {
        std::cerr << "shimejifinder: iconv failed: " << strerror(errno) << std::endl;
        return false;
    }

    str = { outbuf.begin(), outbuf.begin() + (outbuf.size() - outleft) };
    return true;
}

}

#endif
