#define STBI_WINDOWS_UTF8
#include "stb_image.h"
#include "TextureManager.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include "DDSTextureLoader.h"
#include <filesystem>

namespace
{
	// ForwardEffect单例
	static TextureManager* g_pInstance = nullptr;
}

TextureManager::TextureManager()
{
	if (g_pInstance)
		throw std::exception("ForwardEffect is a singleton!");
	g_pInstance = this;
}

TextureManager::~TextureManager()
{
}

TextureManager& TextureManager::Get()
{
	if (!g_pInstance)
		throw std::exception("TextureManager needs an instance!");
	return *g_pInstance;
}

void TextureManager::Init(ID3D11Device* device)
{
	m_pDevice = device;
	m_pDevice->GetImmediateContext(m_pDeviceContext.ReleaseAndGetAddressOf());
}

ID3D11ShaderResourceView* TextureManager::CreateTexture(std::string_view filename, bool enableMips, bool forceSRGB)
{
	size_t fileID = StringToID(filename);
	if (m_TextureSRVs.count(fileID))
		return m_TextureSRVs[fileID].Get();

	auto& res = m_TextureSRVs[fileID];
	std::wstring wstr = UTF8ToWString(filename);
	if (FAILED(DirectX::CreateDDSTextureFromFileEx(m_pDevice.Get(),
		enableMips ? m_pDeviceContext.Get() : nullptr,
		wstr.c_str(), 0, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0, 0, 
		forceSRGB, nullptr, res.GetAddressOf())))
	{
		int width, height, comp;
		
		stbi_uc* img_data = stbi_load(filename.data(), &width, &height, &comp, STBI_rgb_alpha);
		if (img_data)
		{
			CD3D11_TEXTURE2D_DESC texDesc(forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, 
				width, height, 1,
				enableMips ? 0 : 1, 
				D3D11_BIND_SHADER_RESOURCE | (enableMips ? D3D11_BIND_RENDER_TARGET : 0),
				D3D11_USAGE_DEFAULT, 0, 1, 0, 
				enableMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
			Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
			HR(m_pDevice->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf()));
			m_pDeviceContext->UpdateSubresource(tex.Get(), 0, nullptr, img_data, width * sizeof(uint32_t), 0);
			CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, 
				forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
			HR(m_pDevice->CreateShaderResourceView(tex.Get(), &srvDesc, res.GetAddressOf()));
			if (enableMips)
				m_pDeviceContext->GenerateMips(res.Get());
			
			std::string fname = std::filesystem::path(filename).filename().string();
			D3D11SetDebugObjectName(res.Get(), fname);
		}
		stbi_image_free(img_data);
	}

	return res.Get();
}

ID3D11ShaderResourceView* TextureManager::GetTexture(std::string_view filename)
{
	size_t fileID = StringToID(filename);
	if (m_TextureSRVs.count(fileID))
		return m_TextureSRVs[fileID].Get();
	return nullptr;
}
