#ifndef _LIGHT_HELPER_H_
#define _LIGHT_HELPER_H_

#include <DirectXMath.h>

struct DirectionalLight {
	DirectionalLight() = default;

	DirectionalLight(const DirectionalLight&) = default;
	DirectionalLight& operator=(const DirectionalLight&)  = default;

	DirectionalLight(DirectionalLight&&) = default;
	DirectionalLight& operator=(DirectionalLight&&) = default;

	DirectionalLight(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, 
		const DirectX::XMFLOAT4& _specular, const DirectX::XMFLOAT3& _direction ) :
	    ambient(_ambient),
		diffuse(_diffuse),
		specular(_specular),
		direction(_direction),
		pad() {}

	
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	DirectX::XMFLOAT3 direction;
	float pad;
};



struct PointLight {

	PointLight() = default;

	PointLight(const PointLight&) = default;
	PointLight& operator= (const PointLight&) = default;
	PointLight(PointLight&&) = default;
	PointLight& operator=(PointLight&&) = default;

    PointLight(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, const DirectX::XMFLOAT4& _specular,
            const DirectX::XMFLOAT3& _position, float _range,  const DirectX::XMFLOAT3& _attr) : 
			ambient(_ambient),
			diffuse(_diffuse),
			specular(_specular),
			position(_position),
			range(_range),
			attr(_attr),
			pad() {
			}

    DirectX::XMFLOAT4 ambient;
    DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;

	DirectX::XMFLOAT3 position;
	float range;

	DirectX::XMFLOAT3 attr;
	float pad;

};

struct SpotLight {

	SpotLight() = default;

	SpotLight(const SpotLight&) = default;
	SpotLight& operator=(const SpotLight&) = default;
	SpotLight(SpotLight&&) = default;
	SpotLight& operator=(SpotLight&&) = default;

    SpotLight(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, const DirectX::XMFLOAT4& _specular,
            const DirectX::XMFLOAT3& _position, float _range, const DirectX::XMFLOAT3& _direction, float _spot,
            const DirectX::XMFLOAT3& _attr)
            : ambient(_ambient),
              diffuse(_diffuse),
              specular(_specular),
              position(_position),
              range(_range),
			  direction(_direction),
			  spot(_spot),
              attr(_attr),
              pad() {}

    DirectX::XMFLOAT4 ambient;
    DirectX::XMFLOAT4 diffuse;
    DirectX::XMFLOAT4 specular;

    DirectX::XMFLOAT3 position;
    float range;

    DirectX::XMFLOAT3 direction;
    float spot;

    DirectX::XMFLOAT3 attr;
    float pad;
};

struct Material {

	Material() = default;
	
	Material(const Material&) = default;
	Material& operator=(const Material&) = default;
	Material(Material&&) = default;
	Material& operator=(Material&&) = default;

	Material(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, const DirectX::XMFLOAT4& _specular, const DirectX::XMFLOAT4& _reflect) :
		ambient(_ambient),
		diffuse(_diffuse),
		specular(_specular),
		reflect(_reflect) {}

	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	DirectX::XMFLOAT4 reflect;
};

#endif  // _LIGHT_HELPER_H_