#ifndef EFFECT_HELPER_H_
#define EFFECT_HELPER_H_

#include "DXTrace.h"

template <typename DerivedType>
struct AlignedType {
	static void* operator new(size_t size) {

		const size_t alignedSize = __alignof(DerivedType);
		static_assert(alignedSize > 8, "AlignedNew is only useful with > 8 bytes aligment!");

		void* ptr = _aligned_malloc(size, alignedSize);
		if (!ptr) {
			throw std::bad_alloc();
		}

		return ptr;
	}

	static void operator delete(void* ptr) {
		_aligned_free(ptr);
	}
};

struct CBufferBase {
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	BOOL isDirty;
	ComPtr<ID3D11Buffer>  cbuffer;

	CBufferBase() : isDirty() {}
	virtual ~CBufferBase() = default;

	virtual HRESULT CreateBuffer(ID3D11Device* device) = 0;
	virtual void UpdateBuffer(ID3D11DeviceContext* context) = 0;
	virtual void BindVS(ID3D11DeviceContext* context) = 0;
	virtual void BindPS(ID3D11DeviceContext* context) = 0;
	virtual void BindCS(ID3D11DeviceContext* context) = 0;
	virtual void BindDS(ID3D11DeviceContext* context) = 0;
	virtual void BindGS(ID3D11DeviceContext* context) = 0;
	virtual void BindHS(ID3D11DeviceContext* context) = 0;

	
};

template <UINT startSlot, class T>
struct CBufferObject : CBufferBase {

	T data;

	CBufferObject() : CBufferBase(), data() {}

	HRESULT CreateBuffer(ID3D11Device* device) override {
		if (cbuffer)
			return S_OK;
	
		D3D11_BUFFER_DESC cbd;
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.ByteWidth = sizeof(T);
		return device->CreateBuffer(&cbd, nullptr, cbuffer.GetAddressOf());
	}

	void UpdateBuffer(ID3D11DeviceContext* context) {
		if (isDirty) {
			isDirty = FALSE;

			D3D11_MAPPED_SUBRESOURCE mapped;
			context->Map(cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			memcpy_s(mapped.pData, sizeof(T), &data, sizeof(T));
			context->Unmap(cbuffer.Get(),0);
		}
	}

	void BindVS(ID3D11DeviceContext* context) override {
		context->VSSetConstantBuffers(startSlot, 1, cbuffer.GetAddressOf());
	}

	void BindPS(ID3D11DeviceContext* context) override {
		context->PSSetConstantBuffers(startSlot, 1, cbuffer.GetAddressOf());
	}

	void BindGS(ID3D11DeviceContext* context) override {
		context->GSSetConstantBuffers(startSlot, 1, cbuffer.GetAddressOf());
	}

	void BindCS(ID3D11DeviceContext* context) override {
		context->CSSetConstantBuffers(startSlot, 1, cbuffer.GetAddressOf());
	}

	void BindDS(ID3D11DeviceContext* context) override {
		context->DSSetConstantBuffers(startSlot, 1, cbuffer.GetAddressOf());
	}

	void BindHS(ID3D11DeviceContext* context) override {
		context->DSSetConstantBuffers(startSlot, 1, cbuffer.GetAddressOf());
	}


};



#endif // EFFECT_HELPER_H_