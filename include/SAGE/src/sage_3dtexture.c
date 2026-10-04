/**
 * sage_3dtexture.h
 * 
 * SAGE (Simple Amiga Game Engine) project
 * 3D texture management
 * 
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 25.1 February 2025 (updated: 26/02/2025)
 */

#include <exec/types.h>
#include <dos/dos.h>
#include <utility/tagitem.h>

#include <proto/dos.h>
#include <proto/Warp3D.h>
#include <proto/Maggie3D.h>

#include <sage/sage_error.h>
#include <sage/sage_logger.h>
#include <sage/sage_memory.h>
#include <sage/sage_context.h>
#include <sage/sage_bitmap.h>
#include <sage/sage_3dtexture.h>

#include <sage/sage_debug.h>

/** SAGE context */
extern SAGE_Context SageContext;

/**
 * Convert a SAGE texture format into a Maggie3D texture format.
 */
UWORD SAGE_GetMaggieTextureFormat(ULONG texformat)
{
  switch (texformat) {
    case STEX_PIXFMT_CLUT:
      return M3D_PIXFMT_CLUT;
    case STEX_PIXFMT_RGB16:
      return M3D_PIXFMT_RGB16;
    case STEX_PIXFMT_RGB24:
      return M3D_PIXFMT_RGB24;
    case STEX_PIXFMT_ARGB32:
      return M3D_PIXFMT_ARGB32;
  }
  return M3D_PIXFMT_UNKNOWN;
}

/**
 * Check if texture size is valid
 */
BOOL SAGE_CheckTextureSize(ULONG width, ULONG height)
{
  if (width != height) {
    return FALSE;
  }
  if (width != STEX_SIZE64 && width != STEX_SIZE128 && width != STEX_SIZE256 && width != STEX_SIZE512) {
    return FALSE;
  }
  return TRUE;
}

/**
 * Create a texture from a file
 *
 * @param index     Texture index
 * @param file_name File name
 *
 * @return Operation success
 */
BOOL SAGE_CreateTextureFromFile(UWORD index, STRPTR file_name)
{
  SAGE_Picture *picture;

  SD(SAGE_DebugLog("Create texture #%d from file %s", index, file_name);)
  if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return FALSE;
  }
  // Don't remap picture when using Warp3D or Maggie3D
  if (SageContext.Sage3D->render_system != S3DD_S3DRENDER) {
    SD(SAGE_DebugLog("Disable picture remapping");)
    SAGE_AutoRemapPicture(FALSE);
  }
  if ((picture = SAGE_LoadPicture(file_name)) != NULL) {
    // Create a new texture
    if (SAGE_CreateTextureFromPicture(index, 0, 0, STEX_FULLSIZE, picture)) {
      SAGE_ReleasePicture(picture);
      SAGE_AutoRemapPicture(TRUE);
      return TRUE;
    }
    SAGE_ReleasePicture(picture);
  }
  SAGE_AutoRemapPicture(TRUE);
  return FALSE;
}

/**
 * Create a texture from a picture
 *
 * @param index   Texture index
 * @param left    Left position of texture in picture
 * @param top     Top position of texture in picture
 * @param size    Texture size
 * @param picture SAGE Picture pointer
 *
 * @return Operation success
 */
