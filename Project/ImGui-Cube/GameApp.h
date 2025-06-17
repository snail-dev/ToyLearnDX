#ifndef __GAME_APP_H_
#define __GAME_APP_H_

#include "d3dApp.h"
#include "d3dUtil.h"

class GameApp : public D3DApp {
public:

    struct VertexPosColor {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
		static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
	};

	struct ConstantBuffer {
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
		DirectX::XMFLOAT4 color;
		uint32_t useCustomColor;
		uint32_t pads[3];
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

private:
    ComPtr<ID3D11InputLayout> m_pVertexLayout;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11PixelShader> m_pPixelShader;

	ComPtr<ID3D11Buffer> m_pIndexBuffer;
	ComPtr<ID3D11Buffer> m_pConstantBuffer;

	ConstantBuffer m_CBuffer;
};

#endif // __GAME_APP_H_
