#pragma once

#include "SuzzaTypes.h"
#include "HandleBase.h"

namespace suz
{
	template<typename Obj>
	class ObjectPoolBase
	{
	public:
		ObjectPoolBase() = default;

		template<typename... Args>
		HandleBase Allocate(Args&&...args)
		{
			uint32 index = 0;
			if (!m_freeIndex.empty())
			{
				/* 优先分配释放过的插槽 */
				index = m_freeIndex.back();
				m_freeIndex.pop_back();
			}
			else
			{
				/* 无空闲, 直接扩容 */
				index = static_cast<uint32>(m_objSlots.size());
				m_objSlots.emplace_back();
			}

			ObjectInfo& objInfo = m_objSlots[index];
			objInfo.isAlive = true;
			objInfo.generation += 1;
			new (&objInfo.obj) Obj{ std::forward<Args>(args)... };

			return { index,objInfo.generation };
		}

		void Free(HandleBase ObjHandle)
		{
			if (!CheckValid(ObjHandle))
				return;

			ObjectInfo& objInfo = m_objSlots[ObjHandle.index];
			
			objInfo.isAlive = false;
			objInfo.generation += 1;
			objInfo.obj.~Obj();

			/* 将释放的槽放回空闲列表 */
			m_freeIndex.push_back(ObjHandle.index);
		}

		Obj* Find(HandleBase ObjHandle)
		{
			if (!CheckValid(ObjHandle))
				return nullptr;

			ObjectInfo& objInfo = m_objSlots[ObjHandle.index];
			return &objInfo.obj;
		}

		bool CheckValid(HandleBase ObjHandle)
		{
			if (ObjHandle.index >= m_objSlots.size())
				return false;

			ObjectInfo& objInfo = m_objSlots[ObjHandle.index];
			return objInfo.isAlive && (ObjHandle.generation == objInfo.generation);
		}

		void Clear()
		{
			for (uint32 i = 0; i < m_objSlots.size(); i++)
			{
				ObjectInfo& objInfo = m_objSlots[i];
				if (!objInfo.isAlive)
					continue;

				objInfo.isAlive = false;
				objInfo.generation += 1;
				objInfo.obj.~Obj();

				m_freeIndex.push_back(i);
			}
		}

	protected:
		struct ObjectInfo
		{
			bool isAlive = false;
			uint32 generation = 0;
			Obj obj;
		};

		Vector<ObjectInfo> m_objSlots = {};
		Vector<uint32> m_freeIndex = {};					// 记录被释放过的插槽
	};


	template<typename Ptr>
	class PointerPoolBase
	{
	public:
		PointerPoolBase() = default;

		HandleBase Allocate(Ptr* ObjPtr)
		{
			uint32 index = 0;
			if (!m_freeIndices.empty())
			{
				index = m_freeIndices.back();
				m_freeIndices.pop_back();
			}
			else
			{
				index = static_cast<uint32>(m_ptrSlots.size());
				m_ptrSlots.emplace_back();
			}

			ObjectPtrInfo& ptrInfo = m_ptrSlots[index];
			ptrInfo.isAlive = true;
			ptrInfo.generation += 1;
			ptrInfo.ptr = ObjPtr;

			return { index,ptrInfo.generation };
		}

		void Free(HandleBase ObjHandle)
		{
			if (!CheckValid(ObjHandle))
				return;

			ObjectPtrInfo& ptrInfo = m_ptrSlots[ObjHandle.index];
			ptrInfo.isAlive = false;
			ptrInfo.generation += 1;

			delete ptrInfo.ptr;
			ptrInfo.ptr = nullptr;

			m_freeIndices.push_back(ObjHandle.index);
		}

		Ptr* Find(HandleBase ObjHandle)
		{
			if (!CheckValid(ObjHandle))
				return nullptr;

			ObjectPtrInfo& ptrInfo = m_ptrSlots[ObjHandle.index];
			return ptrInfo.ptr;
		}

		bool CheckValid(HandleBase ObjHandle)
		{
			if (ObjHandle.index >= m_ptrSlots.size())
				return false;

			ObjectPtrInfo& ptrInfo = m_ptrSlots[ObjHandle.index];
			return ptrInfo.isAlive && (ptrInfo.generation == ObjHandle.generation) && ptrInfo.ptr;
		}

		void Clear()
		{
			for (uint32 i = 0; i < m_ptrSlots.size(); i++)
			{
				ObjectPtrInfo& ptrInfo = m_ptrSlots[i];
				if (!ptrInfo.isAlive)
					continue;

				ptrInfo.isAlive = false;
				ptrInfo.generation += 1;

				delete ptrInfo.ptr;
				ptrInfo.ptr = nullptr;

				m_freeIndices.push_back(i);
			}
		}

	protected:
		struct ObjectPtrInfo
		{
			bool isAlive = false;
			uint32 generation = 0;
			Ptr* ptr = nullptr;
		};

		Vector<ObjectPtrInfo> m_ptrSlots = {};
		Vector<uint32> m_freeIndices = {};
	};
}