BOOL SAGE_CreateTextureFromPicture(UWORD index, UWORD left, UWORD top, UWORD size, SAGE_Picture * picture)
{
  SAGE_Screen *screen;
  SAGE_3DTexture *texture;
  UWORD idxcol;

  SD(SAGE_DebugLog("Create texture #%d (%d,%d)x%d", index, left, top, size);)
  // Check for video device
  if (SageContext.SageVideo == NULL) {
    SAGE_SetError(SERR_NO_VIDEODEVICE);
    return FALSE;
  }
  screen = SageContext.SageVideo->screen;
  if (screen == NULL) {
    SAGE_SetError(SERR_NO_SCREEN);
    return FALSE;
  }
  if (index >= STEX_MAX_TEXTURES) {
    SAGE_SetError(SERR_TEX_INDEX);
    return FALSE;
  }
  if (picture == NULL || picture->bitmap == NULL) {
    SAGE_SetError(SERR_NULL_POINTER);
    return FALSE;
  }
  // Check for size compliance
  if (size == STEX_FULLSIZE) {
    if (!SAGE_CheckTextureSize(picture->bitmap->width, picture->bitmap->height)) {
      SAGE_SetError(SERR_TEXTURE_SIZE);
      return FALSE;
    }
    size = picture->bitmap->width;
  }
  if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return FALSE;
  }
  if (SageContext.Sage3D->textures[index] != NULL) {
    SAGE_ReleaseTexture(index);
  }
  // Allocate and init texture
  texture = (SAGE_3DTexture *)SAGE_AllocMem(sizeof(SAGE_3DTexture));
  if (texture != NULL) {
    texture->size = size;
    if ((texture->bitmap = SAGE_AllocBitmap(texture->size, texture->size, picture->bitmap->depth, 0, picture->bitmap->pixformat, NULL)) != NULL) {
      texture->w3dtex = NULL;
      texture->m3dtex = NULL;
      if (SAGE_BlitPictureToBitmap(picture, left, top, texture->size, texture->size, texture->bitmap, 0, 0)) {
        // Set texture format
        switch (picture->bitmap->pixformat) {
          case PIXFMT_CLUT:
            texture->texformat = STEX_PIXFMT_CLUT;
            for (idxcol = 0;idxcol < STEX_MAXCOLORS;idxcol++) {
              texture->palette[idxcol] = picture->color_map[idxcol];
            }
            break;
          case PIXFMT_RGB15:
            texture->texformat = STEX_PIXFMT_RGB15;
            break;
          case PIXFMT_RGB16:
            texture->texformat = STEX_PIXFMT_RGB16;
            break;
          case PIXFMT_RGB24:
            texture->texformat = STEX_PIXFMT_RGB24;
            break;
          case PIXFMT_ARGB32:
            texture->texformat = STEX_PIXFMT_ARGB32;
            break;
          case PIXFMT_RGBA32:
            texture->texformat = STEX_PIXFMT_RGBA32;
            break;
          default:
            texture->texformat = STEX_PIXFMT_UNKNOWN;
        }
        texture->data_size = texture->bitmap->height * texture->bitmap->bpr;
        SageContext.Sage3D->textures[index] = texture;
        SD(SAGE_DebugLog(
            "Texture #%d created 0x%X (%dx%dx%d)",
            index, texture, texture->bitmap->width, texture->bitmap->height, texture->bitmap->depth
        );)
        return TRUE;
      }
    }
    SAGE_FreeMem(texture);
    return FALSE;
  }
  SAGE_SetError(SERR_NO_MEMORY);
  return FALSE;
}

/**
 * Get a texture by his index
 *
 * @param index Texture index
 *
 * @return Texture structure
 */
SAGE_3DTexture *SAGE_GetTexture(WORD index)
{
  // Check for 3d device
  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return NULL;
  })
  SAFE(if (index >= STEX_MAX_TEXTURES) {
    SAGE_SetError(SERR_TEX_INDEX);
    return NULL;
  })
  if (index == STEX_USECOLOR) {
    return NULL;
  }
  return SageContext.Sage3D->textures[index];
}

/**
 * Get the Warp3D texture by his index
 *
 * @param index Texture index
 *
 * @return W3D texture structure
 */
W3D_Texture *SAGE_GetW3DTexture(WORD index)
{
  // Check for 3d device
  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return NULL;
  })
  SAFE(if (index >= STEX_MAX_TEXTURES) {
    SAGE_SetError(SERR_TEX_INDEX);
    return NULL;
  })
  if (index == STEX_USECOLOR) {
    return NULL;
  }
  return SageContext.Sage3D->textures[index]->w3dtex;
}

