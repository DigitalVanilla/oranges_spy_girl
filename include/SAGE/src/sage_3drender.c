/**
 * sage_3drender.c
 * 
 * SAGE (Simple Amiga Game Engine) project
 * 3D rendering management
 * 
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 25.1 February 2025 (updated: 10/03/2025)
 */

#include <exec/types.h>

#include <proto/Warp3D.h>
#include <proto/Maggie3D.h>

#include <sage/sage_error.h>
#include <sage/sage_logger.h>
#include <sage/sage_memory.h>
#include <sage/sage_context.h>
#include <sage/sage_blitter.h>
#include <sage/sage_bitmap.h>
#include <sage/sage_screen.h>
#include <sage/sage_draw.h>
#include <sage/sage_3dtexture.h>
#include <sage/sage_3dtexmap.h>
#include <sage/sage_3drender.h>

#include <sage/sage_debug.h>

/** External function for z buffer clear */
extern BOOL ASM SAGE_FastClearZBuffer(
  REG(a0, ULONG source),
  REG(d0, UWORD lines),
  REG(d1, UWORD bytes)
);

/** SAGE context */
extern SAGE_Context SageContext;

VOID SAGE_SetMaggieVertex(M3D_Vertex *vertex, FLOAT x, FLOAT y, FLOAT z, FLOAT u, FLOAT v)
{
  vertex->x = x;
  vertex->y = y;
  vertex->z = z;
  vertex->w = 0.0;
  vertex->u = u;
  vertex->v = v;
  vertex->light = 1.0;
}

VOID SAGE_SetMaggieTriangle(M3D_Triangle *triangle, SAGE_3DElement *element)
{
  SAGE_SetMaggieVertex(&(triangle->v1), element->x1, element->y1, element->z1, element->u1, element->v1);
  SAGE_SetMaggieVertex(&(triangle->v2), element->x2, element->y2, element->z2, element->u2, element->v2);
  SAGE_SetMaggieVertex(&(triangle->v3), element->x3, element->y3, element->z3, element->u3, element->v3);
  triangle->texture = SAGE_GetM3DTexture(element->texture);
  triangle->color = element->color;
}

VOID SAGE_SetMaggieQuad(M3D_Quad *quad, SAGE_3DElement *element)
{
  SAGE_SetMaggieVertex(&(quad->v1), element->x1, element->y1, element->z1, element->u1, element->v1);
  SAGE_SetMaggieVertex(&(quad->v2), element->x2, element->y2, element->z2, element->u2, element->v2);
  SAGE_SetMaggieVertex(&(quad->v3), element->x3, element->y3, element->z3, element->u3, element->v3);
  SAGE_SetMaggieVertex(&(quad->v4), element->x4, element->y4, element->z4, element->u4, element->v4);
  quad->texture = SAGE_GetM3DTexture(element->texture);
  quad->color = element->color;
}

BOOL SAGE_IsMaggieDrawError(LONG error)
{
  return (BOOL)(error != M3D_SUCCESS && error != M3D_NOTRIANGLE && error != M3D_NOQUAD);
}

/** For debug purpose */
extern BOOL engine_debug;

/*****************************************************************************
 *                   START DEBUG
 *****************************************************************************/

