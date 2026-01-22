/*
	MD3TexDB.cpp - Source for MD3Texture Database implimentation.

	Copryight (c) 2003 Blaine Myers
*/
#define D3D_MD3
#include <d3dx9.h>
#include "defines.h"
#include "Functions.h"
#include "md3.h"
#include "md3texdb.h"

/*
	CMD3Texture class methods.

	The CMD3Texture objects represent nodes in a linked-list. Each
	object can get the next item in the list by calling GetNext().
	The last object in the list will have m_lpNext set to NULL.
*/

//Constructor simply sets all values to 0 or NULL.
CMD3Texture::CMD3Texture()
{
	m_Texture.lpTexture=NULL;
	m_Texture.szTextureName[0]=0;

	m_lpNext=NULL;
}

//Destructor releases the texture.
CMD3Texture::~CMD3Texture()
{

	if(m_Texture.lpTexture)
		m_Texture.lpTexture->Release();

}

//Returns the next node in the linked-list
CMD3Texture * CMD3Texture::GetNext()
{
	return m_lpNext;
}

//Sets the next node in the list.
void CMD3Texture::SetNext(CMD3Texture * lpNext)
{
	m_lpNext=lpNext;
}

//Sets the texture data for the node.
void CMD3Texture::SetTexture(MD3TEXTURE * lpTexture)
{
	m_Texture.lpTexture=lpTexture->lpTexture;
	strcpy(m_Texture.szTextureName, lpTexture->szTextureName);
}

//Gets the texture data for the node.
void CMD3Texture::GetTexture(MD3TEXTURE * lpTexture)
{
	lpTexture->lpTexture=m_Texture.lpTexture;
	strcpy(lpTexture->szTextureName, m_Texture.szTextureName);
}

/*
	CMD3TextureDB class methods.

	The CMD3TextureDB object manages a linked list.  The first object
	contains a pointer to the second object, which in turn has a
	pointer to the next object and so on.
*/

CMD3TextureDB::CMD3TextureDB():
m_dwNumTextures(0)
{
	//Constructor simly sets the first link to NULL.
	m_lpFirst=NULL;
}

CMD3TextureDB::~CMD3TextureDB()
{
	//Destructor insures that the database is clear.
	ClearDB();
}

//Returns the number of textures
HRESULT CMD3TextureDB::GetNumTextures(DWORD * dwNumTex)
{
	*dwNumTex=m_dwNumTextures;
	return S_OK;
}

