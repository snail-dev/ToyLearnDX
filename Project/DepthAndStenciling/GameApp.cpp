#include "GameApp.h"
#include <assert.h>
#include "DXTrace.h"
#include <DirectXMathMatrix.inl>
#include <DirectXColors.h>

#include "DDSTextureLoader11.h"
#include "WICTextureLoader11.h"

#include "RenderStates.h"

using namespace DirectX;


GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height) :
    D3DApp(hInstance, windowTitle, width, height) ,
	m_CameraMode(CameraMode::ThirdPerson),
	m_CBFrame(),
	m_CBResize(),
	m_CBRarely(),
	m_CBDrawingStates() {

}

GameApp::~GameApp() {

}

bool GameApp::Init() {
	return D3DApp::Init() && InitEffect() && InitResource();
}
void GameApp::OnResize() {
	D3DApp::OnResize();

	if(m_pCamera != nullptr) {
		m_pCamera->SetFrustum(XM_PI/ 3, AspectRatio(), 0.5f, 1000.0f);
		m_pCamera->SetViewPort(0.0f,0.0f, (float)m_mainWindowWidth, (float) m_mainWindowHeight);
		m_CBResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());

		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
		memcpy_s(mappedData.pData, sizeof(CBChangesOnResize), &m_CBResize, sizeof(CBChangesOnResize));
		m_pD3D11DeviceContext->Unmap(m_pConstantBuffer[3].Get(), 0);
	}
}

void GameApp::UpdateScene(float dt) {

	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera> (m_pCamera);
	auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera> (m_pCamera);

	ImGuiIO& io = ImGui::GetIO();
	if(m_CameraMode == CameraMode::Free) {
		float d1 = 0.0f, d2 = 0.0f;
		if(ImGui::IsKeyDown(ImGuiKey_W)) 
			d1 += dt;
		if(ImGui::IsKeyDown(ImGuiKey_S))
			d1 -= dt;
		if(ImGui::IsKeyDown(ImGuiKey_A))
			d2 -= dt;
		if(ImGui::IsKeyDown(ImGuiKey_D))
			d2 += dt;

		cam1st->MoveForward(d1 * 6.0f);
		cam1st->Strafe( d2 * 6.0f);

		if(ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
			cam1st->Pitch(io.MouseDelta.y * 0.01f);
			cam1st->RotateY(io.MouseDelta.x * 0.01f);
		}
	} 
	else if(m_CameraMode == CameraMode::ThirdPerson) {
		XMFLOAT3 target = m_WireFence.GetTransform().GetPosition();
		cam3rd->SetTarget(target);

		if(ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
			cam3rd->RotateX(io.MouseDelta.y * 0.01f);
			cam3rd->RotateY(io.MouseDelta.x * 0.01f);
		}
		cam3rd->Approach(-io.MouseWheel * 1.0f);
	}
	
	if(ImGui::Begin("Depth And Stenciling")) {
		ImGui::Text("W/S/A/D in FPS/Free camera");
		ImGui::Text("Hold the right mouse button and drag the view");

		static int curr_item = 0;
		static const char* modes[] = {
			"Third Person",
			"Free Camera"
		};

		if(ImGui::Combo("Camera Mode", &curr_item, modes, ARRAYSIZE(modes))) {
			if(curr_item == 0 && m_CameraMode != CameraMode::ThirdPerson) {
                if (!cam3rd) {
                    cam3rd = std::make_shared<ThirdPersonCamera>();
                    cam3rd->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
                    m_pCamera = cam3rd;
                }
				XMFLOAT3 target =  m_WireFence.GetTransform().GetPosition();
				cam3rd->SetTarget(target);
				cam3rd->SetDistance(8.0f);
				cam3rd->SetDistanceMinMax(3.0f,20.0f);
				cam3rd->SetRotationX(XM_PIDIV4);
                m_CameraMode = CameraMode::ThirdPerson;
			} 
			else if(curr_item == 1 && m_CameraMode != CameraMode::Free) {
				if(!cam1st) {
					cam1st = std::make_shared<FirstPersonCamera>();
					cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
					m_pCamera = cam1st;
				}
				XMFLOAT3 pos = m_WireFence.GetTransform().GetPosition();
				pos.y += 3;
				XMFLOAT3 look{0.0f,0.0f,1.0f};
				XMFLOAT3 up {0.0f,1.0f,0.0f};

				cam1st->LookTo(pos,look,up);
				m_CameraMode = CameraMode::Free;
			}
		}
	}
	ImGui::End();
	ImGui::Render();

	XMStoreFloat4(&m_CBFrame.eyePos, m_pCamera->GetPositionXM());
	m_CBFrame.view = XMMatrixTranspose(m_pCamera->GetViewXM());
	
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesEveryFrame), &m_CBFrame, sizeof(CBChangesEveryFrame));
	m_pD3D11DeviceContext->Unmap(m_pConstantBuffer[2].Get(), 0);
}

