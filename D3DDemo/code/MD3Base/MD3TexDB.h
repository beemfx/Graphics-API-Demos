#ifndef __MD3TEXDB_H__
#define __MD3TEXDB_H__

#include <d3d9.h>

typedef struct tagMD3TEXTURE
{
	LPDIRECT3DTEXTURE9 lpTexture;
	char szTextureName[MAX_PATH];
}MD3TEXTURE;

#ifdef __cplusplus

class CMD3Texture
{
private:
	MD3TEXTURE m_Texture;
	CMD3Texture * m_lpNext;
public:
	CMD3Texture();
	~CMD3Texture();
	CMD3Texture * GetNext();
	void SetNext(CMD3Texture * lpNext);

	void SetTexture(MD3TEXTURE * lpTexture);
	void GetTexture(MD3TEXTURE * lpTexture);
};

#endif /* __cplusplus */


#endif /* __MD3TEXDB_H__ */
