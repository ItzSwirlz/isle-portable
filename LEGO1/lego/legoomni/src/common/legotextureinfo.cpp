#include "legotextureinfo.h"

#include "extensions/textureloader.h"
#include "legovideomanager.h"
#include "misc.h"
#include "misc/legoimage.h"
#include "misc/legotexture.h"
#include "mxdirectx/mxdirect3d.h"
#include "tgl/d3drm/impl.h"

DECOMP_SIZE_ASSERT(LegoTextureInfo, 0x10)

using namespace Extensions;

// FUNCTION: LEGO1 0x10065bf0
LegoTextureInfo::LegoTextureInfo()
{
	m_name = NULL;
	m_surface = NULL;
	m_palette = NULL;
	m_texture = NULL;
}

// FUNCTION: LEGO1 0x10065c00
LegoTextureInfo::~LegoTextureInfo()
{
	if (m_name) {
		delete[] m_name;
		m_name = NULL;
	}

	if (m_palette) {
		m_palette->Release();
		m_palette = NULL;
	}

	if (m_surface) {
		m_surface->Release();
		m_surface = NULL;
	}

	if (m_texture) {
		m_texture->Release();
		m_texture = NULL;
	}
}

// FUNCTION: LEGO1 0x10065c60
LegoTextureInfo* LegoTextureInfo::Create(const char* p_name, LegoTexture* p_texture)
{
	LegoTextureInfo* textureInfo = new LegoTextureInfo();

	if (p_name == NULL || p_texture == NULL) {
		return NULL;
	}

	if (p_name) {
		textureInfo->m_name = new char[strlen(p_name) + 1];
		strcpy(textureInfo->m_name, p_name);
	}

	if (Extension<TextureLoader>::Call(PatchTexture, textureInfo).value_or(false)) {
		return textureInfo;
	}

	LPDIRECTDRAW pDirectDraw = VideoManager()->GetDirect3D()->DirectDraw();
	LegoImage* image = p_texture->GetImage();

	DDSURFACEDESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	desc.dwWidth = image->GetWidth();
	desc.dwHeight = image->GetHeight();
	desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
	desc.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
	desc.ddpfPixelFormat.dwRGBBitCount = 8;

	MxS32 i;
	const LegoU8* bits;
	MxU8* surface;

	if (pDirectDraw->CreateSurface(&desc, &textureInfo->m_surface, NULL) != DD_OK) {
		goto done;
	}

	bits = image->GetBits();

	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);

	if (textureInfo->m_surface->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY, NULL) != DD_OK) {
		goto done;
	}

	surface = (MxU8*) desc.lpSurface;
	if (desc.dwWidth == desc.lPitch) {
		memcpy(surface, bits, desc.dwWidth * desc.dwHeight);
	}
	else {
		for (i = 0; i < desc.dwHeight; i++) {
			*(MxU32*) surface = *(MxU32*) bits;
			surface += desc.lPitch;
			bits += desc.dwWidth;
		}
	}

	textureInfo->m_surface->Unlock(desc.lpSurface);

	PALETTEENTRY entries[256];
	memset(entries, 0, sizeof(entries));

	for (i = 0; i < sizeOfArray(entries); i++) {
		if (i < image->GetCount()) {
			entries[i].peFlags = PC_NONE;
			entries[i].peRed = image->GetPalette()->colors[i].r;
			entries[i].peGreen = image->GetPalette()->colors[i].g;
			entries[i].peBlue = image->GetPalette()->colors[i].b;
		}
		else {
			entries[i].peFlags = D3DPAL_RESERVED;
		}
	}

	if (pDirectDraw->CreatePalette(DDPCAPS_ALLOW256 | DDPCAPS_8BIT, entries, &textureInfo->m_palette, NULL) != DD_OK) {
		goto done;
	}

	textureInfo->m_surface->SetPalette(textureInfo->m_palette);

	if (((TglImpl::RendererImpl*) VideoManager()->GetRenderer())
			->CreateTextureFromSurface(textureInfo->m_surface, &textureInfo->m_texture) != D3DRM_OK) {
		goto done;
	}

	textureInfo->m_texture->SetAppData((LPD3DRM_APPDATA) textureInfo);
	return textureInfo;

done:
	if (textureInfo->m_name != NULL) {
		delete[] textureInfo->m_name;
		textureInfo->m_name = NULL;
	}

	if (textureInfo->m_palette != NULL) {
		textureInfo->m_palette->Release();
		textureInfo->m_palette = NULL;
	}

	if (textureInfo->m_surface != NULL) {
		textureInfo->m_surface->Release();
		textureInfo->m_surface = NULL;
	}

	if (textureInfo != NULL) {
		delete textureInfo;
	}

	return NULL;
}

// FUNCTION: LEGO1 0x10065f60
BOOL LegoTextureInfo::SetGroupTexture(Tgl::Mesh* pMesh, LegoTextureInfo* p_textureInfo)
{
	TglImpl::MeshImpl::MeshData* data = ((TglImpl::MeshImpl*) pMesh)->ImplementationData();
	data->groupMesh->SetGroupTexture(data->groupIndex, p_textureInfo->m_texture);
	return TRUE;
}

// FUNCTION: LEGO1 0x10065f90
BOOL LegoTextureInfo::GetGroupTexture(Tgl::Mesh* pMesh, LegoTextureInfo*& p_textureInfo)
{
	TglImpl::MeshImpl::MeshData* data = ((TglImpl::MeshImpl*) pMesh)->ImplementationData();

	IDirect3DRMMesh* mesh = data->groupMesh;
	D3DRMGROUPINDEX id = data->groupIndex;
	LPDIRECT3DRMTEXTURE returnPtr = NULL;
	LPDIRECT3DRMTEXTURE2 texture = NULL;

	if (mesh->GetGroupTexture(id, &returnPtr) == D3DRM_OK) {
		if (returnPtr->QueryInterface(IID_IDirect3DRMTexture2, (LPVOID*) &texture) == D3DRM_OK) {
			p_textureInfo = (LegoTextureInfo*) texture->GetAppData();

			texture->Release();
			returnPtr->Release();
		}

		return TRUE;
	}

	return FALSE;
}

// FUNCTION: LEGO1 0x10066010
LegoResult LegoTextureInfo::LoadBits(const LegoU8* p_bits)
{
	if (m_surface != NULL && m_texture != NULL) {
		DDSURFACEDESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.dwSize = sizeof(desc);

		if (m_surface->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY, NULL) == DD_OK) {
			MxU8* surface = (MxU8*) desc.lpSurface;
			const LegoU8* bits = p_bits;

			if (desc.dwWidth == desc.lPitch) {
				memcpy(desc.lpSurface, p_bits, desc.dwWidth * desc.dwHeight);
			}
			else {
				for (MxS32 i = 0; i < desc.dwHeight; i++) {
					memcpy(surface, bits, desc.dwWidth);
					surface += desc.lPitch;
					bits += desc.dwWidth;
				}
			}

			m_surface->Unlock(desc.lpSurface);
			m_texture->Changed(TRUE, FALSE);
			return SUCCESS;
		}
	}

	return FAILURE;
}