void GameApp::DrawScene() {
	assert(m_pD3D11DeviceContext);
	assert(m_pSwapChain);

	m_pD3D11DeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::Black));
	m_pD3D11DeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pD3D11DeviceContext->RSSetState(nullptr);
	m_pD3D11DeviceContext->OMSetDepthStencilState(RenderStates::DSSWriteStencil.Get(),1);
	m_pD3D11DeviceContext->OMSetBlendState(RenderStates::BSNoColorWrite.Get(),nullptr, 0xFFFFFFFF);

	m_Mirror.Draw(m_pD3D11DeviceContext.Get());

	m_CBDrawingStates.isReflection = true;
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBDrawingStates), &m_CBDrawingStates, sizeof(CBDrawingStates));
	m_pD3D11DeviceContext->Unmap(m_pConstantBuffer[1].Get(), 0);

	m_pD3D11DeviceContext->RSSetState(RenderStates::RSCullClockWise.Get());
	m_pD3D11DeviceContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(),1);
	m_pD3D11DeviceContext->OMSetBlendState(nullptr,nullptr, 0xFFFFFFFF);

	m_Walls[2].Draw(m_pD3D11DeviceContext.Get());
	m_Walls[3].Draw(m_pD3D11DeviceContext.Get());
	m_Walls[4].Draw(m_pD3D11DeviceContext.Get());
	m_Floor.Draw(m_pD3D11DeviceContext.Get());

	m_pD3D11DeviceContext->RSSetState(RenderStates::RSNoCull.Get());
	m_pD3D11DeviceContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), 1);
	m_pD3D11DeviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);

	m_WireFence.Draw(m_pD3D11DeviceContext.Get());
	m_Water.Draw(m_pD3D11DeviceContext.Get());
	m_Mirror.Draw(m_pD3D11DeviceContext.Get());

	m_CBDrawingStates.isReflection = false;
	HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(&mappedData.pData, sizeof(CBDrawingStates), &m_CBDrawingStates, sizeof(CBDrawingStates));
	m_pD3D11DeviceContext->Unmap(m_pConstantBuffer[1].Get(), 0);

	m_pD3D11DeviceContext->RSSetState(nullptr);
	m_pD3D11DeviceContext->OMSetDepthStencilState(nullptr,0);
	m_pD3D11DeviceContext->OMSetBlendState(nullptr,nullptr, 0xFFFFFFFF);

	for(auto& wall: m_Walls) {
		wall.Draw(m_pD3D11DeviceContext.Get());
	}
	m_Floor.Draw(m_pD3D11DeviceContext.Get());

	m_pD3D11DeviceContext->RSSetState(RenderStates::RSNoCull.Get());
	m_pD3D11DeviceContext->OMSetDepthStencilState(nullptr, 0);
	m_pD3D11DeviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFF);

	m_WireFence.Draw(m_pD3D11DeviceContext.Get());
	m_Water.Draw(m_pD3D11DeviceContext.Get());
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
	
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd,sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	cbd.ByteWidth = sizeof(CBChangesEveryDrawing);
	HR(m_pD3D11Device->CreateBuffer(&cbd, nullptr, m_pConstantBuffer[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBDrawingStates);
	HR(m_pD3D11Device->CreateBuffer(&cbd, nullptr, m_pConstantBuffer[1].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesEveryFrame);
	HR(m_pD3D11Device->CreateBuffer(&cbd, nullptr, m_pConstantBuffer[2].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesOnResize);
	HR(m_pD3D11Device->CreateBuffer(&cbd, nullptr, m_pConstantBuffer[3].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesRelaly);
	HR(m_pD3D11Device->CreateBuffer(&cbd,nullptr, m_pConstantBuffer[4].GetAddressOf()));

	ComPtr<ID3D11ShaderResourceView> texture;

	Material material{};
	material.ambient = DirectX::XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
	material.diffuse = DirectX::XMFLOAT4(1.0f,1.0f,1.0f,1.0f);
	material.specular = DirectX::XMFLOAT4(0.2f,0.2f,0.2f,16.0f);

	HR(DirectX::CreateDDSTextureFromFile(m_pD3D11Device.Get(), L"Texture\\WireFence.dds", nullptr, texture.GetAddressOf()));
	m_WireFence.SetBuffer(m_pD3D11Device.Get(), Geometry::CreateBox());
	m_WireFence.GetTransform().SetPosition(0.0f,0.01f, 7.5f);
	m_WireFence.SetTexture(texture.Get());
	m_WireFence.SetMaterial(material);

	HR(DirectX::CreateDDSTextureFromFile(m_pD3D11Device.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Floor.SetBuffer(m_pD3D11Device.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f,20.0f), XMFLOAT2(5.0f,5.0f)));
	m_Floor.SetTexture(texture.Get());
	m_Floor.GetTransform().SetPosition(0.0f,-1.0f,0.0f);
	m_Floor.SetMaterial(material);
	m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);

	m_Walls.resize(5);
	HR(CreateDDSTextureFromFile(m_pD3D11Device.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));



	for(int i = 0; i < 5; i++)
	{
		m_Walls[i].SetMaterial(material);
		m_Walls[i].SetTexture(texture.Get());
	}

	m_Walls[0].SetBuffer(m_pD3D11Device.Get(), Geometry::CreatePlane(XMFLOAT2(6.0f,8.0f), XMFLOAT2(1.5f,2.0f)));
	m_Walls[0].GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Walls[0].GetTransform().SetPosition(-7.0f, 3.0f, 10.0f);

    m_Walls[1].SetBuffer(m_pD3D11Device.Get(), Geometry::CreatePlane(XMFLOAT2(6.0f, 8.0f), XMFLOAT2(1.5f, 2.0f)));
    m_Walls[1].GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
    m_Walls[1].GetTransform().SetPosition(7.0f, 3.0f, 10.0f);

    m_Walls[2].SetBuffer(m_pD3D11Device.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
    m_Walls[2].GetTransform().SetRotation(-XM_PIDIV2, XM_PIDIV2, 0.0f);
    m_Walls[2].GetTransform().SetPosition(10.0f, 3.0f, 0.0f);

    m_Walls[3].SetBuffer(m_pD3D11Device.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
    m_Walls[3].GetTransform().SetRotation(-XM_PIDIV2, XM_PI, 0.0f);
    m_Walls[3].GetTransform().SetPosition(0.0f, 3.0f, -10.0f);

    m_Walls[4].SetBuffer(m_pD3D11Device.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
    m_Walls[4].GetTransform().SetRotation(-XM_PIDIV2, -XM_PIDIV2, 0.0f);
    m_Walls[4].GetTransform().SetPosition(-10.0f, 3.0f, 0.0f);

    material.ambient = DirectX::XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
	material.diffuse = DirectX::XMFLOAT4(1.0f,1.0f,1.0f,0.5f);
	material.specular = DirectX::XMFLOAT4(0.8f,0.8f,0.8f,32.0f);

	HR(CreateDDSTextureFromFile(m_pD3D11Device.Get(), L"Texture\\water.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Water.SetBuffer(m_pD3D11Device.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f,20.0f), XMFLOAT2(10.0f,10.0f)));
	m_Water.SetTexture(texture.Get());
	m_Water.SetMaterial(material);

    material.ambient = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    material.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
    material.specular = DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	HR(CreateDDSTextureFromFile(m_pD3D11Device.Get(),L"Texture\\ice.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Mirror.SetBuffer(m_pD3D11Device.Get(), Geometry::CreatePlane(XMFLOAT2(8.0f,8.0f),XMFLOAT2(1.0f,1.0f)));
	m_Mirror.GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Mirror.GetTransform().SetPosition(0.0f,3.0f,10.0f);
	m_Mirror.SetTexture(texture.Get());
	m_Mirror.SetMaterial(material);

    auto camera = std::make_shared<ThirdPersonCamera>();
    m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_mainWindowWidth, (float)m_mainWindowHeight);
	camera->SetTarget(m_WireFence.GetTransform().GetPosition());
	camera->SetDistance(8.0f);
	camera->SetDistanceMinMax(3.0f,20.0f);
	camera->SetRotationX(XM_PIDIV4);

	m_CBFrame.view = XMMatrixTranspose(m_pCamera->GetViewXM());
	XMStoreFloat4(&m_CBFrame.eyePos, m_pCamera->GetPositionXM());

	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	m_CBResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());

	m_CBRarely.reflection = XMMatrixTranspose(XMMatrixReflect(XMVectorSet(0.0f,0.0f,-1.0f,10.0f)));
	m_CBRarely.dirLight[0].ambient = DirectX::XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
    m_CBRarely.dirLight[0].diffuse = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    m_CBRarely.dirLight[0].specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_CBRarely.dirLight[0].direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);

    m_CBRarely.pointLight[0].position = DirectX::XMFLOAT3(0.0f, 15.0f, 0.0f);
    m_CBRarely.pointLight[0].ambient = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_CBRarely.pointLight[0].diffuse = DirectX::XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    m_CBRarely.pointLight[0].specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_CBRarely.pointLight[0].attr = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
    m_CBRarely.pointLight[0].range = 25.0f;

	m_CBRarely.numDirLight = 1;
	m_CBRarely.numPointLight = 1;
	m_CBRarely.numSpotLight = 0;


	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer[3].Get(),0,D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBChangesOnResize), &m_CBResize, sizeof(CBChangesOnResize));
    m_pD3D11DeviceContext->Unmap(m_pConstantBuffer[3].Get(),0);

    HR(m_pD3D11DeviceContext->Map(m_pConstantBuffer[4].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBChangesRelaly), &m_CBRarely, sizeof(CBChangesRelaly));
    m_pD3D11DeviceContext->Unmap(m_pConstantBuffer[4].Get(), 0);


	RenderStates::InitAll(m_pD3D11Device.Get());

	m_pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pD3D11DeviceContext->IASetInputLayout(m_pVertexLayout3D.Get());

	m_pD3D11DeviceContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);

	m_pD3D11DeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer[0].GetAddressOf());
	m_pD3D11DeviceContext->VSSetConstantBuffers(1, 1, m_pConstantBuffer[1].GetAddressOf());
	m_pD3D11DeviceContext->VSSetConstantBuffers(2, 1, m_pConstantBuffer[2].GetAddressOf());
    m_pD3D11DeviceContext->VSSetConstantBuffers(3, 1, m_pConstantBuffer[3].GetAddressOf());
    m_pD3D11DeviceContext->VSSetConstantBuffers(4, 1, m_pConstantBuffer[4].GetAddressOf());

    m_pD3D11DeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer[0].GetAddressOf());
	m_pD3D11DeviceContext->PSSetConstantBuffers(1, 1, m_pConstantBuffer[1].GetAddressOf());
	m_pD3D11DeviceContext->PSSetConstantBuffers(2, 1, m_pConstantBuffer[2].GetAddressOf());
    m_pD3D11DeviceContext->PSSetConstantBuffers(4, 1, m_pConstantBuffer[4].GetAddressOf());

    m_pD3D11DeviceContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);
	m_pD3D11DeviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());


	D3D11SetDebugObjectName(m_pVertexLayout2D.Get(), "VertexPosTexLayout");
	D3D11SetDebugObjectName(m_pVertexLayout3D.Get(), "VertexPosNormalTexLayout");
	D3D11SetDebugObjectName(m_pVertexShader2D.Get(), "Basic_2D_VS");
	D3D11SetDebugObjectName(m_pPixelShader2D.Get(), "Basic_2D_PS");
	D3D11SetDebugObjectName(m_pVertexShader3D.Get(), "Basic_3D_VS");
	D3D11SetDebugObjectName(m_pPixelShader3D.Get(), "Basic_3D_PS");
    D3D11SetDebugObjectName(m_pConstantBuffer[0].Get(), "CBDrawing");
    D3D11SetDebugObjectName(m_pConstantBuffer[1].Get(), "CBStates");
    D3D11SetDebugObjectName(m_pConstantBuffer[2].Get(), "CBFrame");
    D3D11SetDebugObjectName(m_pConstantBuffer[3].Get(), "CBOnResize");
    D3D11SetDebugObjectName(m_pConstantBuffer[4].Get(), "CBRarely");

    m_Floor.SetDebugObjectName("Floor");
	m_Mirror.SetDebugObjectName("Mirror");
	m_WireFence.SetDebugObjectName("WireFence");
	m_Water.SetDebugObjectName("Water");
	m_Walls[0].SetDebugObjectName("Walls[0]");
	m_Walls[1].SetDebugObjectName("Walls[1]");
	m_Walls[2].SetDebugObjectName("Walls[2]");
	m_Walls[3].SetDebugObjectName("Walls[3]");
    m_Walls[4].SetDebugObjectName("Walls[4]");

    return true;
}


