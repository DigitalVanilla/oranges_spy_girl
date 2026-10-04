;------------------------------------------------------------------------------
; spy_tunnel_060.asm
;
; 68060 scalar tunnel renderer.
; The C side keeps timing, pattern loading, pattern effects, and table generation.
; This routine only draws the already prepared tunnel pixels.
;------------------------------------------------------------------------------

  SECTION spy_tunnel_060,code

;------------------------------------------------------------------------------
; Span-skipping tunnel renderer. Circle geometry is prepared by the C side, so
; this path replaces texture calculations inside each core span with zero fills.
;
; void SpyTunnelRenderFrame060Asm(
;   UWORD *dst,                    8(a6)
;   const UWORD *distance_row,     12(a6)
;   const UWORD *angle_row,        16(a6)
;   ULONG table_stride,            20(a6)
;   ULONG dst_stride,              24(a6)
;   const UWORD *texture,          28(a6)
;   ULONG width,                   32(a6)
;   ULONG height,                  36(a6)
;   ULONG shift_x,                 40(a6)
;   ULONG shift_y,                 44(a6)
;   const UWORD *skip_start,       48(a6)
;   const UWORD *skip_count        52(a6)
; );
;------------------------------------------------------------------------------

  xdef _SpyTunnelRenderFrame060Asm

_SpyTunnelRenderFrame060Asm:
  link    a6,#-12
  movem.l d2-d7/a2-a5,-(sp)

  move.l  8(a6),a0                 ; destination UWORD buffer
  move.l  12(a6),a1                ; distance table row
  move.l  16(a6),a2                ; angle table row
  move.l  28(a6),a3                ; 512x512 RGB565 texture
  move.l  48(a6),a4                ; first skipped pixel for each row
  move.l  52(a6),a5                ; skipped pixel count for each row

  move.l  32(a6),d0                ; width
  tst.l   d0
  beq     .Done
  move.l  36(a6),d0                ; height
  tst.l   d0
  beq     .Done
  move.l  d0,-4(a6)                ; rows remaining

  move.l  20(a6),d5                ; table stride in bytes
  move.l  32(a6),d0
  add.l   d0,d0                    ; visible row width in table bytes
  sub.l   d0,d5                    ; skip to next table row

  move.l  24(a6),d2                ; destination stride in bytes
  move.l  32(a6),d0
  add.l   d0,d0                    ; visible row width in destination bytes
  sub.l   d0,d2                    ; skip to next destination row

  move.l  40(a6),d4                ; distance/texture-y shift
  andi.l  #$1ff,d4
  move.l  44(a6),d3                ; angle/texture-x shift
  andi.l  #$1ff,d3

.NextRow:
  moveq   #0,d6
  move.w  (a4)+,d6                 ; pixels before the black core
  moveq   #0,d7
  move.w  (a5)+,d7                 ; black core pixels

  move.l  32(a6),d0
  sub.l   d6,d0
  sub.l   d7,d0
  move.l  d0,-8(a6)                ; pixels after the black core
  move.l  d7,-12(a6)               ; preserve skip count across render call

  bsr     .RenderSpan

  move.l  -12(a6),d6
  beq     .AfterSkip

  move.l  d6,d0
  add.l   d0,d0
  adda.l  d0,a1                    ; skip distance calculations
  adda.l  d0,a2                    ; skip angle calculations

  subq.l  #1,d6
  moveq   #0,d0
.ClearSpan:
  move.w  d0,(a0)+                 ; replace mapping work with a black pixel
  dbf     d6,.ClearSpan

.AfterSkip:
  move.l  -8(a6),d6
  bsr     .RenderSpan

  adda.l  d5,a1
  adda.l  d5,a2
  adda.l  d2,a0
  subq.l  #1,-4(a6)
  bne     .NextRow
  bra     .Done

; Render d6 pixels from the current table and destination pointers. Pairing
; keeps the original loop density while allowing odd-sized circle boundaries.
.RenderSpan:
  tst.l   d6
  beq     .SpanDone

  move.l  d6,d7
  andi.l  #1,d7                    ; trailing odd pixel
  lsr.l   #1,d6                    ; complete pixel pairs
  beq     .OddPixel
  subq.l  #1,d6

.NextPixelPair:
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
  move.w  (a3,d0.l),(a0)+

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
  move.w  (a3,d0.l),(a0)+

  dbf     d6,.NextPixelPair

.OddPixel:
  tst.l   d7
  beq     .SpanDone

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
  move.w  (a3,d0.l),(a0)+

.SpanDone:
  rts

.Done:
  movem.l (sp)+,d2-d7/a2-a5
  unlk    a6
  rts
