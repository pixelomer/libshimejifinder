#pragma once

#if !SHIMEJIFINDER_NO_LIBUNARR

#include <unarr.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
#endif
ar_stream *ar_open_FILE(FILE *f);

#endif