#if _SAGE_DEBUG_MODE_ == 1
VOID SAGE_Dump3DElement(SAGE_3DElement *element)
{
  SAGE_DebugLog("Dump SAGE 3D element");
  switch (element->type) {
    case S3DR_ELEM_POINT:
      SAGE_DebugLog("- A point");
      break;
    case S3DR_ELEM_LINE:
      SAGE_DebugLog("- A line");
      break;
    case S3DR_ELEM_TRIANGLE:
      SAGE_DebugLog("- A triangle");
      break;
    case S3DR_ELEM_QUAD:
      SAGE_DebugLog("- A quad");
      break;
  }
  SAGE_DebugLog(" => x1=%f  y1=%f  z1=%f  u1=%f  v1=%f", element->x1, element->y1, element->z1, element->u1, element->v1);
  if (element->type > S3DR_ELEM_POINT) {
    SAGE_DebugLog(" => x2=%f  y2=%f  z2=%f  u2=%f  v2=%f", element->x2, element->y2, element->z2, element->u2, element->v2);
  }
  if (element->type > S3DR_ELEM_LINE) {
    SAGE_DebugLog(" => x3=%f  y3=%f  z3=%f  u3=%f  v3=%f", element->x3, element->y3, element->z3, element->u3, element->v3);
  }
  if (element->type > S3DR_ELEM_TRIANGLE) {
    SAGE_DebugLog(" => x4=%f  y4=%f  z4=%f  u4=%f  v4=%f", element->x4, element->y4, element->z4, element->u4, element->v4);
  }
  SAGE_DebugLog(" => texture=%d  color=0x%08X", element->texture, element->color);
}

VOID SAGE_DumpElementList(SAGE_SortedElement *tlist, UWORD nb_elem)
{
  UWORD index;
  
  SAGE_DebugLog("Dump 3D element list (%d)", nb_elem);
  for (index = 0;index < nb_elem;index++) {
    SAGE_Dump3DElement(tlist[nb_elem].element);
  }
}

VOID SAGE_DumpRenderModeState(VOID)
{
  LONGBITS states;

  SAGE_DebugLog("Render mode state");
  if (SageContext.Sage3D != NULL) {
    states = SageContext.Sage3D->render.options;
    SAGE_DebugLog(" - Z buffer is %s", ((states&S3DR_ZBUFFER) ? "active" : "inactive"));
    SAGE_DebugLog(" - Perspective correction is %s", ((states&S3DR_PERSPECTIVE) ? "active" : "inactive"));
    SAGE_DebugLog(" - Gouraud shading is %s", ((states&S3DR_GOURAUD) ? "active" : "inactive"));
    SAGE_DebugLog(" - Bilinear filtering is %s", ((states&S3DR_BILINEAR) ? "active" : "inactive"));
    SAGE_DebugLog(" - Fogging is %s", ((states&S3DR_FOGGING) ? "active" : "inactive"));
  } else {
    SAGE_DebugLog("3D Device not available !");
  }
}
#endif

/*****************************************************************************
 *                   END DEBUG
 *****************************************************************************/

/**
 * Initialize the 3D renderer
 */
BOOL SAGE_Init3DRender()
{
  if (SageContext.Sage3D != NULL) {
    SageContext.Sage3D->render.render_elements = 0;
    SageContext.Sage3D->render.render_mode = S3DR_RENDER_TEXT;
  }
  return TRUE;
}

/**
 * Allocate the Z buffer
 *
 * @return Operation success
 */
