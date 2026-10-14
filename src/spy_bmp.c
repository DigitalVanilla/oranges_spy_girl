#include <dos/dos.h>
#include <exec/types.h>
#include <string.h>

#include <proto/dos.h>

#include <sage/sage_bitmap.h>
#include <sage/sage_error.h>
#include <sage/sage_memory.h>
#include <sage/sage_picture.h>

#include "spy_bmp.h"
#include "spy_log.h"

#define SPY_BMP_FILE_HEADER_SIZE 14UL
#define SPY_BMP_MIN_DIB_SIZE     40UL
#define SPY_BMP_MAX_DIB_SIZE     124UL
#define SPY_BMP_BI_RGB           0UL
#define SPY_BMP_BI_BITFIELDS     3UL
#define SPY_BMP_RGB565_RED_MASK  0x0000f800UL
#define SPY_BMP_RGB565_GREEN_MASK 0x000007e0UL
#define SPY_BMP_RGB565_BLUE_MASK 0x0000001fUL

static UWORD SpyBmpReadLe16(const UBYTE *data)
{
  return (UWORD)((UWORD)data[0] | ((UWORD)data[1] << 8));
}

static ULONG SpyBmpReadLe32(const UBYTE *data)
{
  return
    (ULONG)data[0] |
    ((ULONG)data[1] << 8) |
    ((ULONG)data[2] << 16) |
    ((ULONG)data[3] << 24);
}

static BOOL SpyBmpReadExact(BPTR file, APTR buffer, ULONG size)
{
  return Read(file, buffer, (LONG)size) == (LONG)size;
}

static BOOL SpyBmpSeek(BPTR file, ULONG offset)
{
  return Seek(file, (LONG)offset, OFFSET_BEGINNING) != -1;
}

