#ifndef SPY_GIRL_MAGGIE_STREAM_H
#define SPY_GIRL_MAGGIE_STREAM_H

#include <exec/types.h>

typedef struct SpyMaggieStream SpyMaggieStream;

SpyMaggieStream *SpyMaggieStreamLoad(const char *stream_name);
BOOL SpyMaggieStreamIsReady(const SpyMaggieStream *stream);
ULONG SpyMaggieStreamGetTicksPerFrame(const SpyMaggieStream *stream);
ULONG SpyMaggieStreamGetCurrentFrame(const SpyMaggieStream *stream);
ULONG SpyMaggieStreamGetFrameCount(const SpyMaggieStream *stream);
void SpyMaggieStreamUpdate(SpyMaggieStream *stream, ULONG time_ticks, ULONG ticks_per_frame);
void SpyMaggieStreamDraw(const SpyMaggieStream *stream);
void SpyMaggieStreamFree(SpyMaggieStream *stream);

#endif
