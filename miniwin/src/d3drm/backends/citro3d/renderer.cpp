#include "d3drmrenderer_citro3d.h"
#include "miniwin.h"
#include "miniwin/d3drm.h"
#include "miniwin/windows.h"

Direct3DRMRenderer* Citro3DRenderer::Create(DWORD width, DWORD height)
{
	MINIWIN_NOT_IMPLEMENTED();
	return new Citro3DRenderer(width, height);
}

// constructor parameters not finalized
Citro3DRenderer::Citro3DRenderer(DWORD width, DWORD height)
{
	MINIWIN_NOT_IMPLEMENTED();
}

Citro3DRenderer::~Citro3DRenderer()
{
	MINIWIN_NOT_IMPLEMENTED();
}

void Citro3DRenderer::PushLights(const SceneLight* lightsArray, size_t count)
{
	MINIWIN_NOT_IMPLEMENTED();
}

void Citro3DRenderer::SetProjection(const D3DRMMATRIX4D& projection, D3DVALUE front, D3DVALUE back)
{
	MINIWIN_NOT_IMPLEMENTED();
}

void Citro3DRenderer::SetFrustumPlanes(const Plane* frustumPlanes)
{
	MINIWIN_NOT_IMPLEMENTED();
}

Uint32 Citro3DRenderer::GetTextureId(IDirect3DRMTexture* texture)
{
	MINIWIN_NOT_IMPLEMENTED();
	return 0;
}

Uint32 Citro3DRenderer::GetMeshId(IDirect3DRMMesh* mesh, const MeshGroup* meshGroup)
{
	MINIWIN_NOT_IMPLEMENTED();
	return 0;
}

DWORD Citro3DRenderer::GetWidth()
{
	MINIWIN_NOT_IMPLEMENTED();
	return 0;
}

DWORD Citro3DRenderer::GetHeight()
{
	MINIWIN_NOT_IMPLEMENTED();
	return 0;
}

void Citro3DRenderer::GetDesc(D3DDEVICEDESC* halDesc, D3DDEVICEDESC* helDesc)
{
	MINIWIN_NOT_IMPLEMENTED();
}

const char* Citro3DRenderer::GetName()
{
	return "Citro3D";
}

HRESULT Citro3DRenderer::BeginFrame()
{
	MINIWIN_NOT_IMPLEMENTED();
	return S_OK;
}

void Citro3DRenderer::EnableTransparency()
{
	MINIWIN_NOT_IMPLEMENTED();
}

void Citro3DRenderer::SubmitDraw(DWORD meshId, const D3DRMMATRIX4D& modelViewMatrix, const Matrix3x3& normalMatrix, const Appearance& appearance)
{
	MINIWIN_NOT_IMPLEMENTED();
}

HRESULT Citro3DRenderer::FinalizeFrame()
{
	MINIWIN_NOT_IMPLEMENTED();
	return S_OK;
}
