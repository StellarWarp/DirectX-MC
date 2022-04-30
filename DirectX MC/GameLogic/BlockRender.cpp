#include "BlockRender.h"

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

BlockRender* g_pInstance = nullptr;

void BlockRender::SetMesh(ID3D11Device* device,
	const void* vertices, UINT vertexSize, UINT vertexCount,
	const void* indices, UINT indexCount, DXGI_FORMAT indexFormat)
{
	vertexStride = vertexSize;

	vertexCount = vertexCount;
	this->indexCount = indexCount;
	this->indexFormat = indexFormat;

	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = vertexSize * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(device->CreateBuffer(&vbd, &InitData, vertexBuffer.ReleaseAndGetAddressOf()));

	// 设置索引缓冲区描述
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	if (indexFormat == DXGI_FORMAT_R16_UINT)
	{
		ibd.ByteWidth = indexCount * (UINT)sizeof(WORD);
	}
	else
	{
		ibd.ByteWidth = indexCount * (UINT)sizeof(DWORD);
	}
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = indices;
	HR(device->CreateBuffer(&ibd, &InitData, indexBuffer.ReleaseAndGetAddressOf()));

}

BlockRender::BlockRender()
{
	if (g_pInstance)
		throw std::exception("BasicEffect is a singleton!");
	g_pInstance = this;
}

BlockRender* BlockRender::GetInstance()
{
	return g_pInstance;
}