//Adds a texture from a filename, uses filename as texture name.
HRESULT CMD3TextureDB::AddTexture(LPDIRECT3DDEVICE9 lpDevice, char szTexName[MAX_PATH])
{
	CMD3Texture * lpCurrent=NULL;
	CMD3Texture * lpNext=NULL;

	LPDIRECT3DTEXTURE9 lpTexture=NULL;

	MD3TEXTURE md3Texture;
	HRESULT hr=0;
	
	char szName[MAX_PATH];

	ZeroMemory(&md3Texture, sizeof(MD3TEXTURE));

	//Reomving the path directory from the filename gives a good
	//texture name.
	RemoveDirectoryFromString(szName, szTexName);

	//If the texture already exists we silently return.
	if(SUCCEEDED(GetTexture(szName, &lpTexture))){
		lpTexture->Release();
		return S_FALSE;
	}

	
	if(m_lpFirst==NULL){
		//If there are currently no textures in the database
		//we create the first node.
		m_lpFirst=new CMD3Texture;
		if(m_lpFirst==NULL){
			return E_FAIL;
		}
		m_lpFirst->SetNext(NULL);
		//Load the texture from file.
		hr=D3DXCreateTextureFromFileExA(
			lpDevice, 
			szTexName,
			D3DX_DEFAULT,
			D3DX_DEFAULT,
			D3DX_DEFAULT,
			0,
			D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED,
			D3DX_DEFAULT,
			D3DX_DEFAULT, 
			0xFF000000L,
			NULL,
			NULL,
			&md3Texture.lpTexture);
		if(FAILED(hr)){
			//If we failed we delete the node
			//and return an error.
			SAFE_DELETE(m_lpFirst);
			return E_FAIL;
		}
		//Copy over the texture name.
		strcpy(md3Texture.szTextureName, szName);
		//Put the texture in the node.
		m_lpFirst->SetTexture(&md3Texture);
		
	}else{
		lpCurrent=m_lpFirst;

		//Go through the nodes until we find an empty one.
		while(lpCurrent->GetNext() != NULL){
			lpCurrent=lpCurrent->GetNext();
		}
		//Create the new node
		lpNext=new CMD3Texture;
		if(lpNext==NULL){
			return E_FAIL;
		}
		lpNext->SetNext(NULL);
		//Create the texture from file
		hr=D3DXCreateTextureFromFileExA(
			lpDevice, 
			szTexName,
			D3DX_DEFAULT,
			D3DX_DEFAULT,
			D3DX_DEFAULT,
			0,
			D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED,
			D3DX_DEFAULT,
			D3DX_DEFAULT, 
			0xFF000000L,
			NULL,
			NULL,
			&md3Texture.lpTexture);
		if(FAILED(hr)){
			SAFE_DELETE(lpNext);
			return E_FAIL;
		}
		strcpy(md3Texture.szTextureName, szName);
		lpNext->SetTexture(&md3Texture);
		//Set the new node as the next node to the old last node.
		lpCurrent->SetNext(lpNext);
	}
	//Increment the number of textures.
	m_dwNumTextures++;
	return S_OK;
}

/*
	The code for this version of AddTexture works pretty much
	the same as the previous method except it uses the szTexName
	parameter as the texture name, and it merely obtains a pointer
	to the lpTexture parameter, instead of creating the file from
	disk.  Note that it calls the AddRef function on the texture, so
	Release() will need to be called on any textures passed to this
	function before the program terminates.
*/
HRESULT CMD3TextureDB::AddTexture(LPDIRECT3DTEXTURE9 lpTexture, char szTexName[])
{
	//Se previous method for information on how this works.
	CMD3Texture * lpCurrent=NULL;
	CMD3Texture * lpNext=NULL;

	LPDIRECT3DTEXTURE9 lpTempTexture=NULL;

	MD3TEXTURE md3Texture;
	HRESULT hr=0;

	ZeroMemory(&md3Texture, sizeof(MD3TEXTURE));

	if(lpTexture==NULL)return E_FAIL;

	//First we check to see if the texture already exists.
	if(SUCCEEDED(GetTexture(szTexName, &lpTexture))){
		lpTexture->Release();
		return S_FALSE;
	}

	if(m_lpFirst==NULL){
		m_lpFirst=new CMD3Texture;
		if(m_lpFirst==NULL){
			return E_FAIL;
		}
		m_lpFirst->SetNext(NULL);
		md3Texture.lpTexture=lpTexture;
		md3Texture.lpTexture->AddRef();
		strcpy(md3Texture.szTextureName, szTexName);
		m_lpFirst->SetTexture(&md3Texture);
		
	}else{
		lpCurrent=m_lpFirst;

		while(lpCurrent->GetNext() != NULL){
			lpCurrent=lpCurrent->GetNext();
		}
		lpNext=new CMD3Texture;
		if(lpNext==NULL){
			return E_FAIL;
		}
		lpNext->SetNext(NULL);
		md3Texture.lpTexture=lpTexture;
		md3Texture.lpTexture->AddRef();
		strcpy(md3Texture.szTextureName, szTexName);
		lpNext->SetTexture(&md3Texture);
		lpCurrent->SetNext(lpNext);
	}
	m_dwNumTextures++;
	return S_OK;
}

