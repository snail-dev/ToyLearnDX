#include "d3dUtil.h"
#include "DXTrace.h"
#include "Effect.h"
#include "EffectHelper.h"

#include "Vertex.h"

#include <vector>

class BasicEffect::Impl : public AlignedType<BasicEffect::Impl>
{
public:

	struct CBChangesEveryDrawing {
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;
		Material material;
	};

	struct CBDrawingStates {
		int isReflection;
		int isShadow;
		DirectX::XMINT2 pad;
	};

	struct CBChangesEveryFrame {
		DirectX::XMMATRIX view;
		DirectX::XMFLOAT3 eyePos;
		float pad;
	};

	struct CBChangesOnResize {
		DirectX::XMMATRIX proj;
	};

	struct CBChangesRarely {
		DirectX::XMMATRIX reflection;
		DirectX::XMMATRIX shadow;
		DirectX::XMMATRIX refShadow;

		DirectionalLight dirLight[BasicEffect::maxLightCount];
		PointLight pointLight[BasicEffect::maxLightCount];
		SpotLight spotLight[BasicEffect::maxLightCount];
	};

public:
	Impl() : mDirty() {}
	~Impl() = default;

public:

	CBufferObject<0, CBChangesEveryDrawing> m_CBDrawing;
	CBufferObject<1, CBDrawingStates> m_CBStates;
	CBufferObject<2, CBChangesEveryFrame> m_CBFrame;
	CBufferObject<3, CBChangesOnResize> m_CBOnResize;
	CBufferObject<4, CBChangesRarely> m_CBRarely;

	BOOL mDirty;

	std::vector<CBufferBase*> m_pCBuffers;

	ComPtr<ID3D11VertexShader> m_pVertexShader3D;
	ComPtr<ID3D11PixelShader> m_pPixelShader3D;

	ComPtr<ID3D11VertexShader> m_pVertexShader2D;
	ComPtr<ID3D11PixelShader> m_pPixelShader2D;

	ComPtr<ID3D11InputLayout> m_pVertexLayout2D;
	ComPtr<ID3D11InputLayout> m_pVertexLayout3D;

	ComPtr<ID3D11ShaderResourceView> m_srv;

};

namespace {
	static BasicEffect* g_instance = nullptr;
}


BasicEffect::BasicEffect()
{
	if (g_instance)
		throw std::exception("BasicEffect is a singleton");

	g_instance = this;
	pImpl = std::make_unique<BasicEffect::Impl>();
}

BasicEffect::~BasicEffect()
{

}

BasicEffect::BasicEffect(BasicEffect&& moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
}

BasicEffect& BasicEffect::operator=(BasicEffect&& moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
	return *this;
}

BasicEffect& BasicEffect::GetInstance()
{
	if (!g_instance)
		throw std::exception("BasicEffect needs an instance");

	return *g_instance;
}

bool BasicEffect::InitAll(ID3D11Device* device) {
	if (!device)
		return false;

	if (!pImpl->m_pCBuffers.empty())
		return true;

	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	ComPtr<ID3DBlob> blob;

	HR(CreateShaderFromFile(L"HLSL\\Basic_2D_VS.cso", L"HLSL\\Basic_2D_VS.hlsl", "VS", "vs_5_0", blob.GetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pVertexShader2D.GetAddressOf()));
	HR(device->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexLayout2D.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\Basic_3D_VS.cso", L"HLSL\\Basic_3D_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pVertexShader3D.GetAddressOf()));
	HR(device->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexLayout3D.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\Basic_2D_PS.cso", L"HLSL\\Basic_2D_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pPixelShader2D.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\Basic_3D_PS.cso", L"HLSL\\Basic_3D_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pPixelShader3D.GetAddressOf()));



	pImpl->m_pCBuffers.assign({
		&pImpl->m_CBDrawing,
		&pImpl->m_CBFrame,
		&pImpl->m_CBStates,
		&pImpl->m_CBOnResize,
		&pImpl->m_CBRarely
	});

	for (auto& pBuffer : pImpl->m_pCBuffers) {
		HR(pBuffer->CreateBuffer(device));
	}

	D3D11SetDebugObjectName(pImpl->m_pVertexLayout2D.Get(), "VertexPosTexLayout");
	D3D11SetDebugObjectName(pImpl->m_pVertexLayout3D.Get(), "VertexPosNormalTexLayout");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[0]->cbuffer.Get(), "CBDrawing");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[1]->cbuffer.Get(), "CBStates");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[2]->cbuffer.Get(), "CBFrame");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[3]->cbuffer.Get(), "CBOnResize");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[4]->cbuffer.Get(), "CBRarely");
	D3D11SetDebugObjectName(pImpl->m_pVertexShader2D.Get(), "Basic_2D_VS");
	D3D11SetDebugObjectName(pImpl->m_pVertexShader3D.Get(), "Basic_3D_VS");
	D3D11SetDebugObjectName(pImpl->m_pPixelShader2D.Get(), "Basic_2D_PS");
	D3D11SetDebugObjectName(pImpl->m_pPixelShader3D.Get(), "Basic_3D_PS");

	return true;
}


