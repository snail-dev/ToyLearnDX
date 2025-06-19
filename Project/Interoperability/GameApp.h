#ifndef __GAME_APP_H_
#define __GAME_APP_H_

#include "d3dApp.h"
#include "d3dUtil.h"
#include "LightHelper.h"
#include "Vertex.h"
#include "Geometry.h"

#include <DirectXMath.h>

#include "Keyboard.h"
#include "Mouse.h"


class GameApp : public D3DApp {
public:
	struct VSConstantBuffer {
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
		DirectX::XMMATRIX worldInvTranspose;
	};

	struct PSConstantBuffer {
		DirectionalLight dirLight;
		PointLight pointLight;
		SpotLight spotLight;
		Material material;
		DirectX::XMFLOAT4 eyePos;
	};

    GameApp(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


private:
    bool InitEffect();
	bool InitResource();
    bool InitMouse();

    bool ResetMesh(const Geometry::MeshData<VertexPosNormalColor>& meshData);

private:
    ComPtr<ID3D11InputLayout> m_pVertexLayout;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
    ComPtr<ID3D11Buffer> m_pIndexBuffer;
    ComPtr<ID3D11Buffer> m_pConstantBuffer[2];
	UINT m_IndexCount = 0;

    ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11PixelShader> m_pPixelShader;
	VSConstantBuffer m_VSConstantBuffer;
	PSConstantBuffer m_PSConstantBuffer;

	DirectionalLight m_DirLight;
	PointLight m_PointLight;
	SpotLight m_SpotLight;

	ComPtr<ID3D11RasterizerState> m_pRSWireframe;
	bool m_IsWireframeMode = false;

    ComPtr<ID2D1SolidColorBrush> m_pColorBrush;
    ComPtr<IDWriteFont> m_pFont;
    ComPtr<IDWriteTextFormat> m_pTextFormat;
};

#endif // __GAME_APP_H_
