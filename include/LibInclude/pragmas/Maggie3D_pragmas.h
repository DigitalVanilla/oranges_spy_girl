/**/
/* maggie3d.library*/
/**/
/* Fabrice "Lexo" Labrador <fabrice.labrador@gmail.com>*/
/* V1.5 June 2024 (updated: 01/06/2024)*/
/**/
/* Original V1.0 -> V1.2 functions*/
#pragma libcall Maggie3DBase M3D_CheckMaggie 1e 0
#pragma libcall Maggie3DBase M3D_CreateContext 24 9802
#pragma libcall Maggie3DBase M3D_DestroyContext 2a 801
#pragma libcall Maggie3DBase M3D_SetDrawRegion 30 A9803
#pragma libcall Maggie3DBase M3D_SetState 36 10803
#pragma libcall Maggie3DBase M3D_AllocZBuffer 3c 801
#pragma libcall Maggie3DBase M3D_FreeZBuffer 42 801
#pragma libcall Maggie3DBase M3D_ClearZBuffer 48 801
#pragma libcall Maggie3DBase M3D_AllocTexture 4e B210A9807
#pragma libcall Maggie3DBase M3D_AllocTextureFile 54 A9803
#pragma libcall Maggie3DBase M3D_FreeTexture 5a 9802
#pragma libcall Maggie3DBase M3D_LockHardware 60 801
#pragma libcall Maggie3DBase M3D_UnlockHardware 66 801
#pragma libcall Maggie3DBase M3D_DrawTriangle 6c 9802
#pragma libcall Maggie3DBase M3D_DrawTriangleArray 72 09803
#pragma libcall Maggie3DBase M3D_DrawTriangleList 78 09803
/* New V1.3 functions*/
#pragma libcall Maggie3DBase M3D_GetState 7e 0802
#pragma libcall Maggie3DBase M3D_SetFilter 84 09803
/* New V1.4 functions*/
#pragma libcall Maggie3DBase M3D_BestModeID 8a 21003
#pragma libcall Maggie3DBase M3D_SetScissor 90 9802
#pragma libcall Maggie3DBase M3D_ClearDrawRegion 96 0802
/* New V1.5 functions*/
#pragma libcall Maggie3DBase M3D_FreeAllTextures 9c 801
#pragma libcall Maggie3DBase M3D_AllocTextureTagList a2 A9803
#pragma libcall Maggie3DBase M3D_DrawSprite a8 109804
/* New V1.6 functions*/
#pragma libcall Maggie3DBase M3D_DrawQuad ae 9802
