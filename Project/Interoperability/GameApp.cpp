#include "GameApp.h"
#include <assert.h>
#include "DXTrace.h"
#include <DirectXMathMatrix.inl>
#include <DirectXColors.h>

#include "Keyboard.h"


GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height) :
    D3DApp(hInstance, windowTitle, width, height) {

	}

GameApp::~GameApp() {

}

bool GameApp::Init() {
	return D3DApp::Init() && InitEffect() && InitResource() && InitMouse();
}
void GameApp::OnResize() {

	assert(m_pD2DFactory);
	assert(m_pDWriteFactory);

	m_pColorBrush.Reset();
	m_pD2DRenderTarget.Reset();
	D3DApp::OnResize();

	ComPtr<IDXGISurface> surface;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf())));
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	HRESULT hr = m_pD2DFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, m_pD2DRenderTarget.GetAddressOf());
	surface.Reset();

	if(hr == E_NOINTERFACE) {
		OutputDebugStringW(L"\n警告: ");
	}
	else if(hr == S_OK) {
		HR(m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), m_pColorBrush.GetAddressOf()));
		HR(m_pDWriteFactory->CreateTextFormat(L"宋体", nullptr, DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20, L"zh-cn", m_pTextFormat.GetAddressOf()));
	} else {
		assert(m_pD2DRenderTarget);
	}
}
void GameApp::UpdateScene(float dt) {

	static float phi = 0.0f, theta = 0.0f;
	phi += 0.3f * dt;
	theta += 0.37f * dt;

	DirectX::XMMATRIX W = DirectX::XMMatrixRotationX(phi) * DirectX::XMMatrixRotationY(theta);
	m_VSConstantBuffer.world = DirectX::XMMatrixTranspose(W);
	m_VSConstantBuffer.worldInvTranspose = DirectX::XMMatrixTranspose(InverseTranspose(W));

	using Keyboard = DirectX::Keyboard;
	Keyboard::State state = m_pKeyboard->GetState();
	m_KeyboardTracker.Update(state);
	if(m_KeyboardTracker.IsKeyPressed(Keyboard::D1)) {
		m_PSConstantBuffer.dirLight = m_DirLight;
		m_PSConstantBuffer.pointLight = PointLight();
		m_PSConstantBuffer.spotLight = SpotLight();
	}
	else if(m_KeyboardTracker.IsKeyPressed(Keyboard::D2)) {
		m_PSConstantBuffer.dirLight = DirectionalLight();
		m_PSConstantBuffer.pointLight = m_PointLight;
		m_PSConstantBuffer.spotLight = SpotLight();
    } else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D3)) {
        m_PSConstantBuffer.dirLight = DirectionalLight();
        m_PSConstantBuffer.pointLight = PointLight();
        m_PSConstantBuffer.spotLight = m_SpotLight;
    }

	if(m_KeyboardTracker.IsKeyPressed(Keyboard::Q)) {
		auto meshData = Geometry::CreateBox<VertexPosNormalColor>();
		ResetMesh(meshData);
    } else if (m_KeyboardTracker.IsKeyPressed(Keyboard::W)) {
        auto meshData = Geometry::CreateSphere<VertexPosNormalColor>();
        ResetMesh(meshData);
    } else if (m_KeyboardTracker.IsKeyPressed(Keyboard::E)) {
        auto meshData = Geometry::CreateCylinder<VertexPosNormalColor>();
        ResetMesh(meshData);
    } else if (m_KeyboardTracker.IsKeyPressed(Keyboard::R)) {
        auto meshData = Geometry::CreateCone<VertexPosNormalColor>();
        ResetMesh(meshData);
    } else if(m_KeyboardTracker.IsKeyPressed(Keyboard::S)) {
		m_IsWireframeMode = !m_IsWireframeMode;
		m_pD3D11DeviceContext->RSSetState(m_IsWireframeMode ? m_pRSWireframe.Get() : nullptr);
	}

    D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer[0].Get(),0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData,sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
	m_pD3D11DeviceContext->Unmap(m_pConstantBuffer[0].Get(),0);

	HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer[1].Get(),0,D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData,sizeof(PSConstantBuffer),&m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pD3D11DeviceContext->Unmap(m_pConstantBuffer[1].Get(),0);

}
void GameApp::DrawScene() {
	assert(m_pD3D11DeviceContext);
	assert(m_pSwapChain);

	m_pD3D11DeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::Black));
	m_pD3D11DeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_pD3D11DeviceContext->DrawIndexed(m_IndexCount, 0, 0);

	if(m_pD2DRenderTarget) {
		m_pD2DRenderTarget->BeginDraw();
		std::wstring textStr = L"切换灯光类型: 1--平行光 2--点光 3--聚光灯\n"
			L"切换模型: Q--立方体 W--球体 E--圆柱体 R--圆锥体\n"
			L"S-切换模式  当前模式：";
		if(m_IsWireframeMode) {
			textStr += L"线框模式";
		} else {
			textStr += L"面模式";
		}
		m_pD2DRenderTarget->DrawText(textStr.c_str(), (UINT)textStr.size(), m_pTextFormat.Get(), D2D1_RECT_F{ 0.0f,0.0f,600.0f,200.0f }, m_pColorBrush.Get());
		m_pD2DRenderTarget->EndDraw();
	}

	HR(m_pSwapChain->Present(0,0));
}

