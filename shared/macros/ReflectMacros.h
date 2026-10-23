#pragma once

#include "SuzzaTypes.h"

#define SUZZA_REGISTER_ALL_REFLECT_CLASS

#define SUZZA_OFFSET_OF(ClassMember) suz::OffsetOf(&ClassMember)

namespace suz
{
	template<typename ClassT, typename MemberT>
	constexpr usize OffsetOf(MemberT ClassT::* memberPtr) noexcept
	{
		/* 把0转为类指针const ClassT*, 假设该对象放在内存地址0处 */
		const ClassT* obj_ptr = reinterpret_cast<const ClassT*>(0);
		/* 使用成员指针运算符访问类里面的成员 */
		const MemberT& member = obj_ptr->*memberPtr;
		/* 拿到成员的偏移地址并返回 */
		return reinterpret_cast<usize>(&member);
	}
}


#define SUZZA_MACRO_COMBINED_INNER(A, B, C, D) A##B##C##D
#define SUZZA_MACRO_COMBINED(A, B, C, D) SUZZA_MACRO_COMBINED_INNER(A, B, C, D)


#define SUZZA_REFLECT_CLASS() SUZZA_MACRO_COMBINED(SUZZA_GENERATED_NAME, _, __LINE__, _REFLECT_BODY)


#if defined(SUZZA_REFLECT_PARSING) || defined(__clang__)
#define SUZZA_FUNCTION(Property, ...) __attribute__((annotate("function:", #Property, #__VA_ARGS__)))
#define SUZZA_PROPERTY(...) __attribute__((annotate("property:", #__VA_ARGS__)))
#else
#define SUZZA_FUNCTION(Property, ...)
#define SUZZA_PROPERTY(...)
#endif
