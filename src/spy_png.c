#include <dos/dos.h>
#include <exec/types.h>
#include <string.h>

#include <proto/dos.h>

#include <sage/sage_bitmap.h>
#include <sage/sage_error.h>
#include <sage/sage_memory.h>
#include <sage/sage_picture.h>

#include "spy_log.h"
#include "spy_png.h"
#include "third_party/puff.h"

#define SPY_PNG_SIGNATURE_SIZE 8UL
#define SPY_PNG_IHDR_SIZE      13UL
#define SPY_PNG_PALETTE_SIZE   768UL
#define SPY_PNG_MAX_DIMENSION  65535UL
#define SPY_PNG_ADLER_BASE     65521UL
#define SPY_PNG_ADLER_BLOCK    5552UL

typedef struct {
  ULONG width;
  ULONG height;
  ULONG idat_size;
  UWORD palette_entries;
  UBYTE palette[SPY_PNG_PALETTE_SIZE];
  UBYTE transparent_index;
  BOOL has_transparency;
  BOOL has_header;
  BOOL has_palette;
  BOOL has_idat;
  BOOL has_end;
} SpyPngInfo;

static ULONG SpyPngReadBe32(const UBYTE *data)
{
  return
    ((ULONG)data[0] << 24) |
    ((ULONG)data[1] << 16) |
    ((ULONG)data[2] << 8) |
    (ULONG)data[3];
}

static BOOL SpyPngReadExact(BPTR file, APTR buffer, ULONG size)
{
  return size <= 0x7fffffffUL && Read(file, buffer, (LONG)size) == (LONG)size;
}

static BOOL SpyPngSkip(BPTR file, ULONG size)
{
  if (size == 0) {
    return TRUE;
  }
  return size <= 0x7fffffffUL && Seek(file, (LONG)size, OFFSET_CURRENT) != -1;
}

static BOOL SpyPngSeek(BPTR file, ULONG offset)
{
  return offset <= 0x7fffffffUL && Seek(file, (LONG)offset, OFFSET_BEGINNING) != -1;
}

static BOOL SpyPngTypeEquals(const UBYTE type[4], const char *name)
{
  return
    type[0] == (UBYTE)name[0] &&
    type[1] == (UBYTE)name[1] &&
    type[2] == (UBYTE)name[2] &&
    type[3] == (UBYTE)name[3];
}

static BOOL SpyPngReadChunkHeader(BPTR file, ULONG *length, UBYTE type[4])
{
  UBYTE header[8];

  if (!SpyPngReadExact(file, header, sizeof(header))) {
    return FALSE;
  }

  *length = SpyPngReadBe32(header);
  memcpy(type, header + 4, 4);
  return TRUE;
}

