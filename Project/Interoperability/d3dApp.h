#ifndef _D3D_APP_H_
#define _D3D_APP_H_

#include <wrl/client.h>
#include <Windows.h>
#include <d3d11_1.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>

#include "CpuTimer.h"
#include "Keyboard.h"
#include "Mouse.h"

class D3DApp {
public:
    D3DApp(HINSTANCE hinstance, const std::wstring& windowTitle, int windowWidth, int windowHight);
    virtual ~D3DApp();

    HINSTANCE AppInstance() const;
    HWND MainWnd() const;
    float AspectRatio() const;

    int Run();

    virtual bool Init();
    virtual void OnResize();

    virtual void UpdateScene(float dt) = 0;
    virtual void DrawScene() = 0;

    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
    bool InitMainWindow();
    bool InitDirect3D();
    bool InitDirect2D();

    void CalculateFrameStats();

protected:
    HWND m_mainWnd;
    HINSTANCE m_hInstance;
    std::wstring m_mainWindowTitle;
    int m_mainWindowWidth;
    int m_mainWindowHeight;

    bool m_AppPaused = false;
    bool m_Resizing = false;

    bool m_Minimized = false;
    bool m_Maximized = false;

    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    ComPtr<ID3D11Device> m_pD3D11Device;
    ComPtr<ID3D11DeviceContext> m_pD3D11DeviceContext;
    ComPtr<IDXGISwapChain> m_pSwapChain;

    ComPtr<ID3D11Device1> m_pD3D11Device1;
    ComPtr<ID3D11DeviceContext1> m_pD3D11DeviceContext1;
    ComPtr<IDXGISwapChain1> m_pSwapChain1;

    ComPtr<ID3D11Texture2D> m_pDepthStencilBuffer;
    ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
    ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;


    ComPtr<ID2D1Factory> m_pD2DFactory;
    ComPtr<ID2D1RenderTarget> m_pD2DRenderTarget;
    ComPtr<IDWriteFactory> m_pDWriteFactory;

    D3D11_VIEWPORT m_ScreenViewPort;

    bool m_Enable4xMsaa = false;
    UINT m_4xMsaaQualityLevel = 0;

    CpuTimer m_Timer;

    std::unique_ptr<DirectX::Mouse> m_pMouse;
    DirectX::Mouse::ButtonStateTracker m_MouseTracker;
    std::unique_ptr<DirectX::Keyboard> m_pKeyboard;
    DirectX::Keyboard::KeyboardStateTracker m_KeyboardTracker;
    
};

#endif  // _D3D_APP_H_