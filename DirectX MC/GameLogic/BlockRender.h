#pragma once
#include"d3dUtil.h"
#include <DXTrace.h>
//#include"TextureManager.h"
#include"BlockMaterial.h"
#include"GameObject.h"
#include"Block.h"

using namespace DirectX;

enum class BlockType;
class BlockRender;

struct BlockInstancedData
{
	XMMATRIX world;
	XMMATRIX worldInvTranspose;
};

class BlockRender
{
	void SetMesh(ID3D11Device* device,
		const void* vertices, UINT vertexSize, UINT vertexCount,
		const void* indices, UINT indexCount, DXGI_FORMAT indexFormat);

public:
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	int m_RenderRange = 20;
	XMFLOAT3 direction{};
	XMINT3 m_RenderCenter{};
	bool m_clipSpace = true;

	BlockRender();
	static BlockRender* GetInstance();
	void init(ID3D11Device* device);

	void put(BlockType blocktype, XMINT3 pos);

	void reset();

	size_t GetCapacity(BlockType blocktype);

	void ResizeBuffer(ID3D11Device* device, size_t count, BlockType blocktype);

	void Draw(ID3D11DeviceContext* deviceContext, IEffect* effect);

	void SetRenderRange(int range);

	ComPtr<ID3D11ShaderResourceView> GetTexture(BlockType blocktype);

	void SetDebugObjectName(const std::string& name);

private:
	struct BlockData
	{
		Material material;
		ComPtr<ID3D11ShaderResourceView> texDiffuse;
		ComPtr<ID3D11ShaderResourceView> texNormalMap;
		std::vector<Transform> InstancedData;
		ComPtr<ID3D11Buffer> InstancedBuffer;
		size_t capacity = 0;
	};
	std::unordered_map <BlockType, BlockData>m_BlockData;
	std::vector<BlockType> m_iter_array;

	//资源
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	UINT vertexCount = 0;
	UINT indexCount = 0;
	UINT vertexStride = 0;
	DXGI_FORMAT indexFormat;
};

