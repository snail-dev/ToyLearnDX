#include "d3dApp.h"
#include "DXTrace.h"
#include "d3dUtil.h"
#include <assert.h>
#include <sstream>

namespace {
D3DApp* g_pd3dApp = nullptr;
}

D3DApp::D3DApp(HINSTANCE hinstance, const std::wstring& windowTitle, int windowWidth, int windowHight)
        : m_hInstance(hinstance),
          m_mainWindowTitle(windowTitle),
          m_mainWindowWidth(windowWidth),
          m_mainWindowHeight(windowHight) {
    g_pd3dApp = this;
}

D3DApp::~D3DApp() {
	if(m_pD3D11DeviceContext) {
		m_pD3D11DeviceContext->ClearStatus();
	}
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return g_pd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

HINSTANCE D3DApp::AppInstance() const {
    return m_hInstance;
}
float D3DApp::AspectRatio() const {
    return (1.0f * m_mainWindowWidth) / (1.0f * m_mainWindowHeight);
}

HWND D3DApp::MainWnd() const {
    return m_mainWnd;
}

bool D3DApp::Init() {
    return (InitMainWindow() && InitDirect3D());
}

int D3DApp::Run() {
	MSG msg = {0};
	m_Timer.Reset();

	while(msg.message != WM_QUIT) {
		if(PeekMessage(&msg,0,0,0,PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			m_Timer.Tick();
			if(!m_AppPaused) {
				CalculateFrameStats();
				UpdateScene(m_Timer.DeltaTime());
				DrawScene();
			} else {
				Sleep(100);
			}
		}
	}
	return (int)msg.wParam;
}

void D3DApp::OnResize() {
	assert(m_pD3D11DeviceContext);
	assert(m_pD3D11Device);
	assert(m_pSwapChain);

	if(m_pD3D11Device1 != nullptr) {
		assert(m_pD3D11DeviceContext1);
		assert(m_pD3D11Device1);
		assert(m_pSwapChain1);
	}

	m_pRenderTargetView.Reset();
	m_pDepthStencilView.Reset();
	m_pDepthStencilBuffer.Reset();

	ComPtr<ID3D11Texture2D> backBuffer;
	HR(m_pSwapChain->ResizeBuffers(1,m_mainWindowWidth,m_mainWindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(backBuffer.GetAddressOf())));
	HR(m_pD3D11Device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_pRenderTargetView.GetAddressOf()));
	
	D3D11SetDebugObjectName(backBuffer.Get(), "BackBuffer[0]");

	backBuffer.Reset();


	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = m_mainWindowWidth;
	depthStencilDesc.Height = m_mainWindowHeight;
	depthStencilDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;

	if(m_Enable4xMsaa) {
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQualityLevel - 1;
	} else {
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_pD3D11Device->CreateTexture2D(&depthStencilDesc, nullptr, m_pDepthStencilBuffer.GetAddressOf()));
	HR(m_pD3D11Device->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, m_pDepthStencilView.GetAddressOf()));

	m_pD3D11DeviceContext->OMSetRenderTargets(1,m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

    m_ScreenViewPort.TopLeftX = 0;
	m_ScreenViewPort.TopLeftY = 0;
	m_ScreenViewPort.Width = static_cast<float> (m_mainWindowWidth);
	m_ScreenViewPort.Height = static_cast<float> (m_mainWindowHeight);
	m_ScreenViewPort.MinDepth = 0.0f;
	m_ScreenViewPort.MaxDepth = 1.0f;

	m_pD3D11DeviceContext->RSSetViewports(1, &m_ScreenViewPort);

}

bool D3DApp::InitMainWindow() {
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"D3DWndClassName";

    if (!RegisterClass(&wc)) {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    RECT R = {0, 0, m_mainWindowWidth, m_mainWindowHeight};
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);

    int width = R.right - R.left;
    int height = R.bottom - R.top;

    m_mainWnd = CreateWindow(L"D3DWndClassName", m_mainWindowTitle.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
            CW_USEDEFAULT, width, height, 0, 0, m_hInstance, 0);

    if (!m_mainWnd) {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(m_mainWnd, SW_SHOW);
    UpdateWindow(m_mainWnd);

    return true;
}

bool D3DApp::InitDirect3D() {
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] = {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
    };

    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    D3D_FEATURE_LEVEL featureLevel;
    D3D_DRIVER_TYPE driverType;

    for (UINT driverTypeIdx = 0; driverTypeIdx < numDriverTypes; ++driverTypeIdx) {
        driverType = driverTypes[driverTypeIdx];
        hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_pD3D11Device.GetAddressOf(), &featureLevel, m_pD3D11DeviceContext.GetAddressOf());

        if (hr == E_INVALIDARG) {
            hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1],
                    numFeatureLevels - 1, D3D11_SDK_VERSION, m_pD3D11Device.GetAddressOf(), &featureLevel,
                    m_pD3D11DeviceContext.GetAddressOf());
        }

        if (SUCCEEDED(hr)) {
            break;
        }
    }
    if (FAILED(hr)) {
        MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
        return false;
    }

    if (featureLevel != D3D_FEATURE_LEVEL_11_0 && featureLevel != D3D_FEATURE_LEVEL_11_1) {
        MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
        return false;
    }

    m_pD3D11Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQualityLevel);
    assert(m_4xMsaaQualityLevel > 0);

    ComPtr<IDXGIDevice> dxgiDevice = nullptr;
    ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
    ComPtr<IDXGIFactory1> dxgiFactory1 = nullptr;
    ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;

    HR(m_pD3D11Device.As(&dxgiDevice));
    HR(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));
    HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory1.GetAddressOf())));

    hr = dxgiFactory1.As(&dxgiFactory2);
    if (dxgiFactory2 != nullptr) {
        HR(m_pD3D11Device.As(&m_pD3D11Device1));
        HR(m_pD3D11DeviceContext.As(&m_pD3D11DeviceContext1));

        DXGI_SWAP_CHAIN_DESC1 sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.Width = m_mainWindowWidth;
        sd.Height = m_mainWindowHeight;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        if (m_Enable4xMsaa) {
            sd.SampleDesc.Count = 4;
            sd.SampleDesc.Quality = m_4xMsaaQualityLevel - 1;
        } else {
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
        }

        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd;
        fd.RefreshRate.Numerator = 60;
        fd.RefreshRate.Denominator = 1;
        fd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        fd.Windowed = TRUE;
        HR(dxgiFactory2->CreateSwapChainForHwnd(
                m_pD3D11Device.Get(), m_mainWnd, &sd, &fd, nullptr, m_pSwapChain1.GetAddressOf()));
        HR(m_pSwapChain1.As(&m_pSwapChain));
    } else {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferDesc.Width = m_mainWindowWidth;
        sd.BufferDesc.Height = m_mainWindowHeight;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        if (m_Enable4xMsaa) {
            sd.SampleDesc.Count = 4;
            sd.SampleDesc.Quality = m_4xMsaaQualityLevel - 1;
        } else {
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
        }

        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;
        sd.OutputWindow = m_mainWnd;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;

        HR(dxgiFactory1->CreateSwapChain(m_pD3D11Device.Get(), &sd, m_pSwapChain.GetAddressOf()));
    }

    dxgiFactory1->MakeWindowAssociation(m_mainWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

    D3D11SetDebugObjectName(m_pD3D11DeviceContext.Get(), "D3D11DeviceContext");
    D3D11SetDebugObjectName(m_pSwapChain.Get(), "SwapChain");

    OnResize();

    return true;
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                m_AppPaused = true;
				m_Timer.Stop();
            } else {
                m_AppPaused = false;
				m_Timer.Start();
            }
            return 0;
        case WM_SIZE:
            m_mainWindowWidth = LOWORD(lParam);
            m_mainWindowHeight = HIWORD(lParam);
			if(m_pD3D11Device) {
				if(wParam == SIZE_MINIMIZED) {
					m_AppPaused = false;
					m_Minimized = true;
					m_Maximized = false;
				} else if(wParam == SIZE_MAXIMIZED) {
					m_AppPaused = false;
					m_Minimized = false;
					m_Maximized = true;
					OnResize();
				} else if(wParam == SIZE_RESTORED) {
					if(m_Minimized) {
						m_Minimized = false;
						m_AppPaused = false;
					}
					if(m_Maximized) {
						m_Maximized = false;
						m_AppPaused = false;
					}
					if(!m_Resizing) {
						OnResize();
					}
				}
			}
            return 0;

        case WM_ENTERSIZEMOVE:
            m_AppPaused = true;
			m_Timer.Stop();
            m_Resizing = true;
            return 0;
        case WM_EXITSIZEMOVE:
            m_AppPaused = false;
			m_Resizing = false;
			m_Timer.Start();
			OnResize();
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_MENUCHAR:
            return MAKELRESULT(0, MNC_CLOSE);
        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
            return 0;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            return 0;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            return 0;
        case WM_MOUSEMOVE:
            return 0;
    }
    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

void D3DApp::CalculateFrameStats(){
	static int fps = 0;
	static float timeElapsed = 0.0f;

	fps ++;

	if((m_Timer.TotalTime() - timeElapsed ) >= 1.0f) {

		float f = (float) fps;
		float mspf = 1000.0f / f;

		std::wostringstream outs;
		outs.precision(6);
		outs<< m_mainWindowTitle << L"  "
		    << L"FPS: " << f <<L" "
			<< L"Frame Time:" << mspf <<L"(ms)";
			SetWindowText(m_mainWnd, outs.str().c_str());

			fps = 0;
			timeElapsed += 1.0f;
	}
}