BOOL SAGE_AllocateZBuffer()
{
  SD(SAGE_DebugLog("Allocate Z buffer");)
  SAFE(if (SageContext.SageVideo->screen == NULL) {
    SAGE_SetError(SERR_NO_SCREEN);
    return FALSE;
  })
  if (SageContext.Sage3D->render_system == S3DD_W3DRENDER) {
    SAGE_SetError(SERR_WARP3D_LIB);
    return FALSE;
  } else if (SageContext.Sage3D->render_system == S3DD_M3DRENDER) {
    SAFE(if (SageContext.Sage3D->m3d_context == NULL) {
      SAGE_SetError(SERR_NO_3DCONTEXT);
      return FALSE;
    })
    if (SageContext.Sage3D->m3d_context->zbuffer.data != NULL) {
      return TRUE;
    }
    SageContext.Sage3D->maggie3d_error = M3D_AllocZBuffer(SageContext.Sage3D->m3d_context);
    if (SageContext.Sage3D->maggie3d_error != M3D_SUCCESS) {
      SAGE_SetError(SERR_ZBUFFER);
      return FALSE;
    }
    M3D_SetState(SageContext.Sage3D->m3d_context, M3D_ZBUFFER, M3D_ENABLE);
  } else {
    // If Z-buffer is already allocated just return
    if (SageContext.Sage3D->render.zbuffer.buffer != NULL) {
      return TRUE;
    }
    // Allocate SAGE ZBuffer
    SageContext.Sage3D->render.zbuffer.width = SageContext.SageVideo->screen->width;
    SageContext.Sage3D->render.zbuffer.height = SageContext.SageVideo->screen->height;
    SageContext.Sage3D->render.zbuffer.bpr = SageContext.SageVideo->screen->width * sizeof(UWORD);
    SageContext.Sage3D->render.zbuffer.bpp = sizeof(UWORD);
    SageContext.Sage3D->render.zbuffer.buffer = SAGE_AllocMem(SageContext.Sage3D->render.zbuffer.bpr * SageContext.SageVideo->screen->height);
    if (SageContext.Sage3D->render.zbuffer.buffer == NULL) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Release the Z buffer resources
 */
VOID SAGE_ReleaseZBuffer(VOID)
{
  SD(SAGE_DebugLog("Release Z buffer");)
  if (SageContext.Sage3D->render_system == S3DD_W3DRENDER) {
    SAGE_SetError(SERR_WARP3D_LIB);
  } else if (SageContext.Sage3D->render_system == S3DD_M3DRENDER) {
    if (SageContext.Sage3D->m3d_context != NULL) {
      M3D_SetState(SageContext.Sage3D->m3d_context, M3D_ZBUFFER, M3D_DISABLE);
      M3D_FreeZBuffer(SageContext.Sage3D->m3d_context);
    }
  } else {
    if (SageContext.Sage3D->render.zbuffer.buffer != NULL) {
      SAGE_FreeMem(SageContext.Sage3D->render.zbuffer.buffer);
    }
  }
  SageContext.Sage3D->render.zbuffer.buffer = NULL;
}

/**
 * Enable/disable Z buffer
 *
 * @param status Z buffer status
 *
 * @return New Z buffer status
 */
BOOL SAGE_EnableZBuffer(BOOL status)
{
  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return FALSE;
  })
  if (status) {
    SD(SAGE_DebugLog("Enable Z buffer");)
    if (SAGE_AllocateZBuffer()) {
      SageContext.Sage3D->render.options |= S3DR_ZBUFFER;
    }
  } else {
    SD(SAGE_DebugLog("Disable Z buffer");)
    SageContext.Sage3D->render.options &= ~S3DR_ZBUFFER;
    if (SageContext.Sage3D->render_system == S3DD_M3DRENDER && SageContext.Sage3D->m3d_context != NULL) {
      M3D_SetState(SageContext.Sage3D->m3d_context, M3D_ZBUFFER, M3D_DISABLE);
    }
  }
  return (BOOL)(SageContext.Sage3D->render.options & S3DR_ZBUFFER);
}

/**
 * Enable/disable filtering
 *
 * @param status Filtering status
 *
 * @return New filtering status
 */
BOOL SAGE_EnableFiltering(BOOL status)
{
  UWORD index, filtering;

  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return FALSE;
  })
  if (SageContext.Sage3D->render_system == S3DD_W3DRENDER) {
    SAGE_SetError(SERR_WARP3D_LIB);
    SageContext.Sage3D->render.options &= ~S3DR_BILINEAR;
  } else if (SageContext.Sage3D->render_system == S3DD_M3DRENDER) {
    if (SageContext.Sage3D->m3d_context != NULL) {
      if (status) {
        SageContext.Sage3D->render.options |= S3DR_BILINEAR;
        filtering = M3D_LINEAR;
      } else {
        SageContext.Sage3D->render.options &= ~S3DR_BILINEAR;
        filtering = M3D_NEAREST;
      }
      M3D_SetState(SageContext.Sage3D->m3d_context, M3D_FILTERING, status);
      for (index = 0;index < STEX_MAX_TEXTURES;index++) {
        if (SageContext.Sage3D->textures[index] != NULL && SageContext.Sage3D->textures[index]->m3dtex != NULL) {
          M3D_SetFilter(SageContext.Sage3D->m3d_context, SageContext.Sage3D->textures[index]->m3dtex, filtering);
        }
      }
    } else {
      SAGE_SetError(SERR_NO_3DCONTEXT);
      SageContext.Sage3D->render.options &= ~S3DR_BILINEAR;
    }
  } else {
    // No filtering support actually
    SageContext.Sage3D->render.options &= ~S3DR_BILINEAR;
  }
  return (BOOL)(SageContext.Sage3D->render.options & S3DR_BILINEAR);
}

