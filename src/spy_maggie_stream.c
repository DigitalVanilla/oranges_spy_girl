#include <exec/types.h>
#include <proto/Maggie.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <maggie_flags.h>
#include <maggie_vec.h>
#include <maggie_vertex.h>
#include <sage/sage_error.h>

#include "spy_config.h"
#include "spy_log.h"
#include "spy_maggie_stream.h"

#define SPY_MAGGIE_STREAM_INVALID_HANDLE 0xffff
#define SPY_MAGGIE_STREAM_MAGIC "MRPH"
#define SPY_MAGGIE_STREAM_FORMAT_VERSION 0x00010000UL
#define SPY_MAGGIE_STREAM_PLAYER_VERSION 0x00010000UL
#define SPY_MAGGIE_STREAM_TEXTURE_FORMAT_DXT1 4
#define SPY_MAGGIE_STREAM_TEXTURE_SIZE SPY_MAGGIE_TEXTURE_SIZE
#define SPY_MAGGIE_STREAM_TEXTURE_WIDTH (1UL << SPY_MAGGIE_STREAM_TEXTURE_SIZE)
#define SPY_MAGGIE_STREAM_MIN_MIP_TEXTURE_SIZE 5
#define SPY_MAGGIE_STREAM_HEADER_BYTES 52UL
#define SPY_MAGGIE_STREAM_VERTEX_BYTES 20UL
#define SPY_MAGGIE_STREAM_INDEX_BYTES 2UL
#define SPY_MAGGIE_STREAM_POSITION_BYTES 12UL
#define SPY_MAGGIE_STREAM_NORMAL_BYTES 3UL

typedef struct SpyMaggieStreamVertex {
  vec3 pos;
  float u;
  float v;
} SpyMaggieStreamVertex;

typedef struct SpyMaggieStreamFrame {
  vec3 *positions;
} SpyMaggieStreamFrame;

struct SpyMaggieStream {
  UWORD vertex_buffer;
  UWORD index_buffer;
  UWORD texture;
  UWORD vertex_count;
  UWORD index_count;
  UWORD texture_size;
  ULONG frame_count;
  ULONG time_per_frame;
  ULONG timing_origin_ticks;
  ULONG active_ticks_per_frame;
  ULONG last_uploaded_frame;
  BOOL frame_uploaded;
  vec3 bounds_size;
  vec3 bounds_min;
  vec3 *base_positions;
  vec3 *scratch_positions;
  SpyMaggieStreamFrame *frames;
};

static BOOL ReadExact(FILE *fp, void *data, size_t size)
{
  return fread(data, 1, size, fp) == size;
}

static BOOL ReadUlongBE(FILE *fp, ULONG *value)
{
  UBYTE bytes[4];

  if (!ReadExact(fp, bytes, sizeof(bytes))) {
    return FALSE;
  }

  *value =
    ((ULONG)bytes[0] << 24) |
    ((ULONG)bytes[1] << 16) |
    ((ULONG)bytes[2] << 8) |
    ((ULONG)bytes[3]);
  return TRUE;
}

static BOOL ReadUwordBE(FILE *fp, UWORD *value)
{
  UBYTE bytes[2];

  if (!ReadExact(fp, bytes, sizeof(bytes))) {
    return FALSE;
  }

  *value = (UWORD)(((UWORD)bytes[0] << 8) | (UWORD)bytes[1]);
  return TRUE;
}

static BOOL ReadFloatBE(FILE *fp, float *value)
{
  union {
    ULONG ulong_value;
    float float_value;
  } bits;

  if (!ReadUlongBE(fp, &bits.ulong_value)) {
    return FALSE;
  }

  *value = bits.float_value;
  return TRUE;
}

static BOOL ReadVec3BE(FILE *fp, vec3 *value)
{
  return
    ReadFloatBE(fp, &value->x) &&
    ReadFloatBE(fp, &value->y) &&
    ReadFloatBE(fp, &value->z);
}

