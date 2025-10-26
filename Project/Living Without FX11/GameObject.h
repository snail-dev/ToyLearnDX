#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <string>
#include <wrl/client.h>

#include "Transform.h"
#include "Geometry.h"
#include "Effect.h"
#include "DXTrace.h"

class GameObject {
public:

	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	GameObject();

	Transform& GetTransform();
	const Transform& GetTransform() const;

	template <typename VertexType, typename IndexType>
	void SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData);

	void SetTexture(ID3D11ShaderResourceView* texture);

	void SetMaterial(const Material& material);

	void Draw(ID3D11DeviceContext* context, BasicEffect& effect);

	void SetDebugObjectName(const std::string& name);
	
private:
	Transform m_Transform;
	Material m_Material;

	ComPtr<ID3D11ShaderResourceView> m_SRV;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;

	UINT m_VertexStride;
	UINT m_IndexCount;
};

template <typename VertexType, typename IndexType>
inline void GameObject::SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData) {
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	if (device == nullptr)
		return;

	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC  vbd;
	ZeroMemory(&vbd, sizeof(vbd));

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = meshData.vertexVec.data();

	HR(device->CreateBuffer(&vbd, &initData, m_pVertexBuffer.GetAddressOf()));

	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.ByteWidth = (UINT)meshData.indexVec.size() * sizeof(IndexType);
	ibd.CPUAccessFlags = 0;
	initData.pSysMem = meshData.indexVec.data();
	HR(device->CreateBuffer(&ibd, &initData, m_pIndexBuffer.GetAddressOf()));
}

#endif // GAME_OBJECT_H