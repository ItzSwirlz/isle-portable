#include "SDL3/SDL_surface.h"
#include "d3drmrenderer_citro3d.h"
#include "miniwin.h"
#include "miniwin/d3d.h"
#include "miniwin/d3drm.h"
#include "miniwin/windows.h"
#include "citro3d.h"

Direct3DRMRenderer* Citro3DRenderer::Create(DWORD width, DWORD height)
{
	// TODO: Doesn't SDL call this function?
	gfxInitDefault();

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	return new Citro3DRenderer(width, height);
}

// constructor parameters not finalized
Citro3DRenderer::Citro3DRenderer(DWORD width, DWORD height)
{
	m_width = width;
	m_height = height;

	// FIXME: is this the right pixel format?
	m_renderedImage = SDL_CreateSurface(m_width, m_height, SDL_PIXELFORMAT_RGBA32);

	// TODO: is GPU_RB_RGBA8 correct?
	// TODO: is GPU_RB_DEPTH24_STENCIL8 correct?
	m_renderTarget = C3D_RenderTargetCreate(width, height, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);

	MINIWIN_NOT_IMPLEMENTED();
}

Citro3DRenderer::~Citro3DRenderer()
{
	SDL_DestroySurface(m_renderedImage);
	C3D_RenderTargetDelete(m_renderTarget);
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

void Citro3DRenderer::GetDesc(D3DDEVICEDESC* halDesc, D3DDEVICEDESC* helDesc)
{
	// not sure if this is correct?
	MINIWIN_NOT_IMPLEMENTED();

	halDesc->dcmColorModel = D3DCOLORMODEL::RGB;
	helDesc->dwFlags = D3DDD_DEVICEZBUFFERBITDEPTH;
	helDesc->dwDeviceZBufferBitDepth = DDBD_24;
	helDesc->dwDeviceRenderBitDepth = DDBD_24;
	helDesc->dpcTriCaps.dwTextureCaps = D3DPTEXTURECAPS_PERSPECTIVE;
	helDesc->dpcTriCaps.dwShadeCaps = D3DPSHADECAPS_ALPHAFLATBLEND;

	// TODO: shouldn't this be bilinear
	helDesc->dpcTriCaps.dwTextureFilterCaps = D3DPTFILTERCAPS_LINEAR;

	memset(helDesc, 0, sizeof(D3DDEVICEDESC));
}

const char* Citro3DRenderer::GetName()
{
	return "Citro3D";
}

HRESULT Citro3DRenderer::BeginFrame()
{
	MINIWIN_NOT_IMPLEMENTED();
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	return S_OK;
}

void Citro3DRenderer::EnableTransparency()
{
	MINIWIN_NOT_IMPLEMENTED();
}

void Citro3DRenderer::SubmitDraw(
	DWORD meshId,
	const D3DRMMATRIX4D& modelViewMatrix,
	const Matrix3x3& normalMatrix,
	const Appearance& appearance
)
{
	MINIWIN_NOT_IMPLEMENTED();
}

HRESULT Citro3DRenderer::FinalizeFrame()
{
	MINIWIN_NOT_IMPLEMENTED();
	C3D_FrameEnd(0);
	return S_OK;
}

void Citro3DRenderer::Resize(int width, int height, const ViewportTransform& viewportTransform)
{
	MINIWIN_NOT_IMPLEMENTED();
	m_width = width;
	m_height = height;
	m_viewportTransform = viewportTransform;

	SDL_DestroySurface(m_renderedImage);
	// FIXME: is this the right pixel format?
	m_renderedImage = SDL_CreateSurface(m_width, m_height, SDL_PIXELFORMAT_RGBA32);
}

void Citro3DRenderer::Clear(float r, float g, float b)
{
	MINIWIN_NOT_IMPLEMENTED();
}

void Citro3DRenderer::Flip()
{
	MINIWIN_NOT_IMPLEMENTED();
}

void Citro3DRenderer::Draw2DImage(Uint32 textureId, const SDL_Rect& srcRect, const SDL_Rect& dstRect)
{
	MINIWIN_NOT_IMPLEMENTED();
	float left = -m_viewportTransform.offsetX / m_viewportTransform.scale;
	float right = (m_width - m_viewportTransform.offsetX) / m_viewportTransform.scale;
	float top = -m_viewportTransform.offsetY / m_viewportTransform.scale;
	float bottom = (m_height - m_viewportTransform.offsetY) / m_viewportTransform.scale;

	C3D_Mtx mtx;

	// TODO: isLeftHanded set to false. Should it be true?
	Mtx_OrthoTilt(&mtx, left, right, bottom, top, -1, 1, false);
}

void Citro3DRenderer::Download(SDL_Surface* target)
{
	MINIWIN_NOT_IMPLEMENTED();
}