static BOOL ReadStreamVertex(FILE *fp, SpyMaggieStreamVertex *vertex)
{
  return
    ReadVec3BE(fp, &vertex->pos) &&
    ReadFloatBE(fp, &vertex->u) &&
    ReadFloatBE(fp, &vertex->v);
}

static BOOL ReadPackedNormal(FILE *fp, vec3 *normal)
{
  UBYTE bytes[3];

  if (!ReadExact(fp, bytes, sizeof(bytes))) {
    return FALSE;
  }

  normal->x = (float)((BYTE)bytes[0]) / 127.0f;
  normal->y = (float)((BYTE)bytes[1]) / 127.0f;
  normal->z = (float)((BYTE)bytes[2]) / 127.0f;
  vec3_normalise(normal, normal);
  return TRUE;
}

static UWORD TextureSizeFromWidth(ULONG width)
{
  UWORD texture_size = 0;

  if (width == 0) {
    return 0;
  }

  while (width > 1) {
    if ((width & 1UL) != 0) {
      return 0;
    }
    width >>= 1;
    texture_size++;
  }

  return texture_size;
}

static ULONG Dxt1MipChainSize(UWORD texture_size)
{
  ULONG total_size = 0;
  ULONG width = 1UL << texture_size;
  ULONG height = width;

  while (texture_size >= SPY_MAGGIE_STREAM_MIN_MIP_TEXTURE_SIZE) {
    total_size += (width * height) / 2UL;
    texture_size--;
    width >>= 1;
    height >>= 1;
  }

  return total_size;
}

static ULONG StreamTextureHeaderOffset(ULONG frame_count, ULONG vertex_count, ULONG index_count)
{
  return
    SPY_MAGGIE_STREAM_HEADER_BYTES +
    (vertex_count * SPY_MAGGIE_STREAM_VERTEX_BYTES) +
    (index_count * SPY_MAGGIE_STREAM_INDEX_BYTES) +
    (frame_count * vertex_count * (SPY_MAGGIE_STREAM_POSITION_BYTES + SPY_MAGGIE_STREAM_NORMAL_BYTES));
}

static void UploadDxt1MipChain(UWORD texture, UWORD texture_size, UBYTE *texture_data)
{
  ULONG width = 1UL << texture_size;
  ULONG height = width;
  UBYTE *data = texture_data;

  while (texture_size >= SPY_MAGGIE_STREAM_MIN_MIP_TEXTURE_SIZE) {
    magUploadTexture(texture, texture_size, data, MAG_TEXFMT_DXT1);
    data += (width * height) / 2UL;
    texture_size--;
    width >>= 1;
    height >>= 1;
  }
}

static SpyMaggieStream *AllocStream(void)
{
  SpyMaggieStream *stream = malloc(sizeof(SpyMaggieStream));

  if (stream == NULL) {
    return NULL;
  }

  memset(stream, 0, sizeof(SpyMaggieStream));
  stream->vertex_buffer = SPY_MAGGIE_STREAM_INVALID_HANDLE;
  stream->index_buffer = SPY_MAGGIE_STREAM_INVALID_HANDLE;
  stream->texture = SPY_MAGGIE_STREAM_INVALID_HANDLE;
  stream->last_uploaded_frame = 0xffffffffUL;
  return stream;
}

BOOL SpyMaggieStreamIsReady(const SpyMaggieStream *stream)
{
  return
    stream != NULL &&
    stream->vertex_buffer != SPY_MAGGIE_STREAM_INVALID_HANDLE &&
    stream->index_buffer != SPY_MAGGIE_STREAM_INVALID_HANDLE &&
    stream->texture != SPY_MAGGIE_STREAM_INVALID_HANDLE &&
    stream->vertex_count > 0 &&
    stream->index_count > 0 &&
    stream->frame_count > 1 &&
    stream->base_positions != NULL &&
    stream->scratch_positions != NULL &&
    stream->frames != NULL;
}

ULONG SpyMaggieStreamGetTicksPerFrame(const SpyMaggieStream *stream)
{
  if (stream == NULL) {
    return 0;
  }

  return stream->time_per_frame;
}