void BasicEffect::SetRenderDefault(ID3D11DeviceContext* device_context) {
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device_context->IASetInputLayout(pImpl->m_pVertexLayout3D.Get());
	device_context->VSSetShader(pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	device_context->RSSetState(nullptr);
	device_context->PSSetShader(pImpl->m_pPixelShader3D.Get(),nullptr,0);
	device_context->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	device_context->OMSetDepthStencilState(nullptr, 0);
	device_context->OMSetBlendState(nullptr, nullptr, 0XFFFFFFFF);
}

void BasicEffect::SetRenderAlphaBlend(ID3D11DeviceContext* device_context) {
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device_context->IASetInputLayout(pImpl->m_pVertexLayout3D.Get());
	device_context->VSSetShader(pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	device_context->RSSetState(RenderStates::RSNoCull.Get());
	device_context->PSSetShader(pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	device_context->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	device_context->OMSetDepthStencilState(nullptr, 0);
	device_context->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetRenderNoDoubleBlend(ID3D11DeviceContext* device_context, UINT stencilRef) {
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device_context->IASetInputLayout(pImpl->m_pVertexLayout3D.Get());
	device_context->VSSetShader(pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	device_context->RSSetState(RenderStates::RSNoCull.Get());
	device_context->PSSetShader(pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	device_context->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	device_context->OMSetDepthStencilState(RenderStates::DSSNoDoubleBlend.Get(), stencilRef);
	device_context->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}
void BasicEffect::SetWriteStencilOnly(ID3D11DeviceContext* device_context, UINT stencilRef) {
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device_context->IASetInputLayout(pImpl->m_pVertexLayout3D.Get());
	device_context->VSSetShader(pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	device_context->RSSetState(nullptr);
	device_context->PSSetShader(pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	device_context->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	device_context->OMSetDepthStencilState(RenderStates::DSSWriteStencil.Get(), stencilRef);
	device_context->OMSetBlendState(RenderStates::BSNoColorWrite.Get(), nullptr, 0xFFFFFFFF);
}
void BasicEffect::SetRenderDefaultWithStencil(ID3D11DeviceContext* device_context, UINT stencilRef) {
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device_context->IASetInputLayout(pImpl->m_pVertexLayout3D.Get());
	device_context->VSSetShader(pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	device_context->RSSetState(RenderStates::RSCullClockWise.Get());
	device_context->PSSetShader(pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	device_context->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	device_context->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), stencilRef);
	device_context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}
void BasicEffect::SetRenderAlphaBlendWithStencil(ID3D11DeviceContext* device_context, UINT stencilRef) {
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device_context->IASetInputLayout(pImpl->m_pVertexLayout3D.Get());
	device_context->VSSetShader(pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	device_context->RSSetState(RenderStates::RSNoCull.Get());
	device_context->PSSetShader(pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	device_context->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	device_context->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), stencilRef);
	device_context->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}
void BasicEffect::Set2DRenderDefault(ID3D11DeviceContext* device_context) {
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device_context->IASetInputLayout(pImpl->m_pVertexLayout2D.Get());
	device_context->VSSetShader(pImpl->m_pVertexShader2D.Get(), nullptr, 0);
	device_context->RSSetState(nullptr);
	device_context->PSSetShader(pImpl->m_pPixelShader2D.Get(), nullptr, 0);
	device_context->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	device_context->OMSetDepthStencilState(nullptr, 0);
	device_context->OMSetBlendState(nullptr, nullptr, 0XFFFFFFFF);
}
void BasicEffect::Set2DRenderAlphaBlend(ID3D11DeviceContext* device_context) {
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device_context->IASetInputLayout(pImpl->m_pVertexLayout2D.Get());
	device_context->VSSetShader(pImpl->m_pVertexShader2D.Get(), nullptr, 0);
	device_context->RSSetState(RenderStates::RSNoCull.Get());
	device_context->PSSetShader(pImpl->m_pPixelShader2D.Get(), nullptr, 0);
	device_context->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	device_context->OMSetDepthStencilState(nullptr, 0);
	device_context->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetWorldMatrix(DirectX::FXMMATRIX W) {
	auto& cBuffer = pImpl->m_CBDrawing;
	cBuffer.data.world = DirectX::XMMatrixTranspose(W);
	cBuffer.data.worldInvTranspose = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranspose(W));
	pImpl->mDirty = cBuffer.isDirty = true;
}
void BasicEffect::SetViewMatrix(DirectX::FXMMATRIX V) {
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.view = DirectX::XMMatrixTranspose(V);
	pImpl->mDirty = cBuffer.isDirty = true;
}
void BasicEffect::SetProjectionMatrix(DirectX::FXMMATRIX P) {
	auto& cBuffer = pImpl->m_CBOnResize;
	cBuffer.data.proj = DirectX::XMMatrixTranspose(P);
	pImpl->mDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetReflectionMatrix(DirectX::FXMMATRIX R) {
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.reflection = DirectX::XMMatrixTranspose(R);
	pImpl->mDirty = cBuffer.isDirty = true;
}
void BasicEffect::SetShadowMatrix(DirectX::FXMMATRIX S) {
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.shadow = DirectX::XMMatrixTranspose(S);
	pImpl->mDirty = cBuffer.isDirty = true;
}
void BasicEffect::SetRefShadowMatrix(DirectX::FXMMATRIX RefS) {
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.refShadow = DirectX::XMMatrixTranspose(RefS);
	pImpl->mDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetDirLight(size_t pos, const DirectionalLight& dirLight) {
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.dirLight[pos] = dirLight;
	cBuffer.isDirty = pImpl->mDirty = true;
}
void BasicEffect::SetPointLight(size_t pos, const PointLight& pointLight) {
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.pointLight[pos] = pointLight;
	cBuffer.isDirty = pImpl->mDirty = true;
}
void BasicEffect::SetSpotLight(size_t pos, const SpotLight& spotLight) {
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.spotLight[pos] = spotLight;
	cBuffer.isDirty = pImpl->mDirty = true;
}

void BasicEffect::SetMaterial(const Material& material) {
	auto& cBuffer = pImpl->m_CBDrawing;
	cBuffer.data.material = material;
	cBuffer.isDirty = pImpl->mDirty = true;
}

void BasicEffect::SetSRV(ID3D11ShaderResourceView* srv) {
	pImpl->m_srv = srv;
}

void BasicEffect::SetEyePos(const DirectX::XMFLOAT3& eyePos) {
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.eyePos = eyePos;
	cBuffer.isDirty = pImpl->mDirty = true;
}

void BasicEffect::SetReflectionState(bool isOn) {
	auto& cBuffer = pImpl->m_CBStates;
	cBuffer.data.isReflection = isOn;
	cBuffer.isDirty = pImpl->mDirty = true;

}
void BasicEffect::SetShadowState(bool isOn) {
	auto& cBuffer = pImpl->m_CBStates;
	cBuffer.data.isShadow = isOn;
	cBuffer.isDirty = pImpl->mDirty = true;
}

void BasicEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	auto& cbuffers = pImpl->m_pCBuffers;

	cbuffers[0]->BindVS(deviceContext);
	cbuffers[1]->BindVS(deviceContext);
	cbuffers[2]->BindVS(deviceContext);
	cbuffers[3]->BindVS(deviceContext);
	cbuffers[4]->BindVS(deviceContext);

	cbuffers[0]->BindPS(deviceContext);
	cbuffers[1]->BindPS(deviceContext);
	cbuffers[2]->BindPS(deviceContext);
	cbuffers[3]->BindPS(deviceContext);
	cbuffers[4]->BindPS(deviceContext);

	deviceContext->PSSetShaderResources(0, 1, pImpl->m_srv.GetAddressOf());

	if (pImpl->mDirty) {
		pImpl->mDirty = false;

		for (auto& pCBuffer : cbuffers) {
			pCBuffer->UpdateBuffer(deviceContext);
		}
	}
}