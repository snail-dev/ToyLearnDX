#include "GameApp.h"
#include "DXTrace.h"
#include <assert.h>

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height) :
    D3DApp(hInstance, windowTitle, width, height) {

	}

GameApp::~GameApp() {

}

bool GameApp::Init() {
	return D3DApp::Init();
}
void GameApp::OnResize() {
	D3DApp::OnResize();
}
void GameApp::UpdateScene(float dt) {
}
void GameApp::DrawScene() {
	assert(m_pD3D11DeviceContext);
	assert(m_pSwapChain);

	static float blue[4] = {0.0f,0.0f,1.0f,1.0f};
	m_pD3D11DeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), blue);
	m_pD3D11DeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	HR(m_pSwapChain->Present(0,0));
}