ULONG SpyMaggieStreamGetCurrentFrame(const SpyMaggieStream *stream)
{
  if (!SpyMaggieStreamIsReady(stream) || !stream->frame_uploaded) {
    return 0;
  }

  return stream->last_uploaded_frame;
}

ULONG SpyMaggieStreamGetFrameCount(const SpyMaggieStream *stream)
{
  if (!SpyMaggieStreamIsReady(stream)) {
    return 0;
  }

  return stream->frame_count;
}

SpyMaggieStream *SpyMaggieStreamLoad(const char *stream_name)
{
  FILE *fp = NULL;
  SpyMaggieStream *stream = NULL;
  SpyMaggieStreamVertex *stream_vertices = NULL;
  struct MaggieVertex *maggie_vertices = NULL;
  vec3 *initial_normals = NULL;
  UWORD *indices = NULL;
  UBYTE *texture_data = NULL;
  char magic[4];
  ULONG format_version;
  ULONG min_player_version;
  ULONG time_per_frame;
  ULONG frame_count;
  ULONG vertex_count;
  ULONG index_count;
  ULONG texture_width;
  ULONG texture_height;
  ULONG texture_format;
  ULONG texture_data_size;
  ULONG texture_header_offset;
  UWORD texture_size;
  ULONG frame_index;
  ULONG vertex_index;
  ULONG index;

  if (stream_name == NULL) {
    SpyLog("Maggie stream not loaded: invalid stream name\n");
    SAGE_SetError(SERR_NULL_POINTER);
    return NULL;
  }
  if (MaggieBase == NULL) {
    SpyLog("Maggie stream not loaded: maggie.library is not open\n");
    SAGE_SetError(SERR_NO_MAGGIE);
    return NULL;
  }

  fp = fopen(stream_name, "rb");
  if (fp == NULL) {
    SpyLog("Maggie stream not loaded: can't open %s\n", stream_name);
    SAGE_SetError(SERR_OPENFILE);
    return NULL;
  }

  stream = AllocStream();
  if (stream == NULL) {
    SpyLog("Maggie stream not loaded: not enough memory\n");
    SAGE_SetError(SERR_NO_MEMORY);
    goto fail;
  }

  if (!ReadExact(fp, magic, sizeof(magic)) || memcmp(magic, SPY_MAGGIE_STREAM_MAGIC, sizeof(magic)) != 0) {
    SpyLog("Maggie stream not loaded: %s is not an MPT stream\n", stream_name);
    SAGE_SetError(SERR_FILEFORMAT);
    goto fail;
  }

  if (
    !ReadUlongBE(fp, &format_version) ||
    !ReadUlongBE(fp, &min_player_version) ||
    !ReadUlongBE(fp, &time_per_frame) ||
    !ReadUlongBE(fp, &frame_count) ||
    !ReadUlongBE(fp, &vertex_count) ||
    !ReadUlongBE(fp, &index_count) ||
    !ReadVec3BE(fp, &stream->bounds_size) ||
    !ReadVec3BE(fp, &stream->bounds_min)
  ) {
    SpyLog("Maggie stream not loaded: short header in %s\n", stream_name);
    SAGE_SetError(SERR_READFILE);
    goto fail;
  }

  if (format_version != SPY_MAGGIE_STREAM_FORMAT_VERSION) {
    SpyLog(
      "Maggie stream not loaded: unsupported MPT format in %s (0x%08lx, expected 0x%08lx)\n",
      stream_name,
      format_version,
      SPY_MAGGIE_STREAM_FORMAT_VERSION
    );
    SAGE_SetError(SERR_FILEFORMAT);
    goto fail;
  }

  if (min_player_version > SPY_MAGGIE_STREAM_PLAYER_VERSION) {
    SpyLog(
      "Maggie stream not loaded: %s requires MPT player 0x%08lx (current 0x%08lx)\n",
      stream_name,
      min_player_version,
      SPY_MAGGIE_STREAM_PLAYER_VERSION
    );
    SAGE_SetError(SERR_FILEFORMAT);
    goto fail;
  }

  if (time_per_frame == 0 || frame_count < 2 || vertex_count == 0 || index_count == 0) {
    SpyLog("Maggie stream not loaded: invalid stream counts in %s\n", stream_name);
    SAGE_SetError(SERR_FILEFORMAT);
    goto fail;
  }
  if (vertex_count > 0xfffcUL || index_count > 0xffffUL) {
    SpyLog("Maggie stream not loaded: stream is too large for Maggie buffers\n");
    SAGE_SetError(SERR_NO_MEMORY);
    goto fail;
  }

  stream->time_per_frame = time_per_frame;
  stream->frame_count = frame_count;
  stream->vertex_count = (UWORD)vertex_count;
  stream->index_count = (UWORD)index_count;

  stream_vertices = malloc(sizeof(SpyMaggieStreamVertex) * vertex_count);
  indices = malloc(sizeof(UWORD) * index_count);
  stream->frames = malloc(sizeof(SpyMaggieStreamFrame) * frame_count);
  stream->base_positions = malloc(sizeof(vec3) * vertex_count);
  stream->scratch_positions = malloc(sizeof(vec3) * vertex_count);
  maggie_vertices = malloc(sizeof(struct MaggieVertex) * vertex_count);
  initial_normals = malloc(sizeof(vec3) * vertex_count);
  if (
    stream_vertices == NULL ||
    maggie_vertices == NULL ||
    initial_normals == NULL ||
    indices == NULL ||
    stream->frames == NULL ||
    stream->base_positions == NULL ||
    stream->scratch_positions == NULL
  ) {
    SpyLog("Maggie stream not loaded: not enough memory for %s\n", stream_name);
    SAGE_SetError(SERR_NO_MEMORY);
    goto fail;
  }
  memset(stream->frames, 0, sizeof(SpyMaggieStreamFrame) * frame_count);

  for (vertex_index = 0; vertex_index < vertex_count; vertex_index++) {
    if (!ReadStreamVertex(fp, &stream_vertices[vertex_index])) {
      SpyLog("Maggie stream not loaded: short vertex data in %s\n", stream_name);
      SAGE_SetError(SERR_READFILE);
      goto fail;
    }
  }

  for (index = 0; index < index_count; index++) {
    if (!ReadUwordBE(fp, &indices[index]) || indices[index] >= vertex_count) {
      SpyLog("Maggie stream not loaded: invalid index data in %s\n", stream_name);
      SAGE_SetError(SERR_FILEFORMAT);
      goto fail;
    }
  }

  for (frame_index = 0; frame_index < frame_count; frame_index++) {
    stream->frames[frame_index].positions = malloc(sizeof(vec3) * vertex_count);
    if (stream->frames[frame_index].positions == NULL) {
      SpyLog("Maggie stream not loaded: not enough memory for frame data\n");
      SAGE_SetError(SERR_NO_MEMORY);
      goto fail;
    }

    for (vertex_index = 0; vertex_index < vertex_count; vertex_index++) {
      if (!ReadVec3BE(fp, &stream->frames[frame_index].positions[vertex_index])) {
        SpyLog("Maggie stream not loaded: short frame position data in %s\n", stream_name);
        SAGE_SetError(SERR_READFILE);
        goto fail;
      }
    }
    if (frame_index == 0) {
      for (vertex_index = 0; vertex_index < vertex_count; vertex_index++) {
        if (!ReadPackedNormal(fp, &initial_normals[vertex_index])) {
          SpyLog("Maggie stream not loaded: short frame normal data in %s\n", stream_name);
          SAGE_SetError(SERR_READFILE);
          goto fail;
        }
      }
    } else {
      if (fseek(fp, (long)(vertex_count * SPY_MAGGIE_STREAM_NORMAL_BYTES), SEEK_CUR) != 0) {
        SpyLog("Maggie stream not loaded: short frame normal data in %s\n", stream_name);
        SAGE_SetError(SERR_READFILE);
        goto fail;
      }
    }
  }

  texture_header_offset = StreamTextureHeaderOffset(frame_count, vertex_count, index_count);
  if (fseek(fp, (long)texture_header_offset, SEEK_SET) != 0) {
    SpyLog("Maggie stream not loaded: cannot seek texture header in %s at %lu\n", stream_name, texture_header_offset);
    SAGE_SetError(SERR_READFILE);
    goto fail;
  }

  if (
    !ReadUlongBE(fp, &texture_width) ||
    !ReadUlongBE(fp, &texture_height) ||
    !ReadUlongBE(fp, &texture_format)
  ) {
    SpyLog("Maggie stream not loaded: missing texture data in %s\n", stream_name);
    SAGE_SetError(SERR_READFILE);
    goto fail;
  }

  texture_size = TextureSizeFromWidth(texture_width);
  if (
    texture_width != SPY_MAGGIE_STREAM_TEXTURE_WIDTH ||
    texture_height != SPY_MAGGIE_STREAM_TEXTURE_WIDTH ||
    texture_size != SPY_MAGGIE_STREAM_TEXTURE_SIZE ||
    texture_format != SPY_MAGGIE_STREAM_TEXTURE_FORMAT_DXT1
  ) {
    SpyLog(
      "Maggie stream not loaded: unsupported texture in %s at %lu (%lux%lu format %lu, expected %lux%lu DXT1)\n",
      stream_name,
      texture_header_offset,
      texture_width,
      texture_height,
      texture_format,
      SPY_MAGGIE_STREAM_TEXTURE_WIDTH,
      SPY_MAGGIE_STREAM_TEXTURE_WIDTH
    );
    SAGE_SetError(SERR_FILEFORMAT);
    goto fail;
  }

  texture_data_size = Dxt1MipChainSize(texture_size);
  texture_data = malloc(texture_data_size);
  if (texture_data == NULL) {
    SpyLog("Maggie stream not loaded: not enough memory for texture\n");
    SAGE_SetError(SERR_NO_MEMORY);
    goto fail;
  }
  if (!ReadExact(fp, texture_data, texture_data_size)) {
    SpyLog("Maggie stream not loaded: short texture data in %s\n", stream_name);
    SAGE_SetError(SERR_READFILE);
    goto fail;
  }

  stream->vertex_buffer = magAllocateVertexBuffer(stream->vertex_count);
  stream->index_buffer = magAllocateIndexBuffer(stream->index_count);
  stream->texture = magAllocateTexture(texture_size);
  stream->texture_size = texture_size;
  if (!SpyMaggieStreamIsReady(stream)) {
    SpyLog("Maggie stream not loaded: cannot allocate Maggie buffers\n");
    SAGE_SetError(SERR_NO_MEMORY);
    goto fail;
  }

  for (vertex_index = 0; vertex_index < vertex_count; vertex_index++) {
    stream->base_positions[vertex_index] = stream_vertices[vertex_index].pos;
    maggie_vertices[vertex_index].pos = stream_vertices[vertex_index].pos;
    maggie_vertices[vertex_index].normal = initial_normals[vertex_index];
    maggie_vertices[vertex_index].tex[0].u = stream_vertices[vertex_index].u;
    maggie_vertices[vertex_index].tex[0].v = stream_vertices[vertex_index].v;
    maggie_vertices[vertex_index].tex[0].w = 1.0f;
    maggie_vertices[vertex_index].colour = 0x00ffffff;
  }

  magUploadVertexBuffer(stream->vertex_buffer, maggie_vertices, 0, stream->vertex_count);
  magUploadIndexBuffer(stream->index_buffer, indices, 0, stream->index_count);
  UploadDxt1MipChain(stream->texture, texture_size, texture_data);

  free(initial_normals);
  free(maggie_vertices);
  free(texture_data);
  free(indices);
  free(stream_vertices);
  fclose(fp);

  SpyLog(
    "Maggie stream loaded: %s format=0x%08lx min_player=0x%08lx ticks=%lu frames=%lu vertices=%lu indices=%lu texture=%lux%lu\n",
    stream_name,
    format_version,
    min_player_version,
    time_per_frame,
    frame_count,
    vertex_count,
    index_count,
    texture_width,
    texture_height
  );
  return stream;

fail:
  if (fp != NULL) {
    fclose(fp);
  }
  free(texture_data);
  free(initial_normals);
  free(maggie_vertices);
  free(indices);
  free(stream_vertices);
  SpyMaggieStreamFree(stream);
  return NULL;
}

