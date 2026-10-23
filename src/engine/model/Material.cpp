#include "Material.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"

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