GameApp::GameObject::GameObject() : m_pIndexBuffer(), m_pVertexBuffer(), m_Material(){

}

GameApp::GameObject::~GameObject() {

}

Transform& GameApp::GameObject::GetTransform() {
	return m_Transform;
}

const Transform& GameApp::GameObject::GetTransform() const {
	return m_Transform;
}

template <typename VertexType, typename IndexType>
void GameApp::GameObject::SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData) 
{
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC  vbd;
	ZeroMemory(&vbd, sizeof(vbd));

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.ByteWidth = (UINT) meshData.vertexVec.size() * m_VertexStride;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = meshData.vertexVec.data();

	HR(device->CreateBuffer(&vbd, &initData, m_pVertexBuffer.GetAddressOf()));

	m_IndexCount = (UINT) meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));

	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.ByteWidth = (UINT) meshData.indexVec.size() * sizeof(IndexType);

	initData.pSysMem = meshData.indexVec.data();

	HR(device->CreateBuffer(&ibd, &initData, m_pIndexBuffer.GetAddressOf()));
}

void GameApp::GameObject::SetTexture(ID3D11ShaderResourceView* texture) {
	m_SRV = texture;
}

void GameApp::GameObject::SetMaterial(const Material& material) {
	m_Material = material;
}

void GameApp::GameObject::Draw(ID3D11DeviceContext* context) {
	UINT strides = m_VertexStride;
	UINT offsets = 0;

	context->IASetVertexBuffers(0,1,m_pVertexBuffer.GetAddressOf(), & strides, &offsets);
	context->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT,0);

	ComPtr<ID3D11Buffer> cBuffer = nullptr;
	context->VSGetConstantBuffers(0,1,cBuffer.GetAddressOf());

	CBChangesEveryDrawing cbDrawing;

	XMMATRIX W = m_Transform.GetLocalToWorldMatrixXM();
	cbDrawing.world = XMMatrixTranspose(W);
	cbDrawing.worldInvTranspose =  XMMatrixTranspose(InverseTranspose(W));
	cbDrawing.material = m_Material;

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(context->Map(cBuffer.Get(),0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBChangesEveryDrawing), &cbDrawing, sizeof(CBChangesEveryDrawing));
	context->Unmap(cBuffer.Get(), 0);

	context->PSSetShaderResources(0,1, m_SRV.GetAddressOf());

	context->DrawIndexed(m_IndexCount, 0, 0);
}

void GameApp::GameObject::SetDebugObjectName(const std::string& name) {
#if defined(DEBUG) || defined (_DEBUG)
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name+".VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name+".IndexBuffer");
#else
	UNREFERENCED_PARAMETER(name);
#endif
}