#include "GameApp.h"
#include <assert.h>
#include "DXTrace.h"
#include <DirectXMathMatrix.inl>
#include <DirectXColors.h>

#include "DDSTextureLoader11.h"
#include "WICTextureLoader11.h"


GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height) :
    D3DApp(hInstance, windowTitle, width, height) ,
	m_CurrMode(ShowMode::WoodCrate) {

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

	if(ImGui::Begin("Texture Mapping")) {
		static int curr_mode_item = static_cast<int> (m_CurrMode);
		const char* mode_strs[] = {
			"Box",
			"Fire Anim"
		};
		if(ImGui::Combo("Mode", &curr_mode_item, mode_strs, ARRAYSIZE(mode_strs))) {
			if(curr_mode_item == 0) {
				m_CurrMode = ShowMode::WoodCrate;

				m_pD3D11DeviceContext->IASetInputLayout(m_pVertexLayout3D.Get());
				auto meshData = Geometry::CreateBox();
				ResetMesh(meshData);
				m_pD3D11DeviceContext->VSSetShader(m_pVertexShader3D.Get(),nullptr, 0);
				m_pD3D11DeviceContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);
				m_pD3D11DeviceContext->PSSetShaderResources(0,1,m_pWoodCrate.GetAddressOf());
			} else {
				m_CurrMode = ShowMode::FireAnim;
				m_CurrFrame = 0;
				m_pD3D11DeviceContext->IASetInputLayout(m_pVertexLayout2D.Get());
				auto meshData = Geometry::Create2DShow();
				ResetMesh(meshData);
				m_pD3D11DeviceContext->VSSetShader(m_pVertexShader2D.Get(), nullptr, 0);
				m_pD3D11DeviceContext->PSSetShader(m_pPixelShader2D.Get(), nullptr, 0);
				m_pD3D11DeviceContext->PSSetShaderResources(0,1,m_pFireAnims[0].GetAddressOf());
			}
		}
	}
	ImGui::End();
	ImGui::Render();

    if (m_CurrMode == ShowMode::WoodCrate) {
        static float phi = 0.0f, theta = 0.0f;
        phi += 0.3f * dt;
        theta += 0.37f * dt;
        DirectX::XMMATRIX W = DirectX::XMMatrixRotationX(phi) * DirectX::XMMatrixRotationY(theta);
        m_VSConstantBuffer.world = DirectX::XMMatrixTranspose(W);
        m_VSConstantBuffer.worldInvTranspose = DirectX::XMMatrixTranspose(InverseTranspose(W));

        D3D11_MAPPED_SUBRESOURCE mappedData;
        HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
        memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
        m_pD3D11DeviceContext->Unmap(m_pConstantBuffer[0].Get(), 0);
    } else if(m_CurrMode == ShowMode::FireAnim) {
		static float totDeltaTime = 0;
		totDeltaTime += dt;
		if(totDeltaTime > 1.0/ 60) {
			totDeltaTime -= 1.0 / 60;
			m_CurrFrame = (m_CurrFrame + 1) % 120;
			m_pD3D11DeviceContext->PSSetShaderResources(0,1,m_pFireAnims[m_CurrFrame].GetAddressOf());
		}
	}
}
void GameApp::DrawScene() {
	assert(m_pD3D11DeviceContext);
	assert(m_pSwapChain);

	m_pD3D11DeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::Black));
	m_pD3D11DeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_pD3D11DeviceContext->DrawIndexed(m_IndexCount, 0, 0);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	HR(m_pSwapChain->Present(0,0));
}

bool GameApp::InitEffect() {
	ComPtr<ID3DBlob> blob;

	HR(CreateShaderFromFile(L"HLSL\\Basic_2D_VS.cso", L"HLSL\\Basic_2D_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pD3D11Device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader2D.GetAddressOf()));
	HR(m_pD3D11Device->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout2D.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\Basic_2D_PS.cso", L"HLSL\\Basic_2D_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pD3D11Device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader2D.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\Basic_3D_VS.cso", L"HLSL\\Basic_3D_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pD3D11Device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader3D.GetAddressOf()));
	HR(m_pD3D11Device->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout3D.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\Basic_3D_PS.cso", L"HLSL\\Basic_3D_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pD3D11Device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader3D.GetAddressOf()));

	return true;
}

