#include "GameApp.h"
#include <assert.h>
#include "DXTrace.h"

const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};


GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height) :
    D3DApp(hInstance, windowTitle, width, height) {

	}

GameApp::~GameApp() {

}

bool GameApp::Init() {
	return D3DApp::Init() && InitEffect() && InitResource();
}
void GameApp::OnResize() {
	D3DApp::OnResize();
}
void GameApp::UpdateScene(float dt) {
}
void GameApp::DrawScene() {
	assert(m_pD3D11DeviceContext);
	assert(m_pSwapChain);

	static float black[4] = {0.0f,0.0f,0.0f,1.0f};
	m_pD3D11DeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), black);
	m_pD3D11DeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pD3D11DeviceContext->Draw(3,0);
	HR(m_pSwapChain->Present(0,0));
}

bool GameApp::InitEffect() {
	ComPtr<ID3DBlob> blob;

	HR(CreateShaderFromFile(L"HLSL\\Triangle_VS.cso", L"HLSL\\Triangle_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pD3D11Device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
	HR(m_pD3D11Device->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\Triangle_PS.cso", L"HLSL\\Triangle_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pD3D11Device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

	return true;
}

bool GameApp::InitResource() {
    VertexPosColor vertices[] = {
            {XMFLOAT3(0.0f, 0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
            {XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
            {XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
    };

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(vertices);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(m_pD3D11Device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));
}