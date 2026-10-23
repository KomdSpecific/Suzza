#include "ReflectionInfo.h"
#include "macros/ReflectMacros.h"
#include "RegisterAllClass.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	const FieldInfo FieldInfo::CreateFieldInfo(const std::string& Name, const std::string& TypeName, usize Offset, usize TypeSize, Editability EditEnum)
	{
		FieldInfo fieldInfo{};
		fieldInfo.name = Name;
		fieldInfo.typeName = TypeName;
		fieldInfo.offset = Offset;
		fieldInfo.editable = EditEnum;

		fieldInfo.getterFunc = [Offset](void* obj, void* outValue) {
			outValue = (char*)obj + Offset;
			};

		if (EditEnum == Editability::Editable)
		{
			fieldInfo.setterFunc = [Offset, TypeSize](void* obj, void* inValue) {
				void* memberPtr = (char*)obj + Offset;
				memcpy(memberPtr, inValue, TypeSize);
				};
		}

		return fieldInfo;
	}

	void ReflectionSystem::RegisterClassInfo(const std::string& ClassName, const ClassInfo& InClassInfo)
	{
		if (!InClassInfo.IsValid()) return;

		m_reflectionRegistry[ClassName] = InClassInfo;
	}

	const ClassInfo ReflectionSystem::GetReflectionClassInfo(const std::string& ClassName) const
	{
		auto registry = m_reflectionRegistry.find(ClassName);
		if (registry != m_reflectionRegistry.end())
		{
			return registry->second;
		}

		SUZZA_LOG_ERROR("找不到元数据注册的类! ", ClassName);
		return {};
	}

	void ReflectionSystem::RegisterAllReflectionClass()
	{
		SUZZA_REGISTER_ALL_REFLECT_CLASS;
	}
}