//Retrieves a texture by a reference number.
HRESULT CMD3TextureDB::GetTexture(
	DWORD dwRef, 
	LPDIRECT3DTEXTURE9 * lppTexture)
{
	CMD3Texture * lpCurrent=NULL;
	MD3TEXTURE md3Texture;
	DWORD i=0;

	ZeroMemory(&md3Texture, sizeof(MD3TEXTURE));

	if( (dwRef>m_dwNumTextures) || (dwRef<1) )return E_FAIL;


	//What we do to retriev the chosen texture
	//is go through the nodes until we get
	//to the speicifed number.  Then we let
	//the pointer into the lppTexture param.
	lpCurrent=m_lpFirst;
	for(i=0; i<(dwRef-1); i++){
		lpCurrent=lpCurrent->GetNext();
		if(lpCurrent==NULL)return E_FAIL;
	}
	lpCurrent->GetTexture(&md3Texture);
	*lppTexture=md3Texture.lpTexture;
	md3Texture.lpTexture->AddRef();
	return S_OK;
}

//Get a texture from it's name.
HRESULT CMD3TextureDB::GetTexture(
	char szTexName[], 
	LPDIRECT3DTEXTURE9 * lppTexture)
{
	CMD3Texture * lpCurrent=NULL;
	MD3TEXTURE md3Texture;
	DWORD i=0;

	ZeroMemory(&md3Texture, sizeof(MD3TEXTURE));

	//What we do is loop through each node
	//until we find on with a name matching
	//the szTexName param.
	lpCurrent=m_lpFirst;
	for(i=0; i<m_dwNumTextures; i++){
		if(lpCurrent==NULL)return E_FAIL;
		lpCurrent->GetTexture(&md3Texture);
		if(strcmp(md3Texture.szTextureName, szTexName)==0){
			*lppTexture=md3Texture.lpTexture;
			md3Texture.lpTexture->AddRef();
			return S_OK;
		}
		lpCurrent=lpCurrent->GetNext();
	}
	return E_FAIL;
}

//Sets the current render texture based on index.
HRESULT CMD3TextureDB::SetRenderTexture(
	DWORD dwRef, 
	DWORD dwStage,
	LPDIRECT3DDEVICE9 lpDevice)
{
	CMD3Texture * lpCurrent=NULL;
	MD3TEXTURE md3Texture;
	DWORD i=0;

	ZeroMemory(&md3Texture, sizeof(MD3TEXTURE));

	if( (dwRef>m_dwNumTextures) || (dwRef<1) )return E_FAIL;


	//Go through each node till we get to the one we want.
	lpCurrent=m_lpFirst;
	for(i=0; i<(dwRef-1); i++){
		lpCurrent=lpCurrent->GetNext();
		if(lpCurrent==NULL)return E_FAIL;
	}
	lpCurrent->GetTexture(&md3Texture);
	//Render the selected texture in the chosen stage.
	(lpDevice)->SetTexture(dwStage, md3Texture.lpTexture);
	return S_OK;
}

//Sets the current render texture based on name.
HRESULT CMD3TextureDB::SetRenderTexture(
	LPSTR szTexName,
	DWORD dwStage, 
	LPDIRECT3DDEVICE9 lpDevice)
{
	CMD3Texture * lpCurrent=NULL;
	MD3TEXTURE md3Texture;
	DWORD i=0;

	ZeroMemory(&md3Texture, sizeof(MD3TEXTURE));

	//Go through each texture till we find one with the matching name.
	lpCurrent=m_lpFirst;
	for(i=0; i<m_dwNumTextures; i++){
		if(lpCurrent==NULL)return E_FAIL;
		lpCurrent->GetTexture(&md3Texture);
		if(strcmp(md3Texture.szTextureName, szTexName)==0){
			//Set the render texture in the chosen stage.
			(lpDevice)->SetTexture(dwStage, md3Texture.lpTexture);
			return S_OK;
		}
		lpCurrent=lpCurrent->GetNext();
	}
	//If no texture had the specified name we fail.
	return E_FAIL;
}