/**
 * Get the Maggie3D texture by his index
 *
 * @param index Texture index
 *
 * @return W3D texture structure
 */
M3D_Texture *SAGE_GetM3DTexture(WORD index)
{
  // Check for 3d device
  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return NULL;
  })
  SAFE(if (index >= STEX_MAX_TEXTURES) {
    SAGE_SetError(SERR_TEX_INDEX);
    return NULL;
  })
  if (index == STEX_USECOLOR) {
    return NULL;
  }
  return SageContext.Sage3D->textures[index]->m3dtex;
}

/**
 * Return the first free slot in texture array
 *
 * @return Free texture index or -1 when no slot is available
 */
WORD SAGE_GetFreeTextureIndex()
{
  WORD index;
  
  // Check for 3d device
  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return NULL;
  })
  for (index = 0;index < STEX_MAX_TEXTURES;index++) {
    if (SageContext.Sage3D->textures[index] == NULL) {
      return index;
    }
  }
  return -1;
}

/**
 * Get a texture size by his index
 *
 * @param index Texture index
 *
 * @return Texture size
 */
UWORD SAGE_GetTextureSize(UWORD index)
{
  // Check for 3d device
  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return 0;
  })
  SAFE(if (index >= STEX_MAX_TEXTURES) {
    SAGE_SetError(SERR_TEX_INDEX);
    return 0;
  })
  return SageContext.Sage3D->textures[index]->size;
}

/**
 * Get texture buffer by his index
 *
 * @param index Texture index
 *
 * @return Texture buffer address
 */
APTR SAGE_GetTextureBuffer(UWORD index)
{
  // Check for 3d device
  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return NULL;
  })
  SAFE(if (index >= STEX_MAX_TEXTURES) {
    SAGE_SetError(SERR_TEX_INDEX);
    return NULL;
  })
  return SageContext.Sage3D->textures[index]->bitmap->bitmap_buffer;
}

/**
 * Release a texture
 *
 * @param index Texture index
 * 
 * @return Operation success
 */
BOOL SAGE_ReleaseTexture(UWORD index)
{
  SAGE_3DTexture *texture;

  texture = SAGE_GetTexture(index);
  if (texture != NULL) {
    SAGE_RemoveTexture(index);
    SAGE_ReleaseBitmap(texture->bitmap);
    SAGE_FreeMem(texture);
    SageContext.Sage3D->textures[index] = NULL;
    return TRUE;
  }
  return FALSE;
}

/**
 * Remove all textures from memory
 * 
 * @return Operation success
 */
BOOL SAGE_ClearTextures(VOID)
{
  UWORD index;

  SD(SAGE_DebugLog("Clear textures (%d)", STEX_MAX_TEXTURES);)
  for (index = 0;index < STEX_MAX_TEXTURES;index++) {
    SAGE_ReleaseTexture(index);
  }
  return TRUE;
}

/**
 * Add a texture to the card memory
 *
 * @param index Texture index
 * 
 * @return Operation success
 */