static BOOL SpyPngReadMetadata(BPTR file, SpyPngInfo *info, const char **failure)
{
  static const UBYTE signature[SPY_PNG_SIGNATURE_SIZE] = {
    0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a
  };
  UBYTE file_signature[SPY_PNG_SIGNATURE_SIZE];
  UBYTE type[4];
  UBYTE header[SPY_PNG_IHDR_SIZE];
  UBYTE alpha[SPIC_MAXCOLORS];
  UBYTE crc[4];
  ULONG length;
  ULONG index;
  UWORD transparent_count;
  BOOL first_chunk = TRUE;

  memset(info, 0, sizeof(*info));
  memset(alpha, 255, sizeof(alpha));

  if (!SpyPngSeek(file, 0) || !SpyPngReadExact(file, file_signature, sizeof(file_signature))) {
    *failure = "short PNG signature";
    return FALSE;
  }
  if (memcmp(file_signature, signature, sizeof(signature)) != 0) {
    *failure = "invalid PNG signature";
    return FALSE;
  }

  while (!info->has_end) {
    if (!SpyPngReadChunkHeader(file, &length, type)) {
      *failure = "short chunk header";
      return FALSE;
    }

    if (SpyPngTypeEquals(type, "IHDR")) {
      if (!first_chunk || info->has_header || length != SPY_PNG_IHDR_SIZE) {
        *failure = "invalid IHDR chunk";
        return FALSE;
      }
      if (!SpyPngReadExact(file, header, sizeof(header))) {
        *failure = "short IHDR chunk";
        return FALSE;
      }

      info->width = SpyPngReadBe32(header);
      info->height = SpyPngReadBe32(header + 4);
      if (
        info->width == 0 || info->width > SPY_PNG_MAX_DIMENSION ||
        info->height == 0 || info->height > SPY_PNG_MAX_DIMENSION
      ) {
        *failure = "invalid PNG dimensions";
        return FALSE;
      }
      if (
        header[8] != 8 || header[9] != 3 ||
        header[10] != 0 || header[11] != 0 || header[12] != 0
      ) {
        *failure = "only non-interlaced indexed 8-bit PNG is supported";
        return FALSE;
      }
      info->has_header = TRUE;
    } else if (SpyPngTypeEquals(type, "PLTE")) {
      if (
        !info->has_header || info->has_palette || info->has_idat ||
        length == 0 || length > SPY_PNG_PALETTE_SIZE || (length % 3UL) != 0
      ) {
        *failure = "invalid PLTE chunk";
        return FALSE;
      }
      if (!SpyPngReadExact(file, info->palette, length)) {
        *failure = "short PLTE chunk";
        return FALSE;
      }
      info->palette_entries = (UWORD)(length / 3UL);
      info->has_palette = TRUE;
    } else if (SpyPngTypeEquals(type, "tRNS")) {
      if (
        !info->has_palette || info->has_idat ||
        length == 0 || length > info->palette_entries
      ) {
        *failure = "invalid tRNS chunk";
        return FALSE;
      }
      if (!SpyPngReadExact(file, alpha, length)) {
        *failure = "short tRNS chunk";
        return FALSE;
      }

      transparent_count = 0;
      for (index = 0; index < length; index++) {
        if (alpha[index] == 0) {
          info->transparent_index = (UBYTE)index;
          transparent_count++;
        } else if (alpha[index] != 255) {
          *failure = "partial PNG alpha is not supported";
          return FALSE;
        }
      }
      if (transparent_count > 1) {
        *failure = "multiple transparent palette entries are not supported";
        return FALSE;
      }
      info->has_transparency = transparent_count == 1;
    } else if (SpyPngTypeEquals(type, "IDAT")) {
      if (!info->has_header || !info->has_palette || length == 0) {
        *failure = "invalid IDAT chunk";
        return FALSE;
      }
      if (info->idat_size > 0xffffffffUL - length) {
        *failure = "PNG data is too large";
        return FALSE;
      }
      info->idat_size += length;
      info->has_idat = TRUE;
      if (!SpyPngSkip(file, length)) {
        *failure = "short IDAT chunk";
        return FALSE;
      }
    } else if (SpyPngTypeEquals(type, "IEND")) {
      if (length != 0 || !info->has_idat) {
        *failure = "invalid IEND chunk";
        return FALSE;
      }
      info->has_end = TRUE;
    } else {
      if ((type[0] & 0x20) == 0) {
        *failure = "unsupported critical PNG chunk";
        return FALSE;
      }
      if (!SpyPngSkip(file, length)) {
        *failure = "short ancillary PNG chunk";
        return FALSE;
      }
    }

    if (!SpyPngReadExact(file, crc, sizeof(crc))) {
      *failure = "missing PNG chunk CRC";
      return FALSE;
    }
    first_chunk = FALSE;
  }

  if (!info->has_header || !info->has_palette || !info->has_idat || info->idat_size < 6) {
    *failure = "incomplete PNG";
    return FALSE;
  }
  return TRUE;
}

static BOOL SpyPngReadIdat(BPTR file, UBYTE *compressed, ULONG expected_size, const char **failure)
{
  UBYTE type[4];
  UBYTE crc[4];
  ULONG length;
  ULONG offset = 0;

  if (!SpyPngSeek(file, SPY_PNG_SIGNATURE_SIZE)) {
    *failure = "cannot seek to PNG chunks";
    return FALSE;
  }

  for (;;) {
    if (!SpyPngReadChunkHeader(file, &length, type)) {
      *failure = "short chunk header on second pass";
      return FALSE;
    }

    if (SpyPngTypeEquals(type, "IDAT")) {
      if (
        offset > expected_size || length > expected_size - offset ||
        !SpyPngReadExact(file, compressed + offset, length)
      ) {
        *failure = "short IDAT data";
        return FALSE;
      }
      offset += length;
    } else if (!SpyPngSkip(file, length)) {
      *failure = "short PNG chunk on second pass";
      return FALSE;
    }

    if (!SpyPngReadExact(file, crc, sizeof(crc))) {
      *failure = "missing PNG chunk CRC on second pass";
      return FALSE;
    }
    if (SpyPngTypeEquals(type, "IEND")) {
      break;
    }
  }

  if (offset != expected_size) {
    *failure = "incomplete IDAT data";
    return FALSE;
  }
  return TRUE;
}