bool GameApp::InitEffect() {
	ComPtr<ID3DBlob> blob;

	HR(CreateShaderFromFile(L"HLSL\\Light_VS.cso", L"HLSL\\Light_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pD3D11Device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
	HR(m_pD3D11Device->CreateInputLayout(VertexPosNormalColor::inputLayout, ARRAYSIZE(VertexPosNormalColor::inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\Light_PS.cso", L"HLSL\\Light_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pD3D11Device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

	return true;
}

bool GameApp::InitMouse() {
	m_pMouse->SetWindow(m_mainWnd);
	m_pMouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

	return true;
}

bool GameApp::InitResource() {
	auto meshData = Geometry::CreateBox<VertexPosNormalColor>();
	ResetMesh(meshData);
	
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd,sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(VSConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HR(m_pD3D11Device->CreateBuffer(&cbd,nullptr, m_pConstantBuffer[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(PSConstantBuffer);
	HR(m_pD3D11Device->CreateBuffer(&cbd,nullptr, m_pConstantBuffer[1].GetAddressOf()));


	m_DirLight.ambient = DirectX::XMFLOAT4(0.2f,0.2f,0.2f,1.0f);
	m_DirLight.diffuse = DirectX::XMFLOAT4(0.8f,0.8f,0.8f,1.0f);
	m_DirLight.specular = DirectX::XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
	m_DirLight.direction = DirectX::XMFLOAT3(-0.577f, -0.577f, 0.577f);

	m_PointLight.position = DirectX::XMFLOAT3(0.0f,0.0f,-10.0f);
    m_PointLight.ambient = DirectX::XMFLOAT4(0.3f,0.3f,0.3f,1.0f);
    m_PointLight.diffuse = DirectX::XMFLOAT4(0.7f,0.7f,0.7f,1.0f);
    m_PointLight.specular = DirectX::XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
	m_PointLight.attr = DirectX::XMFLOAT3(0.0f,0.1f,0.0f);
	m_PointLight.range = 25.0f;

    m_SpotLight.position = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f);
    m_SpotLight.direction = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
    m_SpotLight.ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_SpotLight.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_SpotLight.specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_SpotLight.attr = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_SpotLight.spot = 12.0f;
    m_SpotLight.range = 10000.0f;

	m_VSConstantBuffer.world = DirectX::XMMatrixIdentity();
    m_VSConstantBuffer.view = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f,0.0f,-5.0f,0.0f), DirectX::XMVectorSet(0.0f,0.0f,0.0f,0.0f), DirectX::XMVectorSet(0.0f,1.0f,0.0f,0.0f)));
	
	m_VSConstantBuffer.proj = DirectX::XMMatrixTranspose(
		DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f)
	);
	m_VSConstantBuffer.worldInvTranspose = DirectX::XMMatrixIdentity();

	m_PSConstantBuffer.material.ambient = DirectX::XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
    m_PSConstantBuffer.material.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_PSConstantBuffer.material.specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_PSConstantBuffer.dirLight = m_DirLight;
	m_PSConstantBuffer.eyePos = DirectX::XMFLOAT4(0.0f,0.0f,-5.0f,0.0f);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer[1].Get(),0,D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pD3D11DeviceContext->Unmap(m_pConstantBuffer[1].Get(),0);


	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;

	HR(m_pD3D11Device->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));

    m_pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pD3D11DeviceContext->IASetInputLayout(m_pVertexLayout.Get());

	m_pD3D11DeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	m_pD3D11DeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

	m_pD3D11DeviceContext->VSSetConstantBuffers(0,1,m_pConstantBuffer[0].GetAddressOf());
	m_pD3D11DeviceContext->PSSetConstantBuffers(1,1,m_pConstantBuffer[1].GetAddressOf());

	D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
	D3D11SetDebugObjectName(m_pVertexShader.Get(), "Light_VS");
	D3D11SetDebugObjectName(m_pPixelShader.Get(), "Light_PS");
    D3D11SetDebugObjectName(m_pConstantBuffer[0].Get(), "VSConstantBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffer[1].Get(), "PSConstantBuffer");

    return true;
}

bool GameApp::ResetMesh(const Geometry::MeshData<VertexPosNormalColor>& meshData) {
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd,sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT) meshData.vertexVec.size() * sizeof(VertexPosNormalColor);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData,sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(m_pD3D11Device->CreateBuffer(&vbd,&InitData, m_pVertexBuffer.GetAddressOf()));

	UINT stride = sizeof(VertexPosNormalColor);
	UINT offset = 0;
	m_pD3D11DeviceContext->IASetVertexBuffers(0,1,m_pVertexBuffer.GetAddressOf(),&stride,&offset);

	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd,sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = (UINT) meshData.indexVec.size() * sizeof(DWORD);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	
	InitData.pSysMem = meshData.indexVec.data();
	HR(m_pD3D11Device->CreateBuffer(&ibd,&InitData, m_pIndexBuffer.GetAddressOf()));
	m_pD3D11DeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");

	return true;

}