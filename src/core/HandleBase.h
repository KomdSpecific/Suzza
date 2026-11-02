#pragma once

#include "SuzzaTypes.h"

namespace suz
{
	struct HandleBase
	{
		HandleBase(uint32 Index = 0, uint32 Generation = 0)
			: index(Index), generation(Generation) {
		};

		uint32 index;						// 对象池数组索引
		uint32 generation;					// 世代号

		bool operator==(const HandleBase& Handle) const
		{
			return index == Handle.index && generation == Handle.generation;
		}
	};
}

namespace std
{
	template<>
	struct hash<suz::HandleBase>
	{
		suz::usize operator()(const suz::HandleBase& Handle) const noexcept
		{
			return (static_cast<suz::uint64>(Handle.index) << 32 | Handle.generation);
		}
	};
}