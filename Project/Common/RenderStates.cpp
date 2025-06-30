#include "RenderStates.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace Microsoft::WRL;

ComPtr<ID3D11RasterizerState>RenderStates::RSWireFrame = nullptr;
ComPtr<ID3D11RasterizerState> RenderStates::RSNoCull = nullptr;
ComPtr<ID3D11RasterizerState> RenderStates::RSCullClockWise = nullptr;

ComPtr<ID3D11SamplerState> RenderStates::SSLinearWrap = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSAnisotropicWrap = nullptr;

ComPtr<ID3D11BlendState> RenderStates::BSNoColorWrite = nullptr;
ComPtr<ID3D11BlendState> RenderStates::BSTransparent = nullptr;
ComPtr<ID3D11BlendState> RenderStates::BSAlphaToCoverage = nullptr;
ComPtr<ID3D11BlendState> RenderStates::BSAdditive = nullptr;

ComPtr<ID3D11DepthStencilState> RenderStates::DSSWriteStencil = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSDrawWithStencil = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDoubleBlend = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDepthTest = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDepthWrite = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDepthTestWithStencil = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDepthWriteWithStencil =nullptr;

bool RenderStates::IsInit()
{
	return RSWireFrame != nullptr;
}

void RenderStates::InitAll(ID3D11Device* device)
{
	if(IsInit())
		return;
	
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&rasterizerDesc, RSWireFrame.GetAddressOf()));

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&rasterizerDesc, RSNoCull.GetAddressOf()));

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&rasterizerDesc, RSCullClockWise.GetAddressOf()));

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MaxAnisotropy = 4;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HR(device->CreateSamplerState(&sampDesc, SSLinearWrap.GetAddressOf()));

	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MaxAnisotropy = 4;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HR(device->CreateSamplerState(&sampDesc, SSAnisotropicWrap.GetAddressOf()));


	D3D11_BLEND_DESC bs;
	ZeroMemory(&bs, sizeof(bs));

	auto& rtDesc = bs.RenderTarget[0];

	bs.AlphaToCoverageEnable = true;
	bs.IndependentBlendEnable = false;
	rtDesc.BlendEnable = false;
	rtDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&bs, BSAlphaToCoverage.GetAddressOf()));

    bs.AlphaToCoverageEnable = false;
    bs.IndependentBlendEnable = false;
    rtDesc.BlendEnable = true;
	rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	HR(device->CreateBlendState(&bs, BSTransparent.GetAddressOf()));

    rtDesc.SrcBlend = D3D11_BLEND_ONE;
    rtDesc.DestBlend = D3D11_BLEND_ONE;
    rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
    rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	HR(device->CreateBlendState(&bs, BSAdditive.GetAddressOf()));

	rtDesc.BlendEnable = false;
    rtDesc.SrcBlend = D3D11_BLEND_ZERO;
    rtDesc.DestBlend = D3D11_BLEND_ONE;
    rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
    rtDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
    rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
    rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtDesc.RenderTargetWriteMask = 0;

	HR(device->CreateBlendState(&bs, BSNoColorWrite.GetAddressOf()));


	D3D11_DEPTH_STENCIL_DESC ds;
	ZeroMemory(&ds,sizeof(ds));

	ds.DepthEnable = true;
	ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	ds.DepthFunc = D3D11_COMPARISON_LESS;
	ds.StencilEnable = true;
	ds.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	ds.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	ds.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	ds.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	ds.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	ds.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ds.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	ds.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	ds.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	ds.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(device->CreateDepthStencilState(&ds, DSSWriteStencil.GetAddressOf()));

    ds.DepthEnable = true;
    ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    ds.DepthFunc = D3D11_COMPARISON_LESS;
    ds.StencilEnable = true;
    ds.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    ds.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    ds.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    ds.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&ds, DSSDrawWithStencil.GetAddressOf()));

    ds.DepthEnable = true;
    ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    ds.DepthFunc = D3D11_COMPARISON_LESS;
    ds.StencilEnable = true;
    ds.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    ds.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    ds.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    ds.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    ds.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    ds.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&ds, DSSNoDoubleBlend.GetAddressOf()));

    ds.DepthEnable = false;
	ds.StencilEnable = false;

	HR(device->CreateDepthStencilState(&ds, DSSNoDepthTest.GetAddressOf()));

    ds.StencilEnable = true;
    ds.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    ds.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    ds.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    ds.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&ds, DSSNoDepthTestWithStencil.GetAddressOf()));

    ds.DepthEnable = true;
    ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    ds.DepthFunc = D3D11_COMPARISON_LESS;
    ds.StencilEnable = false;

	HR(device->CreateDepthStencilState(&ds, DSSNoDepthWrite.GetAddressOf()));

    ds.StencilEnable = true;
    ds.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    ds.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    ds.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    ds.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    ds.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    ds.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&ds, DSSNoDepthWriteWithStencil.GetAddressOf()));

    D3D11SetDebugObjectName(RSWireFrame.Get(), "RSWireFrame");
    D3D11SetDebugObjectName(RSNoCull.Get(), "RSNoCull");
    D3D11SetDebugObjectName(RSCullClockWise.Get(), "RSCullClockWise");
    D3D11SetDebugObjectName(SSLinearWrap.Get(), "SSLinearWrap");
    D3D11SetDebugObjectName(SSAnisotropicWrap.Get(), "SSAnisotropicWrap");
    D3D11SetDebugObjectName(BSNoColorWrite.Get(), "BSNoColorWrite");
    D3D11SetDebugObjectName(BSTransparent.Get(), "BSTransparent");
    D3D11SetDebugObjectName(BSAlphaToCoverage.Get(), "BSAlphaToCoverage");
    D3D11SetDebugObjectName(BSAdditive.Get(), "BSAdditive");
    D3D11SetDebugObjectName(DSSWriteStencil.Get(), "DSSWriteStencil");
    D3D11SetDebugObjectName(DSSDrawWithStencil.Get(), "DSSDrawWithStencil");
    D3D11SetDebugObjectName(DSSNoDoubleBlend.Get(), "DSSNoDoubleBlend");
    D3D11SetDebugObjectName(DSSNoDepthTest.Get(), "DSSNoDepthTest");
    D3D11SetDebugObjectName(DSSNoDepthTestWithStencil.Get(), "DSSNoDepthTestWithStencil");
    D3D11SetDebugObjectName(DSSNoDepthWrite.Get(), "DSSNoDepthWrite");
    D3D11SetDebugObjectName(DSSNoDepthWriteWithStencil.Get(), "DSSNoDepthWriteWithStencil");
}