void SpyMaggieStreamUpdate(SpyMaggieStream *stream, ULONG time_ticks, ULONG ticks_per_frame)
{
  ULONG playable_frames;
  ULONG local_ticks;
  ULONG frame_index;
  ULONG vertex_index;
  vec3 *frame_positions;

  if (!SpyMaggieStreamIsReady(stream)) {
    return;
  }

  if (ticks_per_frame == 0) {
    ticks_per_frame = stream->time_per_frame;
  }

  playable_frames = stream->frame_count - 1;
  if (stream->active_ticks_per_frame == 0) {
    stream->active_ticks_per_frame = ticks_per_frame;
    stream->timing_origin_ticks = time_ticks;
  } else if (stream->active_ticks_per_frame != ticks_per_frame) {
    stream->active_ticks_per_frame = ticks_per_frame;
    stream->timing_origin_ticks = time_ticks - (stream->last_uploaded_frame * ticks_per_frame);
  }

  local_ticks = time_ticks - stream->timing_origin_ticks;
  frame_index = (local_ticks / ticks_per_frame) % playable_frames;
  if (stream->frame_uploaded && frame_index == stream->last_uploaded_frame) {
    return;
  }

  frame_positions = stream->frames[frame_index].positions;

  for (vertex_index = 0; vertex_index < stream->vertex_count; vertex_index++) {
    stream->scratch_positions[vertex_index].x =
      stream->base_positions[vertex_index].x +
      frame_positions[vertex_index].x;
    stream->scratch_positions[vertex_index].y =
      stream->base_positions[vertex_index].y +
      frame_positions[vertex_index].y;
    stream->scratch_positions[vertex_index].z =
      stream->base_positions[vertex_index].z +
      frame_positions[vertex_index].z;
  }

  magUploadVertexPositions(stream->vertex_buffer, stream->scratch_positions, 0, stream->vertex_count);
  stream->last_uploaded_frame = frame_index;
  stream->frame_uploaded = TRUE;
}