/**
 * Tell if a render option is active
 */
BOOL SAGE_Get3DRenderOption(LONGBITS option)
{
  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return FALSE;
  })
  return (BOOL)(SageContext.Sage3D->render.options & option);
}

/**
 * Set the rendering mode
 */
BOOL SAGE_Set3DRenderMode(UWORD mode)
{
  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return FALSE;
  })
  SageContext.Sage3D->render.render_mode = mode;
  return TRUE;
}

/**
 * Clear Z buffer
 */
BOOL SAGE_ClearZBuffer()
{
  if (SageContext.Sage3D->render_system == S3DD_W3DRENDER) {
    SAGE_SetError(SERR_WARP3D_LIB);
    return FALSE;
  } else if (SageContext.Sage3D->render_system == S3DD_M3DRENDER) {
    SAFE(if (SageContext.Sage3D->m3d_context == NULL || SageContext.Sage3D->m3d_context->zbuffer.data == NULL) {
      SAGE_SetError(SERR_ZBUFFER);
      return FALSE;
    })
    SageContext.Sage3D->maggie3d_error = M3D_ClearZBuffer(SageContext.Sage3D->m3d_context);
    if (SageContext.Sage3D->maggie3d_error != M3D_SUCCESS) {
      SAGE_SetError(SERR_ZBUFFER);
      return FALSE;
    }
    return TRUE;
  } else {
    SAFE(if (SageContext.Sage3D->render.zbuffer.buffer == NULL) {
      SAGE_SetError(SERR_ZBUFFER);
      return FALSE;
    })
    return SAGE_FastClearZBuffer(
      (ULONG)SageContext.Sage3D->render.zbuffer.buffer,
      (UWORD)SageContext.Sage3D->render.zbuffer.height,
      (UWORD)SageContext.Sage3D->render.zbuffer.bpr
    );
  }
}

/**
 * Add an element to the rendering queue
 *
 * @param elementt Element to add to the queue
 *
 * @return Operation success
 */
BOOL SAGE_Push3DElement(SAGE_3DElement *element)
{
  SAGE_Render *render;

  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return FALSE;
  })
  render = &(SageContext.Sage3D->render);
  if (render->render_elements < S3DR_MAX_ELEMENTS) {
    memcpy(&(render->s3d_elements[render->render_elements]), element, sizeof(SAGE_3DElement));
    render->ordered_elements[render->render_elements].element = &(render->s3d_elements[render->render_elements]);
    if (element->type == S3DR_ELEM_POINT) {
      render->ordered_elements[render->render_elements].avgz = element->z1;
    } else if (element->type == S3DR_ELEM_LINE) {
      render->ordered_elements[render->render_elements].avgz = (element->z1 + element->z2) / 2.0;
    } else if (element->type == S3DR_ELEM_TRIANGLE) {
      render->ordered_elements[render->render_elements].avgz = (element->z1 + element->z2 + element->z3) / 3.0;
    } else {
      render->ordered_elements[render->render_elements].avgz = (element->z1 + element->z2 + element->z3 + element->z4) / 4.0;
    }
    render->render_elements++;
    return TRUE;
  }
  return FALSE;
}

/**
 * Ascending quick sort the elements in the rendering queue
 *
 * @param elements Elements queue
 * @param low      Lower index for sorting
 * @param high     Higher index for sorting
 */
