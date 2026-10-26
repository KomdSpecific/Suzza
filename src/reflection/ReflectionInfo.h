#pragma once

#include "SuzzaTypes.h"
#include "ContainerAccessor.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <type_traits>

namespace suz
{
	enum class ETypeCategory : uint8
	{
		Primitive,
		Enum,
		Struct,
		Array,
		Pointer,
		Reference,
		Container,
		Unknown
	};

	enum class EContainerType : uint8
	{
		Vector3,
		Vector4,
		Quat,
		Matrix3,
		Matrix4,
		String,
		Vector,
		Map,
		Set,
		Unknown
	};

	enum class EPrimitiveType : uint8
	{
		Bool,
		Int8, UInt8,
		Int16, UInt16,
		Int32, UInt32,
		Int64, UInt64,
		Float,
		Unknown
	};

	enum class EFunctionType : uint8
	{
		GetterFunc = 1 << 0,
		SetterFunc = 1 << 1
	};

	enum class Editability : uint8
	{
		Editable = 1 << 0,
		ReadOnly = 1 << 1
	};

	struct FieldInfo
	{
		std::string name{};							// 变量名
		std::string typeName{};						// 类型名(如果类型是别名则显示类型别名)
		std::string parentClassName{};				// 父类名(如果字段在父类被定义则不为空)
		usize offset;								// 类内偏移量
		Editability editable;						// 是否可更改

		std::function<void(void*, void*)> getterFunc;
		std::function<void(void*, void*)> setterFunc;

		ETypeCategory category = ETypeCategory::Unknown;				// 变量分类
		EPrimitiveType primitiveType = EPrimitiveType::Unknown;			// 变量具体类型(仅当category==Primitive才有效)
		EContainerType containerType = EContainerType::Unknown;			// 容器类型(仅当category==Container才有效)

		std::shared_ptr<IContainerAccessor> accessor = nullptr;

		const bool IsValid() const
		{
			return !name.empty() && !typeName.empty() && getterFunc;
		}

		/* 偏移量方案构造 */
		static FieldInfo CreateFieldInfo(const std::string& Name, const std::string& TypeName, usize Offset, usize TypeSize, Editability EditEnum);
		/* 类成员函数构造 */
		template<typename ClassT, typename MemberT>
		static FieldInfo CreateFieldInfo(
			const std::string& Name,
			const std::string& TypeName,
			const std::string& ParentClassName,
			const MemberT& (ClassT::* GetterFunc)() const,
			void(ClassT::* SetterFunc)(const MemberT&),
			Editability EditEnum
		);

		template<typename ClassT, typename MemberT>
		static FieldInfo CreateFieldInfo(
			const std::string& Name,
			const std::string& TypeName,
			const std::string& ParentClassName,
			const MemberT (ClassT::* GetterFunc)() const,
			void(ClassT::* SetterFunc)(const MemberT&),
			Editability EditEnum
		);
	};

	struct ClassInfo
	{
		std::string className;
		std::string classPath;
		std::vector<std::string> superClassName;
		std::vector<FieldInfo> fields;

		const bool IsValid() const
		{
			return !className.empty() && !classPath.empty() && !fields.empty();
		}
	};


	class ReflectionSystem
	{
	public:
		ReflectionSystem() = default;

		void RegisterClassInfo(const std::string& ClassName, const ClassInfo& InClassInfo);

		static void RegisterAllReflectionClass();

	private:
		ReflectionSystem(ReflectionSystem&) = delete;
		ReflectionSystem(const ReflectionSystem&) = delete;
		ReflectionSystem& operator=(ReflectionSystem&) = delete;
		ReflectionSystem& operator=(const ReflectionSystem&) = delete;

		std::unordered_map<std::string, ClassInfo> m_reflectionRegistry = {};

	public:
		inline const std::unordered_map<std::string, ClassInfo>& GetReflectionRegistry() const { return m_reflectionRegistry; }
		const ClassInfo GetReflectionClassInfo(const std::string& ClassName) const;
	};



	template<typename ClassT, typename MemberT>
	inline FieldInfo FieldInfo::CreateFieldInfo(
		const std::string& Name,
		const std::string& TypeName,
		const std::string& ParentClassName,
		const MemberT& (ClassT::* GetterFunc)() const,
		void(ClassT::* SetterFunc)(const MemberT&),
		Editability EditEnum
	)
	{
		static_assert(std::is_same_v<std::remove_cv_t<MemberT>, MemberT>, "MemberT不能带 const/volatile, 请传字段本身的类型!");

		FieldInfo fieldInfo{};
		fieldInfo.name = Name;
		fieldInfo.typeName = TypeName;
		fieldInfo.parentClassName = ParentClassName;
		fieldInfo.editable = EditEnum;

		fieldInfo.getterFunc = [GetterFunc](void* obj, void* outValue) {
			ClassT* classObj = static_cast<ClassT*>(obj);
			*static_cast<MemberT*>(outValue) = (classObj->*GetterFunc)();
			};

		if (EditEnum == Editability::Editable)
		{
			fieldInfo.setterFunc = [SetterFunc](void* obj, void* inValue) {
				ClassT* classObj = static_cast<ClassT*>(obj);
				MemberT member = *static_cast<MemberT*>(inValue);
				(classObj->*SetterFunc)(member);
				};
		}

		return fieldInfo;
	}

	template<typename ClassT, typename MemberT>
	inline FieldInfo FieldInfo::CreateFieldInfo(
		const std::string& Name, const std::string& TypeName, const std::string& ParentClassName,
		const MemberT(ClassT::* GetterFunc)() const,
		void(ClassT::* SetterFunc)(const MemberT&),
		Editability EditEnum
	)
	{
		FieldInfo fieldInfo{};
		fieldInfo.name = Name;
		fieldInfo.typeName = TypeName;
		fieldInfo.parentClassName = ParentClassName;
		fieldInfo.editable = EditEnum;

		fieldInfo.getterFunc = [GetterFunc](void* obj, void* outValue) {
			const ClassT* classObj = static_cast<const ClassT*>(obj);
			*static_cast<MemberT*>(outValue) = (classObj->*GetterFunc)();
			};

		if (EditEnum == Editability::Editable)
		{
			fieldInfo.setterFunc = [SetterFunc](void* obj, void* inValue) {
				ClassT* classObj = static_cast<ClassT*>(obj);
				MemberT member = *static_cast<MemberT*>(inValue);
				(classObj->*SetterFunc)(member);
				};
		}

		return fieldInfo;
	}
}