void BlockRender::init(ID3D11Device* device)
{
	static std::vector<std::wstring> texture_path;
	static std::vector<std::wstring> normaltex_path;
	std::vector<BlockType> types;
	std::wstring parent_path = L"..\\Texture\\Blocks\\";
	texture_path.push_back(parent_path + L"default_dirt.png");
	texture_path.push_back(parent_path + L"default_grass.png");
	texture_path.push_back(parent_path + L"default_wood.png");
	texture_path.push_back(parent_path + L"default_tree.png");
	texture_path.push_back(parent_path + L"default_green_leaves.png");
	texture_path.push_back(parent_path + L"default_red_leaves.png");
	texture_path.push_back(parent_path + L"default_yellow_leaves.png");
	texture_path.push_back(parent_path + L"default_stone.png");
	texture_path.push_back(parent_path + L"default_brick.png");
	texture_path.push_back(parent_path + L"default_glass.png");

	normaltex_path.push_back(parent_path + L"normal_rock.jfif");
	normaltex_path.push_back(parent_path + L"normal_sooth.jfif");
	normaltex_path.push_back(parent_path + L"normal_wood.jfif");
	normaltex_path.push_back(parent_path + L"normal_rwood.jfif");
	normaltex_path.push_back(parent_path + L"normal_glass.jfif");
	normaltex_path.push_back(parent_path + L"normal_glass.jfif");
	normaltex_path.push_back(parent_path + L"normal_glass.jfif");
	normaltex_path.push_back(parent_path + L"normal_rock.jfif");
	normaltex_path.push_back(parent_path + L"normal_brick.jfif");
	normaltex_path.push_back(parent_path + L"normal_glass.jfif");



	int blocks = texture_path.size();
	m_iter_array.push_back(BlockType::MapLand);
	m_iter_array.push_back(BlockType::MapGrassland);
	m_iter_array.push_back(BlockType::MapWood);
	m_iter_array.push_back(BlockType::MapTree);
	m_iter_array.push_back(BlockType::MapLeaveGreen);
	m_iter_array.push_back(BlockType::MapLeaveRed);
	m_iter_array.push_back(BlockType::MapLeaveYellow);
	m_iter_array.push_back(BlockType::MapStone);
	m_iter_array.push_back(BlockType::MapRedBrick);
	m_iter_array.push_back(BlockType::MapGlass);


	for (UINT i = 0; i < blocks; i++)
	{
		auto& blockData = m_BlockData[m_iter_array[i]];
		auto& material = blockData.material;
		auto& diffuse = blockData.texDiffuse;
		auto& normalmap = blockData.texNormalMap;
		HR(CreateWICTextureFromFile(device, texture_path[i].c_str(), nullptr, diffuse.GetAddressOf()));
		HR(CreateWICTextureFromFile(device, normaltex_path[i].c_str(), nullptr, normalmap.GetAddressOf()));

		material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		material.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	auto mash = Geometry::CreateBox<VertexPosNormalTangentTex>(1.0f, 1.0f, 1.0f);
	const auto& vertices = mash.vertexVec;
	const auto& indices = mash.indexVec;
	SetMesh(device, vertices.data(), sizeof(VertexPosNormalTangentTex),
		(UINT)vertices.size(), indices.data(), (UINT)indices.size(),
		(sizeof(DWORD) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT));

}

void BlockRender::put(BlockType blocktype, XMINT3 pos)
{
	m_BlockData[blocktype].InstancedData.push_back(Transform(
		XMFLOAT3(1.0f, 1.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(pos.x + 0.0001, pos.y + 0.0001, pos.z + 0.0001)));
}

void BlockRender::reset()
{
	for (auto blocktype : m_iter_array)
		m_BlockData[blocktype].InstancedData.clear();
}

size_t BlockRender::GetCapacity(BlockType blocktype)
{
	return m_BlockData[blocktype].capacity;
}

void BlockRender::ResizeBuffer(ID3D11Device* device, size_t count, BlockType blocktype)
{
	auto& blockData = m_BlockData[blocktype];
	auto& pInstanceBuffer = blockData.InstancedBuffer;
	auto& capacity = blockData.capacity;

	// 设置实例缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = (UINT)count * sizeof(BlockInstancedData);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 创建实例缓冲区
	HR(device->CreateBuffer(&vbd, nullptr,
		m_BlockData[blocktype].InstancedBuffer.ReleaseAndGetAddressOf()));

	// 重新调整Capacity
	capacity = count;
}

void BlockRender::Draw(ID3D11DeviceContext* deviceContext, IEffect* effect)
{
	for (auto blocktype : m_iter_array)
	{
		auto& blockData = m_BlockData[blocktype];
		auto& pInstanceBuffer = blockData.InstancedBuffer;
		auto& Capacity = blockData.capacity;
		auto& data = blockData.InstancedData;

		D3D11_MAPPED_SUBRESOURCE mappedData;
		UINT numInsts = (UINT)data.size();
		// 若传入的数据比实例缓冲区还大，需要重新分配
		if (numInsts > Capacity)
		{
			ComPtr<ID3D11Device> device;
			deviceContext->GetDevice(device.GetAddressOf());
			ResizeBuffer(device.Get(), numInsts, blocktype);
		}
		if (numInsts == 0) continue;
		HR(deviceContext->Map(pInstanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

		BlockInstancedData* iter = reinterpret_cast<BlockInstancedData*>(mappedData.pData);
		for (auto& t : data)
		{
			XMMATRIX W = t.GetMatrixXM_T();
			iter->world = XMMatrixTranspose(W);
			iter->worldInvTranspose = XMMatrixTranspose(XMath::InverseTranspose(W));
			iter++;
		}

		deviceContext->Unmap(pInstanceBuffer.Get(), 0);

		UINT strides[2] = { vertexStride, sizeof(BlockInstancedData) };
		UINT offsets[2] = { 0, 0 };
		ID3D11Buffer* buffers[2] = { nullptr, pInstanceBuffer.Get() };
		buffers[0] = vertexBuffer.Get();

		// 设置顶点/索引缓冲区
		deviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
		deviceContext->IASetIndexBuffer(indexBuffer.Get(), indexFormat, 0);

		// 更新数据并应用
		IEffectTextureDiffuse* pEffectTextureDiffuse = dynamic_cast<IEffectTextureDiffuse*>(effect);
		if (pEffectTextureDiffuse)
		{
			pEffectTextureDiffuse->SetTextureDiffuse(blockData.texDiffuse.Get());
		}

		BasicEffect* pBasicEffect = dynamic_cast<BasicEffect*>(effect);
		if (pBasicEffect)
		{
			pBasicEffect->SetTextureNormalMap(blockData.texNormalMap.Get());
			pBasicEffect->SetMaterial(blockData.material);
		}

		effect->Apply(deviceContext);

		deviceContext->DrawIndexedInstanced(indexCount, numInsts, 0, 0, 0);
	}
}

void BlockRender::SetRenderRange(int range)
{
	m_RenderRange = range;
}

ComPtr<ID3D11ShaderResourceView> BlockRender::GetTexture(BlockType blocktype)
{

	return m_BlockData[blocktype].texDiffuse.Get();
}


void BlockRender::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)

	D3D11SetDebugObjectName(vertexBuffer.Get(), name + "BlockVertexBuffer");
	D3D11SetDebugObjectName(indexBuffer.Get(), name + "BlockIndexBuffer");
	for (auto type : m_iter_array)
	{
		auto& m_pInstancedBuffer = m_BlockData[type].InstancedBuffer;
		if (m_pInstancedBuffer)
		{
			D3D11SetDebugObjectName(m_pInstancedBuffer.Get(), name + ".InstancedBuffer");
		}
	}

#endif
}