VOID SAGE_AscQuicksortElements(SAGE_SortedElement *elements, LONG low, LONG high)
{
  SAGE_SortedElement temp;
  DOUBLE pivot;
  LONG idx_low, idx_high;

  if (low >= high) return;
  idx_low = low+1;
  idx_high = high;
  pivot = elements[low].avgz;
  while (idx_low <= idx_high) {
    while (elements[idx_low].avgz <= pivot && idx_low <= high) idx_low++;
    while (elements[idx_high].avgz > pivot && idx_high >= low) idx_high--;
    if (idx_low < idx_high) {
      temp.element = elements[idx_low].element;
      temp.avgz = elements[idx_low].avgz;
      elements[idx_low].element = elements[idx_high].element;
      elements[idx_low].avgz = elements[idx_high].avgz;
      elements[idx_high].element = temp.element;
      elements[idx_high].avgz = temp.avgz;
      idx_low++;
      idx_high--;
    }
  }
  temp.element = elements[low].element;
  temp.avgz = elements[low].avgz;
  elements[low].element = elements[idx_high].element;
  elements[low].avgz = elements[idx_high].avgz;
  elements[idx_high].element = temp.element;
  elements[idx_high].avgz = temp.avgz;
  SAGE_AscQuicksortElements(elements, low, idx_high-1);
  SAGE_AscQuicksortElements(elements, idx_high+1, high);
}

/**
 * Descending quick sort the elements in the rendering queue
 *
 * @param elements Elements queue
 * @param low      Lower index for sorting
 * @param high     Higher index for sorting
 */
VOID SAGE_DescQuicksortElements(SAGE_SortedElement *elements, LONG low, LONG high)
{
  SAGE_SortedElement temp;
  DOUBLE pivot;
  LONG idx_low, idx_high;

  if (low >= high) return;
  idx_low = low+1;
  idx_high = high;
  pivot = elements[low].avgz;
  while (idx_low <= idx_high) {
    while (elements[idx_low].avgz >= pivot && idx_low <= high) idx_low++;
    while (elements[idx_high].avgz < pivot && idx_high >= low) idx_high--;
    if (idx_low < idx_high) {
      temp.element = elements[idx_low].element;
      temp.avgz = elements[idx_low].avgz;
      elements[idx_low].element = elements[idx_high].element;
      elements[idx_low].avgz = elements[idx_high].avgz;
      elements[idx_high].element = temp.element;
      elements[idx_high].avgz = temp.avgz;
      idx_low++;
      idx_high--;
    }
  }
  temp.element = elements[low].element;
  temp.avgz = elements[low].avgz;
  elements[low].element = elements[idx_high].element;
  elements[low].avgz = elements[idx_high].avgz;
  elements[idx_high].element = temp.element;
  elements[idx_high].avgz = temp.avgz;
  SAGE_DescQuicksortElements(elements, low, idx_high-1);
  SAGE_DescQuicksortElements(elements, idx_high+1, high);
}

/**
 * Sort the elements in the rendering queue
 *
 * @param ascending Ascending sort
 *
 * @return Operation success
 */
BOOL SAGE_Sort3DElements(BOOL ascending)
{
  SAGE_Render *render;

  SD(SAGE_TraceLog("SAGE_Sort3DElements(%s)", (ascending ? "TRUE" : "FALSE"));)
  SAFE(if (SageContext.Sage3D == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return FALSE;
  })
  render = &(SageContext.Sage3D->render);
  if (ascending) {
    SAGE_AscQuicksortElements(render->ordered_elements, 0, render->render_elements-1);
  } else {
    SAGE_DescQuicksortElements(render->ordered_elements, 0, render->render_elements-1);
  }
  return TRUE;
}

/**
 * Render elements in wireframe mode
 */
