#include "GameApp.h"
#include <assert.h>
#include "DXTrace.h"
#include <DirectXMathMatrix.inl>
#include <DirectXColors.h>


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
	phi += 0.3f * dt;
	theta += 0.37f * dt;

	DirectX::XMMATRIX W = DirectX::XMMatrixRotationX(phi) * DirectX::XMMatrixRotationY(theta);
	m_VSConstantBuffer.world = DirectX::XMMatrixTranspose(W);
	m_VSConstantBuffer.worldInvTranspose = DirectX::XMMatrixTranspose(InverseTranspose(W));

	if(ImGui::Begin("Lighting")) {
		static int curr_mesh_item = 0;
		const char* mesh_strs[] = {
			"Box",
			"Sphere",
			"Cylinder",
			"Cone",
			"Plane",
			"Terrain"
		};

		if(ImGui::Combo("Mesh", &curr_mesh_item, mesh_strs, ARRAYSIZE(mesh_strs))) {
			Geometry::MeshData<VertexPosNormalColor> meshData;
			switch(curr_mesh_item) {
				case 0: meshData = Geometry::CreateBox<VertexPosNormalColor>();break;
				case 1: meshData = Geometry::CreateSphere<VertexPosNormalColor>();break;
				case 2: meshData = Geometry::CreateCylinder<VertexPosNormalColor>(); break;
				case 3: meshData = Geometry::CreateCone<VertexPosNormalColor>(); break;
				case 4: meshData = Geometry::CreatePlane<VertexPosNormalColor>(); break;
				case 5: meshData = Geometry::CreateTerrain<VertexPosNormalColor>(); break;
			}
			ResetMesh(meshData);
		}

		bool mat_changed = false;
		ImGui::Text("Material");
		ImGui::PushID(3);
		ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.material.ambient.x);
		ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.material.diffuse.x);
		ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.material.specular.x);
		ImGui::PopID();

		static int curr_light_item = 0;
		static const char* light_modes[] = {
			"Directional Light",
			"Point Light",
			"Spot Light"
		};
		ImGui::Text("Light");
		if(ImGui::Combo("Light Type", & curr_light_item, light_modes, ARRAYSIZE(light_modes))) {
			m_PSConstantBuffer.dirLight = (curr_light_item == 0 ? m_DirLight  : DirectionalLight());
			m_PSConstantBuffer.pointLight = (curr_light_item == 1 ? m_PointLight : PointLight());
			m_PSConstantBuffer.spotLight = (curr_light_item == 2 ? m_SpotLight  : SpotLight());
		}

		bool light_changed = false;
		ImGui::PushID(curr_light_item);
		if(curr_light_item == 0) {
			ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.dirLight.ambient.x);
			ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.dirLight.diffuse.x);
			ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.dirLight.specular.x);
		} else if(curr_light_item == 1) {
            ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.pointLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.pointLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.pointLight.specular.x);
            ImGui::InputFloat("Range", &m_PSConstantBuffer.pointLight.range);
            ImGui::InputFloat3("Attenutation", &m_PSConstantBuffer.pointLight.attr.x);
        } else {
            
            ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.spotLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.spotLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.spotLight.specular.x);
            ImGui::InputFloat("Spot", &m_PSConstantBuffer.spotLight.spot);
            ImGui::InputFloat("Range", &m_PSConstantBuffer.spotLight.range);
            ImGui::InputFloat3("Attenutation", &m_PSConstantBuffer.spotLight.attr.x);
        }
		ImGui::PopID();

		if(ImGui::Checkbox("WireFrame Mode", &m_IsWireframeMode)) {
			m_pD3D11DeviceContext->RSSetState(m_IsWireframeMode ? m_pRSWireframe.Get() : nullptr);
		}
	}
	ImGui::End();
	ImGui::Render();

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

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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