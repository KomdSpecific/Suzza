#include "Material.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	Material::Material(const std::string& FilePath)
	{
		const VkDevice& device = Engine::GetVulkanResourceManager().GetDevice()->GetDevice();
		m_texture = std::make_unique<Texture>(device);

		m_texture->InitTexture(FilePath);
	}

	Material::~Material()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class Material Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	void Material::Destroy()
	{
		if (!(*m_isAlive)) return;

		m_texture->Destroy();

		*m_isAlive = false;
	}

	void Material::SetMaterialSlot(const uint16& SlotIndex)
	{
		m_materialSlot = SlotIndex;
		/* 通知纹理使用插槽更改 */
		m_texture->UpdateTextureSlot(SlotIndex);
	}

	const uint16& Material::GetMaterialSlot() const
	{
		return m_materialSlot;
	}
}