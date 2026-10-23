#pragma once

#include "BaseObject.h"
#include "SuzzaTypes.h"
#include "graphics/Texture.h"
#include "commonmacro/VkMacro.h"

#include <string>
#include <utility>

namespace suz
{
	class Material : public BaseObject
	{
	public:
		Material(const std::string& FilePath = "");
		~Material();

		virtual void Destroy() override;

		void SetMaterialSlot(const uint16& SlotIndex);
		const uint16& GetMaterialSlot() const;

	private:
		uint16 m_materialSlot = MAX_SAMPLED_TEXTURE_IMAGES_SLOTS;

		std::unique_ptr<Texture> m_texture = {};
	};
}