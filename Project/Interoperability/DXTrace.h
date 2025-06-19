#ifndef _DX_TRACE_H_
#define _DX_TRACE_H_

#include <Windows.h>

HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* file, _In_ DWORD line, _In_ HRESULT hr, _In_opt_ const WCHAR* msg, _In_ bool popMsgBox);


#if defined (DEBUG) || defined (_DEBUG)
#ifndef HR
#define HR(x)																    \
            {																	\
				HRESULT hr = (x);												\
				if(FAILED(hr))                                                  \
					DXTraceW(__FILEW__, (DWORD)__LINE__, hr, L#x, true);        \
			}                                                              
#endif
#else 
#ifndef HR
#define HR(x) (x)
#endif
#endif




#endif // _DX_TRACE_H_