BOOL SAGE_AddTexture(UWORD index)
{
  SAGE_3DDevice *device;
  SAGE_3DTexture *texture;
  UWORD m3d_format;
  BOOL transparent;
  struct TagItem m3d_tags[8];
  
  SD(SAGE_DebugLog("Add texture #%d", index);)
  device = SageContext.Sage3D;
  if (device == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
  }
  texture = SAGE_GetTexture(index);
  if (texture == NULL) {
    return FALSE;
  }
  if (device->render_system == S3DD_W3DRENDER) {
    SAGE_SetError(SERR_WARP3D_LIB);
    return FALSE;
  } else if (device->render_system == S3DD_M3DRENDER) {
    SAFE(if (device->m3d_context == NULL) {
      SAGE_SetError(SERR_NO_3DCONTEXT);
      return FALSE;
    })
    if (texture->m3dtex != NULL) {
      return TRUE;
    }
    m3d_format = SAGE_GetMaggieTextureFormat(texture->texformat);
    if (m3d_format == M3D_PIXFMT_UNKNOWN) {
      SAGE_SetError(SERR_TEX_ALLOC);
      return FALSE;
    }
    transparent = (BOOL)(texture->bitmap->properties & SBMP_TRANSPARENT);
    m3d_tags[0].ti_Tag = M3D_TT_DATA;
    m3d_tags[0].ti_Data = (ULONG)texture->bitmap->bitmap_buffer;
    m3d_tags[1].ti_Tag = M3D_TT_FORMAT;
    m3d_tags[1].ti_Data = (ULONG)m3d_format;
    m3d_tags[2].ti_Tag = M3D_TT_WIDTH;
    m3d_tags[2].ti_Data = texture->bitmap->width;
    m3d_tags[3].ti_Tag = M3D_TT_HEIGHT;
    m3d_tags[3].ti_Data = texture->bitmap->height;
    m3d_tags[4].ti_Tag = M3D_TT_PALETTE;
    m3d_tags[4].ti_Data = (ULONG)texture->palette;
    m3d_tags[5].ti_Tag = M3D_TT_TRANSPARENCY;
    m3d_tags[5].ti_Data = (ULONG)transparent;
    m3d_tags[6].ti_Tag = M3D_TT_TRSCOLOR;
    m3d_tags[6].ti_Data = texture->bitmap->transparency;
    m3d_tags[7].ti_Tag = TAG_END;
    texture->m3dtex = M3D_AllocTextureTagList(device->m3d_context, &(device->maggie3d_error), m3d_tags);
    if (texture->m3dtex == NULL) {
      SAGE_SetError(SERR_TEX_ALLOC);
      return FALSE;
    }
    if (device->render.options & S3DR_BILINEAR) {
      M3D_SetFilter(device->m3d_context, texture->m3dtex, M3D_LINEAR);
    }
  }
  return TRUE;
}

/**
 * Remove a texture from card memory
 *
 * @param index Texture index
 * 
 * @return Operation success
 */
BOOL SAGE_RemoveTexture(UWORD index)
{
  SAGE_3DDevice *device;
  SAGE_3DTexture *texture;
  
  device = SageContext.Sage3D;
  if (device == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
  }
  texture = SAGE_GetTexture(index);
  if (texture == NULL) {
    return FALSE;
  }
  SD(SAGE_DebugLog("Remove texture #%d", index);)
  if (texture->w3dtex != NULL) {
    SD(SAGE_DebugLog(" free W3D texture #%d", index);)
    texture->w3dtex = NULL;
  }
  if (texture->m3dtex != NULL) {
    SD(SAGE_DebugLog(" free M3D texture #%d", index);)
    M3D_FreeTexture(device->m3d_context, texture->m3dtex);
    texture->m3dtex = NULL;
  }
  return TRUE;
}

/**
 * Remove all textures from card memory
 * 
 * @return Operation success
 */
BOOL SAGE_FlushTextures(VOID)
{
  UWORD index;

  SD(SAGE_DebugLog("Flush textures (%d)", STEX_MAX_TEXTURES);)
  for (index = 0;index < STEX_MAX_TEXTURES;index++) {
    SAGE_RemoveTexture(index);
  }
  return TRUE;
}

/**
 * Define the texture transparency color
 *
 * @param index Texture index
 * @param color Transparent color
 *
 * @return Operation success
 */
BOOL SAGE_SetTextureTransparency(UWORD index, ULONG color)
{
  SAGE_3DTexture *texture;

  texture = SAGE_GetTexture(index);
  if (texture == NULL) {
    return FALSE;
  }
  return SAGE_SetBitmapTransparency(texture->bitmap, color);
}
