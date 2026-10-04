;------------------------------------------------------------------------------
; spy_tunnel_2x_060.asm
;
; Half-resolution tunnel renderer with exact 2x nearest-neighbour expansion.
; Each mapped source pixel is written directly as a 2x2 RGB565 block.
;------------------------------------------------------------------------------

  SECTION spy_tunnel_2x_060,code

; void SpyTunnelRenderFrame2x060Asm(
;   UWORD *dst,                    8(a6)
;   const UWORD *distance_row,    12(a6)
;   const UWORD *angle_row,       16(a6)
;   ULONG table_stride,           20(a6)
;   ULONG dst_stride,             24(a6)
;   const UWORD *texture,         28(a6)
;   ULONG width,                  32(a6)
;   ULONG height,                 36(a6)
;   ULONG shift_x,                40(a6)
;   ULONG shift_y,                44(a6)
;   const UWORD *skip_start,      48(a6)
;   const UWORD *skip_count       52(a6)
; );

  xdef _SpyTunnelRenderFrame2x060Asm

_SpyTunnelRenderFrame2x060Asm:
  link    a6,#-20
  movem.l d2-d7/a2-a5,-(sp)

  move.l  8(a6),a0                 ; first destination row
  move.l  12(a6),a1                ; distance table row
  move.l  16(a6),a2                ; angle table row
  move.l  28(a6),a3                ; 512x512 RGB565 texture
  move.l  48(a6),-16(a6)           ; per-row black span starts
  move.l  52(a6),-20(a6)           ; per-row black span lengths

  move.l  32(a6),d0                ; low-resolution width
  tst.l   d0
  beq     .Done
  move.l  36(a6),d0                ; low-resolution height
  tst.l   d0
  beq     .Done
  move.l  d0,-4(a6)                ; rows remaining

  move.l  20(a6),d5                ; distance/angle row stride
  move.l  32(a6),d0
  add.l   d0,d0                    ; mapped table bytes per source row
  sub.l   d0,d5                    ; table skip after each source row

  move.l  24(a6),d2                ; destination row stride in bytes
  add.l   d2,d2                    ; advance over two output rows
  move.l  32(a6),d0
  lsl.l   #2,d0                    ; expanded output bytes per row
  sub.l   d0,d2                    ; destination skip after each row pair

  move.l  a0,a4
  adda.l  24(a6),a4                ; second destination row

  move.l  40(a6),d4                ; distance/texture-y shift
  andi.l  #$1ff,d4
  move.l  44(a6),d3                ; angle/texture-x shift
  andi.l  #$1ff,d3

.NextRow:
  move.l  -16(a6),a5
  moveq   #0,d6
  move.w  (a5)+,d6                 ; mapped pixels before black core
  move.l  a5,-16(a6)

  move.l  -20(a6),a5
  moveq   #0,d7
  move.w  (a5)+,d7                 ; mapped black core pixels
  move.l  a5,-20(a6)

  move.l  32(a6),d0
  sub.l   d6,d0
  sub.l   d7,d0
  move.l  d0,-8(a6)                ; mapped pixels after black core
  move.l  d7,-12(a6)

  bsr     .RenderSpan

  move.l  -12(a6),d6
  beq     .AfterCore

  move.l  d6,d0
  add.l   d0,d0
  adda.l  d0,a1                    ; skip distance calculations
  adda.l  d0,a2                    ; skip angle calculations

  subq.l  #1,d6
  moveq   #0,d0
.ClearCore:
  move.l  d0,(a0)+                 ; two horizontal black pixels
  move.l  d0,(a4)+                 ; duplicate into the second row
  dbf     d6,.ClearCore

.AfterCore:
  move.l  -8(a6),d6
  bsr     .RenderSpan

  adda.l  d5,a1
  adda.l  d5,a2
  adda.l  d2,a0
  adda.l  d2,a4
  subq.l  #1,-4(a6)
  bne     .NextRow
  bra     .Done

; Map d6 source pixels and write each RGB565 value as a 2x2 block.
.RenderSpan:
  tst.l   d6
  beq     .SpanDone
  subq.l  #1,d6

.NextPixel:
  moveq   #0,d0
  move.w  (a1)+,d0
  add.w   d4,d0
  andi.l  #$1ff,d0
  lsl.l   #8,d0
  add.l   d0,d0

  moveq   #0,d1
  move.w  (a2)+,d1
  add.w   d3,d1
  andi.l  #$1ff,d1
  add.l   d1,d0
  add.l   d0,d0

  move.w  (a3,d0.l),d1
  move.l  d1,d0
  swap    d1
  move.w  d0,d1                    ; duplicate RGB565 into both words
  move.l  d1,(a0)+
  move.l  d1,(a4)+
  dbf     d6,.NextPixel

.SpanDone:
  rts

.Done:
  movem.l (sp)+,d2-d7/a2-a5
  unlk    a6
  rts