bool GameApp::InitResource() {
	auto meshData = Geometry::CreateBox();
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


	HR(DirectX::CreateDDSTextureFromFile(m_pD3D11Device.Get(), L"Texture\\WoodCrate.dds", nullptr, m_pWoodCrate.GetAddressOf()));

	WCHAR strFile[40];
	m_pFireAnims.resize(120);
	for (int i = 1; i <= 120; i++) {
		wsprintf(strFile, L"Texture\\FireAnim\\Fire%03d.bmp", i);
		HR(DirectX::CreateWICTextureFromFile(m_pD3D11Device.Get(), strFile, nullptr, m_pFireAnims[static_cast<size_t>(i) - 1].GetAddressOf()));
	}

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(m_pD3D11Device->CreateSamplerState(&sampDesc,m_pSamplerState.GetAddressOf()));

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

	m_PSConstantBuffer.pointLight[0].position = DirectX::XMFLOAT3(0.0f,0.0f,-10.0f);
	m_PSConstantBuffer.pointLight[0].ambient = DirectX::XMFLOAT4(0.3f,0.3f,0.3f,1.0f);
	m_PSConstantBuffer.pointLight[0].diffuse = DirectX::XMFLOAT4(0.7f,0.7f,0.7f,1.0f);
	m_PSConstantBuffer.pointLight[0].specular = DirectX::XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
	m_PSConstantBuffer.pointLight[0].attr = DirectX::XMFLOAT3(0.0f,0.1f,0.0f);
	m_PSConstantBuffer.pointLight[0].range = 25.0f;
	m_PSConstantBuffer.numDirLight = 0;
	m_PSConstantBuffer.numPointLight = 1;
	m_PSConstantBuffer.numSpotLight = 0;

	m_PSConstantBuffer.material.ambient = DirectX::XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
    m_PSConstantBuffer.material.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_PSConstantBuffer.material.specular = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
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
	m_pD3D11DeviceContext->IASetInputLayout(m_pVertexLayout3D.Get());

	m_pD3D11DeviceContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);

	m_pD3D11DeviceContext->VSSetConstantBuffers(0,1,m_pConstantBuffer[0].GetAddressOf());
	m_pD3D11DeviceContext->PSSetConstantBuffers(1,1,m_pConstantBuffer[1].GetAddressOf());

	m_pD3D11DeviceContext->PSSetSamplers(0,1,m_pSamplerState.GetAddressOf());
	m_pD3D11DeviceContext->PSSetShaderResources(0,1,m_pWoodCrate.GetAddressOf());
	m_pD3D11DeviceContext->PSSetShader(m_pPixelShader3D.Get(),nullptr,0);

	D3D11SetDebugObjectName(m_pVertexLayout2D.Get(), "VertexPosTexLayout");
	D3D11SetDebugObjectName(m_pVertexLayout3D.Get(), "VertexPosNormalTexLayout");
	D3D11SetDebugObjectName(m_pVertexShader2D.Get(), "Basic_2D_VS");
	D3D11SetDebugObjectName(m_pPixelShader2D.Get(), "Basic_2D_PS");
	D3D11SetDebugObjectName(m_pVertexShader3D.Get(), "Basic_3D_VS");
	D3D11SetDebugObjectName(m_pPixelShader3D.Get(), "Basic_3D_PS");
    D3D11SetDebugObjectName(m_pConstantBuffer[0].Get(), "VSConstantBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffer[1].Get(), "PSConstantBuffer");
	D3D11SetDebugObjectName(m_pSamplerState.Get(), "SamplerState");

    return true;
}


template <typename VertexType>
bool GameApp::ResetMesh(const Geometry::MeshData<VertexType>& meshData) {
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd,sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT) meshData.vertexVec.size() * sizeof(VertexType);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData,sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(m_pD3D11Device->CreateBuffer(&vbd,&InitData, m_pVertexBuffer.GetAddressOf()));

	UINT stride = sizeof(VertexType);
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