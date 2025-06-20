#ifndef __GAME_APP_H_
#define __GAME_APP_H_

#include "d3dApp.h"
#include "d3dUtil.h"
#include "LightHelper.h"
#include "Vertex.h"
#include "Geometry.h"

#include <DirectXMath.h>


class GameApp : public D3DApp {
public:
	struct VSConstantBuffer {
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
		DirectX::XMMATRIX worldInvTranspose;
	};

	struct PSConstantBuffer {
		DirectionalLight dirLight[10];
		PointLight pointLight[10];
		SpotLight spotLight[10];
		Material material;
		int numDirLight;
		int numPointLight;
		int numSpotLight;
		float pad;
		DirectX::XMFLOAT4 eyePos;
	};

	enum class ShowMode {WoodCrate, FireAnim};


    GameApp(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


private:
    bool InitEffect();
	bool InitResource();

	template <typename VertexType>
	bool ResetMesh(const Geometry::MeshData<VertexType>& meshData);

private:
    ComPtr<ID3D11InputLayout> m_pVertexLayout2D;
	ComPtr<ID3D11InputLayout> m_pVertexLayout3D;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
    ComPtr<ID3D11Buffer> m_pIndexBuffer;
    ComPtr<ID3D11Buffer> m_pConstantBuffer[2];
	UINT m_IndexCount = 0;
	ShowMode m_CurrMode;
	UINT m_CurrFrame = 0;


	ComPtr<ID3D11ShaderResourceView> m_pWoodCrate;
	std::vector<ComPtr<ID3D11ShaderResourceView>> m_pFireAnims;
	ComPtr<ID3D11SamplerState> m_pSamplerState;

    ComPtr<ID3D11VertexShader> m_pVertexShader3D;
	ComPtr<ID3D11PixelShader> m_pPixelShader3D;
	ComPtr<ID3D11VertexShader> m_pVertexShader2D;
	ComPtr<ID3D11PixelShader> m_pPixelShader2D;

	VSConstantBuffer m_VSConstantBuffer;
	PSConstantBuffer m_PSConstantBuffer;

	ComPtr<ID3D11RasterizerState> m_pRSWireframe;
	bool m_IsWireframeMode = false;

	DirectionalLight m_DirLight;
	PointLight m_PointLight;
	SpotLight m_SpotLight;

};

#endif // __GAME_APP_H_
