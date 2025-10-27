#ifndef _EFFECT_H_
#define _EFFECT_H_

#include <memory>
#include "LightHelper.h"
#include "RenderStates.h"

class IEffect {
public:
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;


	IEffect() = default;
	virtual ~IEffect() = default;

	IEffect(const IEffect&) = delete;
	IEffect& operator=(const IEffect&) = delete;

	IEffect(IEffect&&) = default;
	IEffect& operator=(IEffect&&) = default;

	virtual void Apply(ID3D11DeviceContext* deviceContext) = 0;
};


class BasicEffect : public IEffect {
public:
	BasicEffect();
	virtual ~BasicEffect() override;

	BasicEffect(BasicEffect&& moveFrom) noexcept;
	BasicEffect& operator= (BasicEffect&& moveFrom) noexcept;

	static BasicEffect& GetInstance();


	bool InitAll(ID3D11Device* device);

	void SetRenderDefault(ID3D11DeviceContext* device_context);
	void SetRenderAlphaBlend(ID3D11DeviceContext* device_context);
	void SetRenderNoDoubleBlend(ID3D11DeviceContext* device_context, UINT stencilRef);
	void SetWriteStencilOnly(ID3D11DeviceContext* device_context, UINT stencilRef);
	void SetRenderDefaultWithStencil(ID3D11DeviceContext* device_context, UINT stencilRef);
	void SetRenderAlphaBlendWithStencil(ID3D11DeviceContext* device_context, UINT stencilRef);
	void Set2DRenderDefault(ID3D11DeviceContext* device_context);
	void Set2DRenderAlphaBlend(ID3D11DeviceContext* device_context);

	void SetDrawBoltAnimNoDepthWriteWithStencil(ID3D11DeviceContext* device_context, UINT stencilRef);
	void SetDrawBoltAnimNoDepthWrite(ID3D11DeviceContext* device_context);


	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W);
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V);
	void XM_CALLCONV SetProjectionMatrix(DirectX::FXMMATRIX P);

	void XM_CALLCONV SetReflectionMatrix(DirectX::FXMMATRIX R);
	void XM_CALLCONV SetShadowMatrix(DirectX::FXMMATRIX S);
	void XM_CALLCONV SetRefShadowMatrix(DirectX::FXMMATRIX RefS);

	static const int maxLightCount = 5;

	void SetDirLight(size_t pos, const DirectionalLight& dirLight);
	void SetPointLight(size_t pos, const PointLight& pointLight);
	void SetSpotLight(size_t pos, const SpotLight& spotLight);

	void SetMaterial(const Material& material);

	void SetSRV(ID3D11ShaderResourceView* srv);

	void SetEyePos(const DirectX::XMFLOAT3&  eyePos);


	void SetReflectionState(bool isOn);
	void SetShadowState(bool isOn);

	void Apply(ID3D11DeviceContext* deviceContext);

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};



#endif // _EFFECT_H_