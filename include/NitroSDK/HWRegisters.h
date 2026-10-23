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

#define BG0OFS (*(volatile unsigned int*)0x04000010)
#define BG1OFS (*(volatile unsigned int*)0x04000014)
#define BG2OFS (*(volatile unsigned int*)0x04000018)
#define BG3OFS (*(volatile unsigned int*)0x0400001c)

#define BG0CNTSUB (*(volatile unsigned short*)(0x04001008))
#define BG1CNTSUB (*(volatile unsigned short*)(0x0400100a))
#define BG2CNTSUB (*(volatile unsigned short*)(0x0400100c))
#define BG3CNTSUB (*(volatile unsigned short*)(0x0400100e))

#define BG0OFSSUB (*(volatile unsigned int*)0x04001010)
#define BG1OFSSUB (*(volatile unsigned int*)0x04001014)
#define BG2OFSSUB (*(volatile unsigned int*)0x04001018)
#define BG3OFSSUB (*(volatile unsigned int*)0x0400101c)

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

#define DIVCNT (*(volatile unsigned short*)0x04000280)

// Not volatile, see func_020cddec
#define DIV_NUMER_64 (*(unsigned long long*)0x04000290)
#define DIV_NUMER_LOW (*(unsigned int*)0x04000290)
#define DIV_NUMER_HIGH (*(unsigned int*)0x04000294)

#define DIV_DENOM_64 (*(unsigned long long*)0x04000298)
#define DIV_DENOM_LOW (*(unsigned int*)0x04000298)
#define DIV_DENOM_HIGH (*(unsigned int*)0x0400029c)

#define DIV_RESULT_64 (*(volatile unsigned long long*)0x040002a0)
#define DIV_RESULT_LOW (*(volatile unsigned int*)0x040002a0)
#define DIV_RESULT_HIGH (*(volatile unsigned int*)0x040002a4)

#define DIVREM_RESULT_64 (*(volatile unsigned long long*)0x040002a8)
#define DIVREM_RESULT_LOW (*(volatile unsigned int*)0x040002a8)
#define DIVREM_RESULT_HIGH (*(volatile unsigned int*)0x040002ac)

#define SQRTCNT (*(volatile unsigned short*)0x040002b0)
#define SQRT_RESULT (*(volatile unsigned int*)0x040002b4)
#define SQRT_PARAM_64 (*(volatile unsigned long long*)0x040002b8)
#define SQRT_PARAM_LOW (*(volatile unsigned int*)0x040002b8)
#define SQRT_PARAM_HIGH (*(volatile unsigned int*)0x040002bc)