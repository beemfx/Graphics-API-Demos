/*
	Defines.h - Definitions used for MD3 file support.
*/

#ifndef __DEFINES_H__
#define __DEFINES_H__

#define SAFE_FREE(p)         { if(p) { free(p); (p)=NULL; } }

#ifdef __cplusplus
#define SAFE_DELETE(p)       { if(p) { delete (p); (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete [] (p); (p)=NULL; } }
#endif /* __cplusplus */

#ifdef __cplusplus
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#else /* __cplusplus */
#define SAFE_RELEASE(p)      { if(p) { (p)->lpVtbl->Release((p)); (p)=NULL; } }
#endif /* __cplusplus */

#endif /* __DEFINES_H__ */