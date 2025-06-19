#ifndef _VERTEX_H_
#define _VERTEX_H_

#include <DirectXMath.h>
#include <d3d11_1.h>

struct VertexPos {
	VertexPos() = default;

	VertexPos(const VertexPos&) = default;
	VertexPos& operator=(const VertexPos&) = default;

	VertexPos(VertexPos&&) = default;
	VertexPos& operator=(VertexPos&&) = default;

	VertexPos(const DirectX::XMFLOAT3 _pos) : pos(_pos) {}

	DirectX::XMFLOAT3 pos;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[1];

};

struct VertexPosColor {

	VertexPosColor() = default;

	VertexPosColor(const VertexPosColor&) = default;
	VertexPosColor& operator=(const VertexPosColor&) = default;

	VertexPosColor(VertexPosColor&&) = default;
	VertexPosColor& operator=(VertexPosColor&&) = default;

	VertexPosColor(const DirectX::XMFLOAT3& _pos, const DirectX::XMFLOAT4& _color) : pos(_pos), color(_color) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
};


struct VertexPosTex {

	VertexPosTex() = default;
	VertexPosTex(const VertexPosTex&) = default;
	VertexPosTex& operator=(const VertexPosTex&) = default;

	VertexPosTex(VertexPosTex&&) = default;
	VertexPosTex& operator=(VertexPosTex&&) = default;

	VertexPosTex(const DirectX::XMFLOAT3& _pos, const DirectX::XMFLOAT2& _tex) : pos(_pos), tex(_tex) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 tex;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
};

struct VertexPosSize {

	VertexPosSize() = default;
	VertexPosSize(const VertexPosSize&) = default;
	VertexPosSize& operator=(const VertexPosSize&) = default;
	VertexPosSize(VertexPosSize&&) = default;
	VertexPosSize& operator=(VertexPosSize&&) = default;

	VertexPosSize(const DirectX::XMFLOAT3& _pos, const DirectX::XMFLOAT2& _size) : pos(_pos), size(_size) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 size;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
};


struct VertexPosNormalColor {

	VertexPosNormalColor() = default;
	VertexPosNormalColor(const VertexPosNormalColor&) = default;
	VertexPosNormalColor& operator=(const VertexPosNormalColor&) = default;
	VertexPosNormalColor(VertexPosNormalColor&&) = default;
	VertexPosNormalColor& operator=(VertexPosNormalColor&&) =default;

	VertexPosNormalColor(const DirectX::XMFLOAT3& _pos, const DirectX::XMFLOAT3& _normal, const DirectX::XMFLOAT4& _color) : pos(_pos), normal(_normal), color(_color) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT4 color;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];
};

struct VertexPosNormalTex {

	VertexPosNormalTex() = default;
	VertexPosNormalTex(const VertexPosNormalTex&) = default;
	VertexPosNormalTex(VertexPosNormalTex&&) = default;
	VertexPosNormalTex& operator=(const VertexPosNormalTex&) = default;
	VertexPosNormalTex& operator=(VertexPosNormalTex&&) = default;

	VertexPosNormalTex(const DirectX::XMFLOAT3& _pos, const DirectX::XMFLOAT3& _normal, const DirectX::XMFLOAT2& _tex) : pos(_pos), normal(_normal), tex(_tex) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 tex;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];
};


struct VertexPosNormalTangentTex {

	VertexPosNormalTangentTex() = default;
	VertexPosNormalTangentTex(const VertexPosNormalTangentTex&) = default;
	VertexPosNormalTangentTex(VertexPosNormalTangentTex&&) = default;
	VertexPosNormalTangentTex& operator=(const VertexPosNormalTangentTex&) = default;
	VertexPosNormalTangentTex& operator=(VertexPosNormalTangentTex&&) = default;

	VertexPosNormalTangentTex(const DirectX::XMFLOAT3& _pos, const DirectX::XMFLOAT3& _normal, const DirectX::XMFLOAT4& _tangent, const DirectX::XMFLOAT2& _tex) :
		pos(_pos), normal(_normal), tangent(_tangent), tex(_tex) {}


	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT4 tangent;
	DirectX::XMFLOAT2 tex;

	static const D3D11_INPUT_ELEMENT_DESC inputLayout[4];
};


#endif  // _VERTEX_H_