SAGE_Picture *SpyBmpLoadPicture(const char *filename)
{
  UBYTE file_header[SPY_BMP_FILE_HEADER_SIZE];
  UBYTE dib[SPY_BMP_MAX_DIB_SIZE];
  UBYTE palette_entry[4];
  UBYTE *row_buffer = NULL;
  SAGE_Picture *picture = NULL;
  BPTR file = 0;
  const char *failure = "invalid BMP";
  ULONG dib_size;
  ULONG pixel_offset;
  LONG signed_width;
  LONG signed_height;
  ULONG width;
  ULONG height;
  ULONG depth;
  ULONG compression;
  ULONG colors_used;
  ULONG palette_count;
  ULONG row_stride;
  ULONG row;
  ULONG column;
  ULONG destination_row;
  ULONG red_mask;
  ULONG green_mask;
  ULONG blue_mask;
  ULONG required_offset;
  ULONG pixel_format;
  BOOL top_down;

  if (filename == NULL) {
    SAGE_SetError(SERR_NULL_POINTER);
    return NULL;
  }

  file = Open((STRPTR)filename, MODE_OLDFILE);
  if (file == 0) {
    SAGE_SetError(SERR_OPENFILE);
    SpyLog("BMP load failed: cannot open %s\n", filename);
    return NULL;
  }

  if (!SpyBmpReadExact(file, file_header, sizeof(file_header))) {
    SAGE_SetError(SERR_READFILE);
    failure = "short file header";
    goto fail;
  }
  if (file_header[0] != 'B' || file_header[1] != 'M') {
    SAGE_SetError(SERR_FILEFORMAT);
    failure = "missing BM signature";
    goto fail;
  }

  pixel_offset = SpyBmpReadLe32(file_header + 10);
  if (!SpyBmpReadExact(file, dib, 4)) {
    SAGE_SetError(SERR_READFILE);
    failure = "missing DIB header";
    goto fail;
  }

  dib_size = SpyBmpReadLe32(dib);
  if (dib_size < SPY_BMP_MIN_DIB_SIZE || dib_size > SPY_BMP_MAX_DIB_SIZE) {
    SAGE_SetError(SERR_FILEFORMAT);
    failure = "unsupported DIB header";
    goto fail;
  }
  if (!SpyBmpReadExact(file, dib + 4, dib_size - 4)) {
    SAGE_SetError(SERR_READFILE);
    failure = "short DIB header";
    goto fail;
  }

  signed_width = (LONG)SpyBmpReadLe32(dib + 4);
  signed_height = (LONG)SpyBmpReadLe32(dib + 8);
  depth = (ULONG)SpyBmpReadLe16(dib + 14);
  compression = SpyBmpReadLe32(dib + 16);
  colors_used = SpyBmpReadLe32(dib + 32);

  if (
    signed_width <= 0 || signed_width > 65535L ||
    signed_height == 0 || signed_height == (LONG)0x80000000UL ||
    signed_height > 65535L || signed_height < -65535L ||
    SpyBmpReadLe16(dib + 12) != 1
  ) {
    SAGE_SetError(SERR_FILEFORMAT);
    failure = "invalid dimensions or plane count";
    goto fail;
  }

  width = (ULONG)signed_width;
  top_down = signed_height < 0;
  height = (ULONG)(top_down ? -signed_height : signed_height);

  if (depth == SBMP_DEPTH8 && compression == SPY_BMP_BI_RGB) {
    palette_count = colors_used == 0 ? 256UL : colors_used;
    if (palette_count > 256UL) {
      SAGE_SetError(SERR_FILEFORMAT);
      failure = "invalid 8-bit palette size";
      goto fail;
    }
    required_offset = SPY_BMP_FILE_HEADER_SIZE + dib_size + (palette_count * 4UL);
    if (pixel_offset < required_offset) {
      SAGE_SetError(SERR_FILEFORMAT);
      failure = "palette overlaps pixel data";
      goto fail;
    }

    pixel_format = PIXFMT_CLUT;
  } else if (depth == SBMP_DEPTH16 && compression == SPY_BMP_BI_BITFIELDS) {
    if (dib_size == SPY_BMP_MIN_DIB_SIZE) {
      required_offset = SPY_BMP_FILE_HEADER_SIZE + dib_size + 12UL;
      if (pixel_offset < required_offset || !SpyBmpSeek(file, SPY_BMP_FILE_HEADER_SIZE + dib_size)) {
        SAGE_SetError(SERR_FILEFORMAT);
        failure = "missing RGB565 masks";
        goto fail;
      }
      if (!SpyBmpReadExact(file, palette_entry, sizeof(palette_entry))) {
        SAGE_SetError(SERR_READFILE);
        failure = "short red mask";
        goto fail;
      }
      red_mask = SpyBmpReadLe32(palette_entry);
      if (!SpyBmpReadExact(file, palette_entry, sizeof(palette_entry))) {
        SAGE_SetError(SERR_READFILE);
        failure = "short green mask";
        goto fail;
      }
      green_mask = SpyBmpReadLe32(palette_entry);
      if (!SpyBmpReadExact(file, palette_entry, sizeof(palette_entry))) {
        SAGE_SetError(SERR_READFILE);
        failure = "short blue mask";
        goto fail;
      }
      blue_mask = SpyBmpReadLe32(palette_entry);
    } else if (dib_size >= 52UL) {
      red_mask = SpyBmpReadLe32(dib + 40);
      green_mask = SpyBmpReadLe32(dib + 44);
      blue_mask = SpyBmpReadLe32(dib + 48);
    } else {
      SAGE_SetError(SERR_FILEFORMAT);
      failure = "unsupported RGB565 DIB header";
      goto fail;
    }

    if (
      red_mask != SPY_BMP_RGB565_RED_MASK ||
      green_mask != SPY_BMP_RGB565_GREEN_MASK ||
      blue_mask != SPY_BMP_RGB565_BLUE_MASK
    ) {
      SAGE_SetError(SERR_PIXELFORMAT);
      failure = "16-bit BMP is not RGB565";
      goto fail;
    }
    pixel_format = PIXFMT_RGB16;
    palette_count = 0;
  } else {
    SAGE_SetError(SERR_PIXELFORMAT);
    failure = "only indexed 8-bit and RGB565 BMP are supported";
    goto fail;
  }

  picture = SAGE_AllocPicture();
  if (picture == NULL) {
    failure = "cannot allocate picture";
    goto fail;
  }

  if (depth == SBMP_DEPTH8) {
    if (!SpyBmpSeek(file, SPY_BMP_FILE_HEADER_SIZE + dib_size)) {
      SAGE_SetError(SERR_READFILE);
      failure = "cannot seek to palette";
      goto fail;
    }
    for (column = 0; column < palette_count; column++) {
      if (!SpyBmpReadExact(file, palette_entry, sizeof(palette_entry))) {
        SAGE_SetError(SERR_READFILE);
        failure = "short palette";
        goto fail;
      }
      picture->color_map[column] =
        ((ULONG)palette_entry[2] << 16) |
        ((ULONG)palette_entry[1] << 8) |
        (ULONG)palette_entry[0];
    }
  }

  picture->bitmap = SAGE_AllocBitmap(width, height, depth, 0, pixel_format, NULL);
  if (picture->bitmap == NULL) {
    failure = "cannot allocate bitmap";
    goto fail;
  }

  row_stride = ((width * depth + 31UL) / 32UL) * 4UL;
  row_buffer = (UBYTE *)SAGE_AllocMem(row_stride);
  if (row_buffer == NULL) {
    SAGE_SetError(SERR_NO_MEMORY);
    failure = "cannot allocate row buffer";
    goto fail;
  }
  if (!SpyBmpSeek(file, pixel_offset)) {
    SAGE_SetError(SERR_READFILE);
    failure = "cannot seek to pixel data";
    goto fail;
  }

  for (row = 0; row < height; row++) {
    UBYTE *destination;

    if (!SpyBmpReadExact(file, row_buffer, row_stride)) {
      SAGE_SetError(SERR_READFILE);
      failure = "short pixel data";
      goto fail;
    }

    destination_row = top_down ? row : (height - row - 1UL);
    destination = (UBYTE *)picture->bitmap->bitmap_buffer +
      (destination_row * picture->bitmap->bpr);

    if (depth == SBMP_DEPTH8) {
      memcpy(destination, row_buffer, width);
    } else {
      UWORD *pixels = (UWORD *)destination;
      for (column = 0; column < width; column++) {
        pixels[column] = SpyBmpReadLe16(row_buffer + (column * 2UL));
      }
    }
  }

  SAGE_FreeMem(row_buffer);
  row_buffer = NULL;
  Close(file);
  file = 0;

  if (!SAGE_RemapPicture(picture)) {
    failure = "cannot remap to screen format";
    goto fail;
  }
  picture->bitmap->bpr = picture->bitmap->width * (picture->bitmap->depth / 8UL);

  return picture;

fail:
  SpyLog("BMP load failed: %s (%s)\n", filename, failure);
  if (row_buffer != NULL) {
    SAGE_FreeMem(row_buffer);
  }
  if (file != 0) {
    Close(file);
  }
  if (picture != NULL) {
    SAGE_ReleasePicture(picture);
  }
  return NULL;
}