VOID SAGE_RenderWired3DElements(SAGE_SortedElement *elements, UWORD nb_elements)
{
  SAGE_3DElement *element;
  UWORD index;

  SD(SAGE_TraceLog("** SAGE_RenderWired3DElements(nb_elements %d)", nb_elements);)
  for (index = 0;index < nb_elements;index++) {
    element = elements[index].element;
    if (element->type == S3DR_ELEM_LINE) {
      SAGE_DrawClippedLine((LONG)(element->x1), (LONG)(element->y1),
        (LONG)(element->x2), (LONG)(element->y2), element->color);
    }
    if (element->type == S3DR_ELEM_TRIANGLE) {
      SAGE_DrawClippedLine((LONG)(element->x1), (LONG)(element->y1),
        (LONG)(element->x2), (LONG)(element->y2), element->color);
      SAGE_DrawClippedLine((LONG)(element->x2), (LONG)(element->y2),
        (LONG)(element->x3),(LONG)(element->y3), element->color);
      SAGE_DrawClippedLine((LONG)(element->x3), (LONG)(element->y3),
        (LONG)(element->x1), (LONG)(element->y1), element->color);
    }
    if (element->type == S3DR_ELEM_QUAD) {
      SAGE_DrawClippedLine((LONG)(element->x1), (LONG)(element->y1),
        (LONG)(element->x2), (LONG)(element->y2), element->color);
      SAGE_DrawClippedLine((LONG)(element->x2), (LONG)(element->y2),
        (LONG)(element->x3),(LONG)(element->y3), element->color);
      SAGE_DrawClippedLine((LONG)(element->x3), (LONG)(element->y3),
        (LONG)(element->x4), (LONG)(element->y4), element->color);
      SAGE_DrawClippedLine((LONG)(element->x4), (LONG)(element->y4),
        (LONG)(element->x1), (LONG)(element->y1), element->color);
    }
  }
}

/**
 * Render elements in flat mode
 */
VOID SAGE_RenderFlatted3DElements(SAGE_Screen *screen, SAGE_SortedElement *elements, UWORD nb_elements)
{
  SAGE_3DElement *element;
  S3D_Triangle s3d_triangle;
  UWORD index;

  SD(SAGE_TraceLog("** SAGE_RenderFlatted3DElements(nb_elements %d)", nb_elements);)
  for (index = 0;index < nb_elements;index++) {
    element = elements[index].element;
    if (element->type == S3DR_ELEM_LINE) {
      SAGE_DrawClippedLine(
        (LONG)(element->x1), (LONG)(element->y1), (LONG)(element->x2), (LONG)(element->y2), element->color
      );
    } else if (element->type > S3DR_ELEM_LINE) {
      s3d_triangle.x1 = element->x1;
      s3d_triangle.y1 = element->y1;
      s3d_triangle.z1 = element->z1;
      s3d_triangle.x2 = element->x2;
      s3d_triangle.y2 = element->y2;
      s3d_triangle.z2 = element->z2;
      s3d_triangle.x3 = element->x3;
      s3d_triangle.y3 = element->y3;
      s3d_triangle.z3 = element->z3;
      s3d_triangle.color = SAGE_RemapColor(element->color);
      s3d_triangle.tex = NULL;
      SAGE_DrawColoredTriangle(&s3d_triangle, screen->back_bitmap, &(screen->clipping));
      if (element->type == S3DR_ELEM_QUAD) {
        s3d_triangle.x1 = element->x1;
        s3d_triangle.y1 = element->y1;
        s3d_triangle.z1 = element->z1;
        s3d_triangle.x2 = element->x4;
        s3d_triangle.y2 = element->y4;
        s3d_triangle.z2 = element->z4;
        s3d_triangle.x3 = element->x3;
        s3d_triangle.y3 = element->y3;
        s3d_triangle.z3 = element->z3;
        SAGE_DrawColoredTriangle(&s3d_triangle, screen->back_bitmap, &(screen->clipping));
      }
    }
  }
}

/**
 * Render elements with internal system
 */