void SpyMaggieStreamDraw(const SpyMaggieStream *stream)
{
  if (!SpyMaggieStreamIsReady(stream) || !stream->frame_uploaded) {
    return;
  }

  magSetTexture(0, stream->texture);
  magSetVertexBuffer(stream->vertex_buffer);
  magSetIndexBuffer(stream->index_buffer);
  magDrawIndexedTriangles(0, stream->vertex_count, 0, stream->index_count);
}

void SpyMaggieStreamFree(SpyMaggieStream *stream)
{
  ULONG frame_index;

  if (stream == NULL) {
    return;
  }

  if (stream->frames != NULL) {
    for (frame_index = 0; frame_index < stream->frame_count; frame_index++) {
      free(stream->frames[frame_index].positions);
    }
    free(stream->frames);
  }

  free(stream->base_positions);
  free(stream->scratch_positions);

  if (MaggieBase != NULL) {
    if (stream->vertex_buffer != SPY_MAGGIE_STREAM_INVALID_HANDLE) {
      magFreeVertexBuffer(stream->vertex_buffer);
    }
    if (stream->index_buffer != SPY_MAGGIE_STREAM_INVALID_HANDLE) {
      magFreeIndexBuffer(stream->index_buffer);
    }
    if (stream->texture != SPY_MAGGIE_STREAM_INVALID_HANDLE) {
      magFreeTexture(stream->texture);
    }
  }

  free(stream);
}
