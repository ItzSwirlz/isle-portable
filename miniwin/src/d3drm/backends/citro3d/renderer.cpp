#include "SDL3/SDL_surface.h"
#include "d3drmrenderer_citro3d.h"
#include "miniwin.h"
#include "miniwin/d3d.h"
#include "miniwin/d3drm.h"
#include "miniwin/windows.h"

#include <3ds/console.h>
#include <3ds/gfx.h>
#include <3ds/gpu/gx.h>
#include <3ds/gpu/shaderProgram.h>
#include <c3d/framebuffer.h>
#include <citro3d.h>
#include <3ds.h>

#include "vshader_shbin.h"

int m_projectionShaderUniformLocation;

void sceneInit(shaderProgram_s* prog) {
	m_projectionShaderUniformLocation = shaderInstanceGetUniformLocation(prog->vertexShader, "projection");
}

Direct3DRMRenderer* Citro3DRenderer::Create(DWORD width, DWORD height)
{
	// TODO: Doesn't SDL call this function?
	gfxInitDefault();
	gfxSet3D(false);
	consoleInit(GFX_BOTTOM, nullptr);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	return new Citro3DRenderer(width, height);
}

// constructor parameters not finalized
Citro3DRenderer::Citro3DRenderer(DWORD width, DWORD height)
{
	SDL_Log("Hello from ctor");
	shaderProgram_s program;
	DVLB_s *vsh_dvlb;
	m_width = width;
	m_height = height;

	// FIXME: is this the right pixel format?

	SDL_Log("Pre shader program init");
	shaderProgramInit(&program);
	SDL_Log("Pre parse file");
	vsh_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
	SDL_Log("pre program set vsh");
	shaderProgramSetVsh(&program, &vsh_dvlb->DVLE[0]);

	// WARNING: This might crash, not sure
	SDL_Log("pre bind");
	C3D_BindProgram(&program);

	// todo: move to scene init next
	SDL_Log("setting uniform loc");
	sceneInit(&program);

	// TODO: is GPU_RB_RGBA8 correct?
	// TODO: is GPU_RB_DEPTH24_STENCIL8 correct?
	SDL_Log("Pre render target create");
	m_renderTarget = C3D_RenderTargetCreate(width, height, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);

	SDL_Log("render clear");
	// TODO: what color should be used, if we shouldn't use 0x777777FF
	C3D_RenderTargetClear(m_renderTarget, C3D_CLEAR_ALL, 0x777777FF, 0);

	// TODO: Cleanup as we see what is needed
	m_flipVertFlag = 0;
	m_outTiledFlag = 0;
	m_rawCopyFlag = 0;

	// TODO: correct values?
	m_transferInputFormatFlag = GX_TRANSFER_FMT_RGBA8;
	m_transferOutputFormatFlag = GX_TRANSFER_FMT_RGB8;

	m_transferScaleFlag = GX_TRANSFER_SCALE_NO;

	m_transferFlags = (GX_TRANSFER_FLIP_VERT(m_flipVertFlag) | GX_TRANSFER_OUT_TILED(m_outTiledFlag) | \
		GX_TRANSFER_RAW_COPY(m_rawCopyFlag) | GX_TRANSFER_IN_FORMAT(m_transferInputFormatFlag) | \
		GX_TRANSFER_OUT_FORMAT(m_transferOutputFormatFlag) | GX_TRANSFER_SCALING(m_transferScaleFlag));

	SDL_Log("render set out");
	C3D_RenderTargetSetOutput(m_renderTarget, GFX_TOP, GFX_LEFT, m_transferFlags);

	SDL_Log("Pre create surface");
	m_renderedImage = SDL_CreateSurface(m_width, m_height, SDL_PIXELFORMAT_RGBA32);
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
	MINIWIN_TRACE("Set projection");
	memcpy(&m_projection, projection, sizeof(D3DRMMATRIX4D));
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
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank(); // FIXME: is this the right place to call, if we should at all?
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C3D_FrameDrawOn(m_renderTarget);
	return S_OK;
}

void Citro3DRenderer::EnableTransparency()
{
	MINIWIN_NOT_IMPLEMENTED();
}

void Citro3DRenderer::SubmitDraw(
	DWORD meshId,
	const D3DRMMATRIX4D& modelViewMatrix,
	const D3DRMMATRIX4D& worldMatrix,
	const D3DRMMATRIX4D& viewMatrix,
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
	MINIWIN_TRACE("on draw 2d image");
	float left = -m_viewportTransform.offsetX / m_viewportTransform.scale;
	float right = (m_width - m_viewportTransform.offsetX) / m_viewportTransform.scale;
	float top = -m_viewportTransform.offsetY / m_viewportTransform.scale;
	float bottom = (m_height - m_viewportTransform.offsetY) / m_viewportTransform.scale;

	C3D_Mtx mtx;

	// TODO: isLeftHanded set to false. Should it be true?
	MINIWIN_TRACE("pre orthotilt");
	Mtx_OrthoTilt(&mtx, left, right, bottom, top, -1, 1, false);

	MINIWIN_TRACE("pre fvunifmtx4x4");
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, m_projectionShaderUniformLocation, &mtx);
}

void Citro3DRenderer::Download(SDL_Surface* target)
{
	MINIWIN_NOT_IMPLEMENTED();
}