VOID SAGE_RenderSage3DElements(SAGE_Screen *screen, SAGE_SortedElement *elements, UWORD nb_elements)
{
  SAGE_3DElement *element;
  S3D_Triangle s3d_triangle;
  UWORD index;
  
  SD(SAGE_TraceLog("** SAGE_RenderSage3DElements(nb_elements %d)", nb_elements);)
  for (index = 0;index < nb_elements;index++) {
    element = elements[index].element;
    if (element->type == S3DR_ELEM_LINE) {
      SAGE_DrawClippedLine(
        (LONG)(element->x1), (LONG)(element->y1), (LONG)(element->x2), (LONG)(element->y2), element->color
      );
    } else if (element->type > S3DR_ELEM_LINE) {
      s3d_triangle.x1 = element->x1;
      s3d_triangle.y1 = element->y1;
      s3d_triangle.z1 = element->z1;
      s3d_triangle.u1 = element->u1;
      s3d_triangle.v1 = element->v1;
      s3d_triangle.x2 = element->x2;
      s3d_triangle.y2 = element->y2;
      s3d_triangle.z2 = element->z2;
      s3d_triangle.u2 = element->u2;
      s3d_triangle.v2 = element->v2;
      s3d_triangle.x3 = element->x3;
      s3d_triangle.y3 = element->y3;
      s3d_triangle.z3 = element->z3;
      s3d_triangle.u3 = element->u3;
      s3d_triangle.v3 = element->v3;
      s3d_triangle.color = SAGE_RemapColor(element->color);
      s3d_triangle.tex = SAGE_GetTexture(element->texture);
      if (s3d_triangle.tex == NULL) {
        SAGE_DrawColoredTriangle(&s3d_triangle, screen->back_bitmap, &(screen->clipping));
      } else {
        SAGE_DrawTexturedTriangle(&s3d_triangle, screen->back_bitmap, &(screen->clipping));
      }
      if (element->type == S3DR_ELEM_QUAD) {
        s3d_triangle.x1 = element->x1;
        s3d_triangle.y1 = element->y1;
        s3d_triangle.z1 = element->z1;
        s3d_triangle.u1 = element->u1;
        s3d_triangle.v1 = element->v1;
        s3d_triangle.x2 = element->x4;
        s3d_triangle.y2 = element->y4;
        s3d_triangle.z2 = element->z4;
        s3d_triangle.u2 = element->u4;
        s3d_triangle.v2 = element->v4;
        s3d_triangle.x3 = element->x3;
        s3d_triangle.y3 = element->y3;
        s3d_triangle.z3 = element->z3;
        s3d_triangle.u3 = element->u3;
        s3d_triangle.v3 = element->v3;
        if (s3d_triangle.tex == NULL) {
          SAGE_DrawColoredTriangle(&s3d_triangle, screen->back_bitmap, &(screen->clipping));
        } else {
          SAGE_DrawTexturedTriangle(&s3d_triangle, screen->back_bitmap, &(screen->clipping));
        }
      }
    }
  }
}

/**
 * Render elements with Warp3D
 */
VOID SAGE_RenderWarp3DElements(SAGE_Screen *screen, W3D_Context *context, SAGE_SortedElement *elements, UWORD nb_elements)
{
  (void)screen;
  (void)context;
  (void)elements;
  (void)nb_elements;
  SD(SAGE_TraceLog("** SAGE_RenderWarp3DElements(nb_elements %d)", nb_elements);)
  SAGE_SetError(SERR_WARP3D_LIB);
}

/**
 * Render elements with Maggie
 */
