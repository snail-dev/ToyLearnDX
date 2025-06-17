#include "GameApp.h"
#include <assert.h>
#include "DXTrace.h"
#include <DirectXMathMatrix.inl>

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

	static float phi = 0.0f, theta = 0.0f;
	phi += 0.3f * dt, theta += 0.37f * dt;
	m_CBuffer.world = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationX(phi) * DirectX::XMMatrixRotationY(theta) * DirectX::XMMatrixTranslation(1.f, 1.f, 0.f));

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&mappedData));
	memcpy_s(mappedData.pData, sizeof(m_CBuffer),&m_CBuffer, sizeof(m_CBuffer));
	m_pD3D11DeviceContext->Unmap(m_pConstantBuffer.Get(), 0);

}
void GameApp::DrawScene() {
	assert(m_pD3D11DeviceContext);
	assert(m_pSwapChain);

	static float black[4] = {0.0f,0.0f,0.0f,1.0f};
	m_pD3D11DeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), black);
	m_pD3D11DeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pD3D11DeviceContext->DrawIndexed(36, 0, 0);
	HR(m_pSwapChain->Present(0,0));
}

bool GameApp::InitEffect() {
	ComPtr<ID3DBlob> blob;

	HR(CreateShaderFromFile(L"HLSL\\Cube_VS.cso", L"HLSL\\Cube_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pD3D11Device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
	HR(m_pD3D11Device->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\Cube_PS.cso", L"HLSL\\Cube_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pD3D11Device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

	return true;
}

bool GameApp::InitResource() {
    VertexPosColor vertices[] = {
            {DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)},
            {DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
            {DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},
            {DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
            {DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
            {DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},
            {DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)},
            {DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)}
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


	DWORD indices[] = {
		0,1,2,
		2,3,0,
		4,5,1,
		1,0,4,
		1,5,6,
		6,2,1,
		7,6,5,
		5,4,7,
		3,2,6,
		6,7,3,
		4,0,3,
		3,7,4
	};

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd,sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(indices);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	HR(m_pD3D11Device->CreateBuffer(&ibd,&InitData, m_pIndexBuffer.GetAddressOf()));

	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd,sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HR(m_pD3D11Device->CreateBuffer(&cbd,nullptr,m_pConstantBuffer.GetAddressOf()));

	m_CBuffer.world = DirectX::XMMatrixIdentity();
	m_CBuffer.view = DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(0.0f,0.0f,-5.0f, 0.0f),
			DirectX::XMVectorSet(0.0f,0.0f,0.0f,0.0f),
			DirectX::XMVectorSet(0.0f,1.0f,0.0f,0.0f)
		)
	);

	m_CBuffer.proj = DirectX::XMMatrixTranspose(
		DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, AspectRatio(),1.0f, 100.0f)
	);


	UINT stride = sizeof(VertexPosColor);
	UINT offset = 0;

	m_pD3D11DeviceContext->IASetVertexBuffers(0,1,m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    m_pD3D11DeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pD3D11DeviceContext->IASetInputLayout(m_pVertexLayout.Get());
	m_pD3D11DeviceContext->VSSetConstantBuffers(0,1,m_pConstantBuffer.GetAddressOf());
	m_pD3D11DeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	m_pD3D11DeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

	D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");
	D3D11SetDebugObjectName(m_pConstantBuffer.Get(), "ConstantBuffer");
	D3D11SetDebugObjectName(m_pVertexShader.Get(), "Cube_VS");
	D3D11SetDebugObjectName(m_pPixelShader.Get(), "Cube_PS");

	return true;
}