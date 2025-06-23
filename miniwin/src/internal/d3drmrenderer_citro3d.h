#pragma once

#include "d3drmrenderer.h"
#include "d3drmtexture_impl.h"
#include "ddraw_impl.h"

#include <3ds/gpu/shaderProgram.h>
#include <SDL3/SDL.h>
#include "citro3d.h"
#include <vector>

DEFINE_GUID(Citro3D_GUID, 0x682656F3, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3D, 0x13);

class Citro3DRenderer : public Direct3DRMRenderer {
public:
	static Direct3DRMRenderer* Create(DWORD width, DWORD height);

	// constructor parameters not finalized
	Citro3DRenderer(DWORD width, DWORD height);
	~Citro3DRenderer() override;

	void PushLights(const SceneLight* lightsArray, size_t count) override;
	void SetProjection(const D3DRMMATRIX4D& projection, D3DVALUE front, D3DVALUE back) override;
	void SetFrustumPlanes(const Plane* frustumPlanes) override;
	Uint32 GetTextureId(IDirect3DRMTexture* texture) override;
	Uint32 GetMeshId(IDirect3DRMMesh* mesh, const MeshGroup* meshGroup) override;
	void GetDesc(D3DDEVICEDESC* halDesc, D3DDEVICEDESC* helDesc) override;
	const char* GetName() override;
	HRESULT BeginFrame() override;
	void EnableTransparency() override;
	void SubmitDraw(
		DWORD meshId,
		const D3DRMMATRIX4D& modelViewMatrix,
		const Matrix3x3& normalMatrix,
		const Appearance& appearance
	) override;
	HRESULT FinalizeFrame() override;
	void Resize(int width, int height, const ViewportTransform& viewportTransform) override;
	void Clear(float r, float g, float b) override;
	void Flip() override;
	void Draw2DImage(Uint32 textureId, const SDL_Rect& srcRect, const SDL_Rect& dstRect) override;
	void Download(SDL_Surface* target) override;
private:
	SDL_Surface* m_renderedImage;
	C3D_RenderTarget* m_renderTarget;
	shaderProgram_s* m_shaderProgram;
};

inline static void Citro3DRenderer_EnumDevice(LPD3DENUMDEVICESCALLBACK cb, void* ctx)
{
	Direct3DRMRenderer* device = Citro3DRenderer::Create(640, 480);
	if (device) {
		EnumDevice(cb, ctx, device, Citro3D_GUID);
		delete device;
	}
}
