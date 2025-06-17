#include "d3dUtil.h"

HRESULT CreateShaderFromFile(const WCHAR* csoFile, const WCHAR* hlslFile, LPCSTR* entryPoint, LPCSTR* shaderModel, ID3DBlob** ppBlob) {

	HRESULT hr = S_OK;

	if(csoFile && D3DReadFileToBlob(csoFile, ppBlob) == S_OK) {
		return hr;
	}

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined (DEBUG) || defined (_DEBUG)
    dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
   ID3DBlob* errorBlob = nullptr;
   hr = D3DCompileFromFile(hlslFile, nullptr,D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, dwShaderFlags, 0, ppBlob, &errorBlob);
   if(FAILED(hr)) {
	   if(errorBlob != nullptr) {
		   OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
	   }
	   SAFE_RELEASE(errorBlob);
	   return hr;
   }

   if(csoFile) {
	   return D3DWriteBlobToFile(*ppBlob, csoFile, FALSE);
   }
   return hr;
}