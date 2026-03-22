// Stub implementation of file_stream for platforms that don't need it
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { FILE *fp; } RFILE;

RFILE *filestream_open(const char *path, unsigned mode, unsigned hints)
{
    (void)hints;
    const char *m = "r";
    if (mode == 1) m = "r";
    else if (mode == 2) m = "w";

    RFILE *rf = (RFILE*)calloc(1, sizeof(RFILE));
    if (!rf) return NULL;
    rf->fp = fopen(path, m);
    if (!rf->fp) { free(rf); return NULL; }
    return rf;
}

char *filestream_gets(RFILE *stream, char *s, size_t len)
{
    if (!stream || !stream->fp) return NULL;
    return fgets(s, (int)len, stream->fp);
}

int filestream_close(RFILE *stream)
{
    if (!stream) return -1;
    if (stream->fp) fclose(stream->fp);
    free(stream);
    return 0;
}
