#ifndef _D3D_UTIL_H_
#define _D3D_UTIL_H_

#include <d3d11_1.h>
#include <DirectXCollision.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <vector>
#include <string>

#ifndef GRAPHICS_DEBUGGER_OBJECT_NAME
#define GRAPHICS_DEBUGGER_OBJECT_NAME (1)
#endif

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) == nullptr; }}


template <UINT TNameLength>
inline void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_ const char(&name)[TNameLength]) {
#if (defined(DEBUG) || defined (_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength -1, name);
#else
    UNREFERENCED_PARAMETER(resource);
	UNREFERENCED_PARAMETER(name);
#endif 
}

inline void D3D11SetDebugOjbectName(_In_ ID3D11DeviceChild* resource, _In_ LPCSTR name, _In_ UINT length) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, length, name);
#else
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(length);
#endif
}

inline void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_ const std::string& name) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.length(), name.c_str());
#else
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(name);
#endif
}

inline void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_ std::nullptr_t ) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, 0,nullptr);
#else
    UNREFERENCED_PARAMETER(resource);
#endif
}

template <UINT TNameLength>
inline void D3D11SetDebugObjectName(_In_ IDXGIObject* object, _In_ const char (&name)[TNameLength]) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    object->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
#else
    UNREFERENCED_PARAMETER(object);
    UNREFERENCED_PARAMETER(name);
#endif
}

inline void D3D11SetDebugOjbectName(_In_ IDXGIObject* object, _In_ LPCSTR name, _In_ UINT length) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    object->SetPrivateData(WKPDID_D3DDebugObjectName, length, name);
#else
    UNREFERENCED_PARAMETER(object);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(length);
#endif
}

inline void D3D11SetDebugObjectName(_In_ IDXGIObject* object, _In_ const std::string& name) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.length(), name.c_str());
#else
    UNREFERENCED_PARAMETER(object);
    UNREFERENCED_PARAMETER(name);
#endif
}

inline void D3D11SetDebugObjectName(_In_ IDXGIObject* object, _In_ std::nullptr_t) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    object->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
#else
    UNREFERENCED_PARAMETER(object);
#endif
}

HRESULT CreateShaderFromFile(const WCHAR* csoFile, const WCHAR* hlslFile, LPCSTR* entryPoint, LPCSTR* shaderModel, ID3DBlob** ppBlob);

#endif // _D3D_UTIL_H_