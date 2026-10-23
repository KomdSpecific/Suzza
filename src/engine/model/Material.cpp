#include "Material.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	Material::Material(const std::string& FilePath)
	{
		const VkDevice& device = Engine::GetVulkanResourceManager().GetVulkanInitialize()->GetDevice();
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
		m_texture->Destroy();
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