//Deletes the chosen texture out of the database by index.
HRESULT CMD3TextureDB::DeleteTexture(
	DWORD dwRef)
{
	CMD3Texture * lpPrev=NULL;
	CMD3Texture * lpCurrent=NULL;
	CMD3Texture * lpNext=NULL;

	DWORD i=0;

	lpCurrent=m_lpFirst;

	if((dwRef>m_dwNumTextures) || (dwRef<1))return E_FAIL;
	if(m_dwNumTextures<1)return E_FAIL;

	if(lpCurrent==NULL)return E_FAIL;

	if(dwRef==1){
		//If we want to delete the first
		//node we simply delete it and make
		//the second node the first node.
		lpNext=lpCurrent->GetNext();
		SAFE_DELETE(lpCurrent);
		m_lpFirst=lpNext;
	}else{
		//Go though each texture till we get to the one
		//we want to delete and remove it.  Setting it's
		//previous node's next node the it's next node.
		for(i=0; i<(dwRef-1); i++){
			if(lpCurrent==NULL)return E_FAIL;
			lpPrev=lpCurrent;
			lpCurrent=lpCurrent->GetNext();
		}
		lpNext=lpCurrent->GetNext();
		SAFE_DELETE(lpCurrent);
		lpPrev->SetNext(lpNext);
	}
	//Decrement the number of textures.
	m_dwNumTextures--;

	return S_OK;
}

//Delete a texture from the database, based on teh textures name.
HRESULT CMD3TextureDB::DeleteTexture(
	LPSTR szTexName)
{
	CMD3Texture * lpPrev=NULL;
	CMD3Texture * lpCurrent=NULL;
	CMD3Texture * lpNext=NULL;

	MD3TEXTURE md3Texture;

	DWORD i=0;

	ZeroMemory(&md3Texture, sizeof(MD3TEXTURE));

	lpCurrent=m_lpFirst;

	if(m_dwNumTextures<1)return E_FAIL;

	if(lpCurrent==NULL)return E_FAIL;

	m_lpFirst->GetTexture(&md3Texture);

	if(strcmp(md3Texture.szTextureName, szTexName)==0){
		//If the name matches the first one we 
		//do the same procedure as in the previous method.
		lpNext=lpCurrent->GetNext();
		SAFE_DELETE(lpCurrent);
		m_lpFirst=lpNext;
	}else{
		//Loop through till we find a texture with mathching name.
		for(i=0; i<m_dwNumTextures; i++){
			if(lpCurrent==NULL)return E_FAIL;
			lpPrev=lpCurrent;
			lpCurrent=lpCurrent->GetNext();
			if(lpCurrent==NULL)return E_FAIL;
			lpCurrent->GetTexture(&md3Texture);
			if(strcmp(md3Texture.szTextureName, szTexName)==0)
				break;
		}
		if(lpCurrent==NULL)return E_FAIL;
		lpCurrent->GetTexture(&md3Texture);
		if(strcmp(md3Texture.szTextureName, szTexName)!=0){
			return E_FAIL;
		}
		//When we have texture we want to delete we
		//remove it and make it's previous node's next node
		//it's next node.
		lpNext=lpCurrent->GetNext();
		SAFE_DELETE(lpCurrent);
		lpPrev->SetNext(lpNext);
	}
	m_dwNumTextures--;
	return S_FALSE;
}

//Clears the entire database.
HRESULT CMD3TextureDB::ClearDB()
{
	CMD3Texture * lpCurrent=NULL;
	CMD3Texture * lpNext=NULL;

	lpCurrent=m_lpFirst;

	//Loop through each node and delete it.
	while(lpCurrent != NULL){
		lpNext=lpCurrent->GetNext();
		SAFE_DELETE(lpCurrent);
		lpCurrent=lpNext;
	}

	//Clear the first node so we know it's empty.
	m_lpFirst=NULL;
	m_dwNumTextures=0;
	return S_OK;
}
