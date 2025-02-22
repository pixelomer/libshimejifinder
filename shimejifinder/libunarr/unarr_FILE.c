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

#if !SHIMEJIFINDER_NO_LIBUNARR

// adapted from unarr-imp.h and stream.c
// required for interop with Java

#include <stdio.h>
#include <stdlib.h>
#include "unarr_FILE.h"

typedef void (* ar_stream_close_fn)(void *data);
typedef size_t (* ar_stream_read_fn)(void *data, void *buffer, size_t count);
typedef bool (* ar_stream_seek_fn)(void *data, off64_t offset, int origin);
typedef off64_t (* ar_stream_tell_fn)(void *data);

struct ar_stream_s {
    ar_stream_close_fn close;
    ar_stream_read_fn read;
    ar_stream_seek_fn seek;
    ar_stream_tell_fn tell;
    void *data;
};

static void file_close(void *data)
{
    (void)data;
    /* library user is expected to close the file */
    //fclose(data);
}

static size_t file_read(void *data, void *buffer, size_t count)
{
    return fread(buffer, 1, count, (FILE *)data);
}

static bool file_seek(void *data, off64_t offset, int origin)
{
#ifdef _WIN32
    return _fseeki64(data, offset, origin) == 0;
#else
#if _POSIX_C_SOURCE >= 200112L
    if (sizeof(off_t) == 8)
        return fseeko(data, offset, origin) == 0;
#endif
    if (offset > INT32_MAX || offset < INT32_MIN)
        return false;
    return fseek(data, (long)offset, origin) == 0;
#endif
}

static off64_t file_tell(void *data)
{
#ifdef _WIN32
    return _ftelli64(data);
#elif _POSIX_C_SOURCE >= 200112L
    return ftello(data);
#else
    return ftell(data);
#endif
}

static ar_stream *ar_open_stream(void *data, ar_stream_close_fn close, ar_stream_read_fn read, ar_stream_seek_fn seek, ar_stream_tell_fn tell)
{
    if (data == NULL) {
        return NULL;
    }
    ar_stream *stream = malloc(sizeof(ar_stream));
    if (!stream) {
        fclose(data);
        return NULL;
    }
    stream->data = data;
    stream->close = close;
    stream->read = read;
    stream->seek = seek;
    stream->tell = tell;
    return stream;
}

ar_stream *ar_open_FILE(FILE *f)
{
    return ar_open_stream(f, file_close, file_read, file_seek, file_tell);
}

#else
// ISO C forbids an empty translation unit
extern int dummy(void);
#endif