static ULONG SpyPngAdler32(const UBYTE *data, ULONG size)
{
  ULONG first = 1;
  ULONG second = 0;

  while (size > 0) {
    ULONG block = size > SPY_PNG_ADLER_BLOCK ? SPY_PNG_ADLER_BLOCK : size;
    ULONG remaining = block;

    size -= block;
    while (remaining-- > 0) {
      first += *data++;
      second += first;
    }
    first %= SPY_PNG_ADLER_BASE;
    second %= SPY_PNG_ADLER_BASE;
  }

  return (second << 16) | first;
}

static BOOL SpyPngInflate(
  const UBYTE *compressed,
  ULONG compressed_size,
  UBYTE *filtered,
  ULONG filtered_size,
  const char **failure
)
{
  ULONG zlib_header;
  ULONG expected_adler;
  ULONG output_size = filtered_size;
  ULONG input_size;
  int inflate_result;

  if (compressed_size < 6) {
    *failure = "short zlib stream";
    return FALSE;
  }

  zlib_header = ((ULONG)compressed[0] << 8) | compressed[1];
  if (
    (compressed[0] & 0x0f) != 8 || (compressed[0] >> 4) > 7 ||
    (zlib_header % 31UL) != 0 || (compressed[1] & 0x20) != 0
  ) {
    *failure = "unsupported zlib header";
    return FALSE;
  }

  input_size = compressed_size - 6UL;
  inflate_result = puff(
    filtered,
    (unsigned long *)&output_size,
    compressed + 2,
    (unsigned long *)&input_size
  );
  if (inflate_result != 0 || output_size != filtered_size) {
    *failure = "invalid or incomplete DEFLATE data";
    return FALSE;
  }

  expected_adler = SpyPngReadBe32(compressed + compressed_size - 4UL);
  if (SpyPngAdler32(filtered, filtered_size) != expected_adler) {
    *failure = "PNG Adler-32 mismatch";
    return FALSE;
  }
  return TRUE;
}

static UBYTE SpyPngPaeth(UBYTE left, UBYTE up, UBYTE upper_left)
{
  LONG prediction = (LONG)left + (LONG)up - (LONG)upper_left;
  LONG left_distance = prediction - (LONG)left;
  LONG up_distance = prediction - (LONG)up;
  LONG upper_left_distance = prediction - (LONG)upper_left;

  if (left_distance < 0) {
    left_distance = -left_distance;
  }
  if (up_distance < 0) {
    up_distance = -up_distance;
  }
  if (upper_left_distance < 0) {
    upper_left_distance = -upper_left_distance;
  }

  if (left_distance <= up_distance && left_distance <= upper_left_distance) {
    return left;
  }
  if (up_distance <= upper_left_distance) {
    return up;
  }
  return upper_left;
}

static BOOL SpyPngUnfilter(
  const UBYTE *filtered,
  SAGE_Bitmap *bitmap,
  const char **failure
)
{
  ULONG row;
  ULONG column;
  ULONG width = bitmap->width;
  UBYTE *pixels = (UBYTE *)bitmap->bitmap_buffer;

  for (row = 0; row < bitmap->height; row++) {
    const UBYTE *source = filtered + (row * (width + 1UL));
    UBYTE filter = *source++;
    UBYTE *destination = pixels + (row * bitmap->bpr);
    const UBYTE *previous = row == 0 ? NULL : pixels + ((row - 1UL) * bitmap->bpr);

    if (filter > 4) {
      *failure = "unsupported PNG row filter";
      return FALSE;
    }

    for (column = 0; column < width; column++) {
      UBYTE left = column == 0 ? 0 : destination[column - 1UL];
      UBYTE up = previous == NULL ? 0 : previous[column];
      UBYTE upper_left = previous == NULL || column == 0 ? 0 : previous[column - 1UL];
      ULONG predictor = 0;

      switch (filter) {
        case 1:
          predictor = left;
          break;
        case 2:
          predictor = up;
          break;
        case 3:
          predictor = ((ULONG)left + (ULONG)up) >> 1;
          break;
        case 4:
          predictor = SpyPngPaeth(left, up, upper_left);
          break;
      }
      destination[column] = (UBYTE)((ULONG)source[column] + predictor);
    }
  }
  return TRUE;
}