VOID SAGE_RenderMaggie3DElements(SAGE_Screen *screen, M3D_Context *context, SAGE_SortedElement *elements, UWORD nb_elements)
{
  SAGE_3DElement *element;
  M3D_Triangle triangle;
  M3D_Quad quad;
  LONG error;
  UWORD index;

  SD(SAGE_TraceLog("** SAGE_RenderMaggie3DElements(nb_elements %d)", nb_elements);)
  if (context == NULL) {
    SAGE_SetError(SERR_NO_3DCONTEXT);
    return;
  }
  if (screen->screen_buffer.back_buffer == NULL || screen->screen_buffer.back_buffer->sb_BitMap == NULL) {
    SAGE_SetError(SERR_NO_BITMAP);
    return;
  }
  error = M3D_SetDrawRegion(context, screen->screen_buffer.back_buffer->sb_BitMap, NULL);
  if (error != M3D_SUCCESS) {
    SageContext.Sage3D->maggie3d_error = error;
    SAGE_SetError(SERR_NO_BITMAP);
    return;
  }
  error = M3D_LockHardware(context);
  if (error != M3D_SUCCESS) {
    SageContext.Sage3D->maggie3d_error = error;
    SAGE_SetError(SERR_LOCKHARDWARE);
    return;
  }
  for (index = 0;index < nb_elements;index++) {
    element = elements[index].element;
    if (element->type == S3DR_ELEM_LINE) {
      SAGE_DrawClippedLine(
        (LONG)(element->x1), (LONG)(element->y1), (LONG)(element->x2), (LONG)(element->y2), element->color
      );
    } else if (element->type == S3DR_ELEM_TRIANGLE) {
      SAGE_SetMaggieTriangle(&triangle, element);
      error = M3D_DrawTriangle(context, &triangle);
      if (SAGE_IsMaggieDrawError(error)) {
        SageContext.Sage3D->maggie3d_error = error;
        SAGE_SetError(SERR_DRAWTRIANGLE);
        break;
      }
    } else if (element->type == S3DR_ELEM_QUAD) {
      SAGE_SetMaggieQuad(&quad, element);
      error = M3D_DrawQuad(context, &quad);
      if (SAGE_IsMaggieDrawError(error)) {
        SageContext.Sage3D->maggie3d_error = error;
        SAGE_SetError(SERR_DRAWTRIANGLE);
        break;
      }
    }
  }
  M3D_UnlockHardware(context);
}

/**
 * Render all elements in the queue
 *
 * @return Operation success
 */
BOOL SAGE_Render3DElements()
{
  SAGE_Screen *screen;
  SAGE_3DDevice *device;

  SD(SAGE_TraceLog("*** Render 3D elements ***");)
  SAFE(if (SageContext.SageVideo == NULL) {
    SAGE_SetError(SERR_NO_VIDEODEVICE);
    return FALSE;
  })
  screen = SageContext.SageVideo->screen;
  SAFE(if (screen == NULL) {
    SAGE_SetError(SERR_NO_SCREEN);
    return FALSE;
  })
  device = SageContext.Sage3D;
  SAFE(if (device == NULL) {
    SAGE_SetError(SERR_NO_3DDEVICE);
    return FALSE;
  })
  // Sort elements list
  if (!SAGE_Get3DRenderOption(S3DR_ZBUFFER)) {
    SD(SAGE_TraceLog("** ZBuffer is disable");)
    SAGE_Sort3DElements(FALSE);  // Descending mode
    SD(SAGE_TraceLog("** Descending sort elements");)
    SED(SAGE_DumpElementList(device->render.ordered_elements, device->render.render_elements);)
  } else {
    SD(SAGE_TraceLog("** ZBuffer is enable");)
    SAGE_Sort3DElements(TRUE);  // Ascending mode
    SD(SAGE_TraceLog("** Ascending sort elements");)
    SED(SAGE_DumpElementList(device->render.ordered_elements, device->render.render_elements);)
    SD(SAGE_TraceLog("** Clearing Z buffer");)
    SAGE_ClearZBuffer();
  }
  if (device->render.render_mode == S3DR_RENDER_WIRE) {
    SAGE_RenderWired3DElements(device->render.ordered_elements, device->render.render_elements);
  } else if (device->render.render_mode == S3DR_RENDER_FLAT) {
    SAGE_RenderFlatted3DElements(screen, device->render.ordered_elements, device->render.render_elements);
  } else {
    if (device->render_system == S3DD_W3DRENDER) {
      SAGE_RenderWarp3DElements(screen, device->w3d_context, device->render.ordered_elements, device->render.render_elements);
    } else if (device->render_system == S3DD_M3DRENDER) {
      SAGE_RenderMaggie3DElements(screen, device->m3d_context, device->render.ordered_elements, device->render.render_elements);
    } else {
      SAGE_RenderSage3DElements(screen, device->render.ordered_elements, device->render.render_elements);
    }
  }
  device->render.render_elements = 0;
  return TRUE;
}
