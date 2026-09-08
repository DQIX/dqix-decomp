#pragma once

#define VRAMCNT_A (*(volatile unsigned char*)0x04000240)
#define VRAMCNT_B (*(volatile unsigned char*)0x04000241)
#define VRAMCNT_C (*(volatile unsigned char*)0x04000242)
#define VRAMCNT_D (*(volatile unsigned char*)0x04000243)
#define VRAMCNT_E (*(volatile unsigned char*)0x04000244)
#define VRAMCNT_F (*(volatile unsigned char*)0x04000245)
#define VRAMCNT_G (*(volatile unsigned char*)0x04000246)
#define VRAMCNT_H (*(volatile unsigned char*)0x04000248)
#define VRAMCNT_I (*(volatile unsigned char*)0x04000249)

#define DISPCNT (*(volatile unsigned int*)0x04000000)
#define DISPCNTSUB (*(volatile unsigned int*)0x04001000)
#define DISP3DCNT (*(volatile unsigned short*)0x04000060)

#define VCOUNT (*(volatile unsigned short*)0x04000006)

#define BG0CNT (*(volatile unsigned short*)(0x04000008))
#define BG1CNT (*(volatile unsigned short*)(0x0400000a))
#define BG2CNT (*(volatile unsigned short*)(0x0400000c))
#define BG3CNT (*(volatile unsigned short*)(0x0400000e))

#define BG0CNTSUB (*(volatile unsigned short*)(0x04001008))
#define BG1CNTSUB (*(volatile unsigned short*)(0x0400100a))
#define BG2CNTSUB (*(volatile unsigned short*)(0x0400100c))
#define BG3CNTSUB (*(volatile unsigned short*)(0x0400100e))

#define GX_FOG_COLOR (*(volatile unsigned int*)0x04000358)
#define GX_FOG_OFFSET (*(volatile unsigned int*)0x0400035c)

#define GXFIFO (*(volatile unsigned int*)0x04000400)
#define GXFIFO_MATRIX_MODE (*(volatile unsigned int*)0x04000440)
#define GXFIFO_MATRIX_PUSH (*(volatile unsigned int*)0x04000444)
#define GXFIFO_MATRIX_POP (*(volatile unsigned int*)0x04000448)
#define GXFIFO_MATRIX_STORE (*(volatile unsigned int*)0x0400044c)
#define GXFIFO_MATRIX_GET (*(volatile unsigned int*)0x04000450)
#define GXFIFO_MATRIX_IDENTITY (*(volatile unsigned int*)0x04000454)

#define GXFIFO_MATRIX_SCALE (*(volatile unsigned int*)0x0400046c)
#define GXFIFO_MATRIX_TRANSLATE (*(volatile unsigned int*)0x04000470)
#define GXFIFO_VERTEX_COLOR (*(volatile unsigned int*)0x04000480)
#define GXFIFO_VERTEX_NORMAL (*(volatile unsigned int*)0x04000484)
#define GXFIFO_VERTEX_TEXCOORD (*(volatile unsigned int*)0x04000488)
#define GXFIFO_VERTEX_16 (*(volatile unsigned int*)0x0400048c)
#define GXFIFO_POLYGON_ATTRIBUTES (*(volatile unsigned int*)0x040004a4)
#define GXFIFO_TEXIMAGE_PARAMS (*(volatile unsigned int*)0x040004a8)
#define GXFIFO_PALETTE_BASE (*(volatile unsigned int*)0x040004ac)
#define GXFIFO_DIFFUSE_AMBIENT (*(volatile unsigned int*)0x040004c0)
#define GXFIFO_SPECULAR_EMISSION (*(volatile unsigned int*)0x040004c4)

#define GXFIFO_POLYGON_BEGIN (*(volatile unsigned int*)0x04000500)
#define GXFIFO_POLYGON_END (*(volatile unsigned int*)0x04000504)

#define GXFIFO_TEST_BOX (*(volatile unsigned int*)0x040005c0)

#define GXSTATUS (*(volatile unsigned int*)0x04000600)

#define BLEND_TARGET_BG0 1
#define BLEND_TARGET_BG1 2
#define BLEND_TARGET_BG2 4
#define BLEND_TARGET_BG3 8
#define BLEND_TARGET_OBJ 0x10
#define BLEND_TARGET_BACKDROP 0x20

#define DISPCNT_MASK_BG_MODE 7
// right shift by 24 for numerical value
#define DISPCNT_MASK_CHARACTER_BASE_64K 0x07000000
// right shift by 27 for numerical value
#define DISPCNT_MASK_SCREEN_BASE_64K 0x38000000
#define DISPCNT_ENABLE_BG_EXTENDED_PALETTE 0x40000000
#define DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE 0x80000000

#define DISP3DCNT_USE_CLEAR_TEXTURES 0x4000

#define BGCNT_MASK_PRIORITY 3
// right shift by 2 for numerical value
#define BGCNT_MASK_CHARACTER_BASE_16K 0x3c
#define BGCNT_MASK_MOSAIC 0x40
#define BGCNT_MASK_PALETTE_256 0x80
// right shift by 8 for numerical value
#define BGCNT_MASK_SCREEN_BASE_2K 0x1f00
#define BGCNT_MASK_BG01_EXTENDED_PALETTE 0x2000
#define BGCNT_MASK_SCREEN_SIZE 0xc000