SAGE_Picture *SpyPngLoadPicture(const char *filename)
{
  SpyPngInfo info;
  SAGE_Picture *picture = NULL;
  BPTR file = 0;
  UBYTE *compressed = NULL;
  UBYTE *filtered = NULL;
  const char *failure = "invalid PNG";
  ULONG filtered_size;
  ULONG index;

  if (filename == NULL) {
    SAGE_SetError(SERR_NULL_POINTER);
    return NULL;
  }

  file = Open((STRPTR)filename, MODE_OLDFILE);
  if (file == 0) {
    SAGE_SetError(SERR_OPENFILE);
    SpyLog("PNG load failed: cannot open %s\n", filename);
    return NULL;
  }

  if (!SpyPngReadMetadata(file, &info, &failure)) {
    SAGE_SetError(SERR_FILEFORMAT);
    goto fail;
  }
  if (info.width + 1UL > 0xffffffffUL / info.height) {
    SAGE_SetError(SERR_PICTURE_SIZE);
    failure = "PNG dimensions overflow the decode buffer";
    goto fail;
  }
  filtered_size = (info.width + 1UL) * info.height;

  compressed = (UBYTE *)SAGE_AllocMem(info.idat_size);
  filtered = (UBYTE *)SAGE_AllocMem(filtered_size);
  if (compressed == NULL || filtered == NULL) {
    SAGE_SetError(SERR_NO_MEMORY);
    failure = "not enough memory for PNG decode buffers";
    goto fail;
  }

  if (!SpyPngReadIdat(file, compressed, info.idat_size, &failure)) {
    SAGE_SetError(SERR_READFILE);
    goto fail;
  }
  if (!SpyPngInflate(compressed, info.idat_size, filtered, filtered_size, &failure)) {
    SAGE_SetError(SERR_FILEFORMAT);
    goto fail;
  }

  picture = SAGE_AllocPicture();
  if (picture == NULL) {
    failure = "cannot allocate PNG picture";
    goto fail;
  }
  memset(picture->color_map, 0, sizeof(picture->color_map));
  for (index = 0; index < info.palette_entries; index++) {
    picture->color_map[index] =
      ((ULONG)info.palette[index * 3UL] << 16) |
      ((ULONG)info.palette[index * 3UL + 1UL] << 8) |
      (ULONG)info.palette[index * 3UL + 2UL];
  }

  picture->bitmap = SAGE_AllocBitmap(
    info.width,
    info.height,
    SBMP_DEPTH8,
    0,
    PIXFMT_CLUT,
    NULL
  );
  if (picture->bitmap == NULL) {
    failure = "cannot allocate PNG bitmap";
    goto fail;
  }
  if (!SpyPngUnfilter(filtered, picture->bitmap, &failure)) {
    SAGE_SetError(SERR_FILEFORMAT);
    goto fail;
  }
  if (info.has_transparency) {
    SAGE_SetBitmapTransparency(picture->bitmap, info.transparent_index);
  }

  SAGE_FreeMem(filtered);
  SAGE_FreeMem(compressed);
  Close(file);
  return picture;

fail:
  SpyLog("PNG load failed: %s (%s)\n", filename, failure);
  if (filtered != NULL) {
    SAGE_FreeMem(filtered);
  }
  if (compressed != NULL) {
    SAGE_FreeMem(compressed);
  }
  if (file != 0) {
    Close(file);
  }
  if (picture != NULL) {
    SAGE_ReleasePicture(picture);
  }
  return NULL;
}
