#pragma once

#include "RenderTypes.h"

namespace suz
{
	class Mesh;

	class DrawList
	{
	public:
		DrawList() = default;

		void SubmitMesh(Mesh* TargetMesh, const ModelConstant& Constant);

		const bool IsEmpty() const { return m_meshQueue.empty(); }

		void Clear();

		/* 获取队列列表 */
		inline const UnorderedMap<Mesh*, Vector<ModelConstant>>& GetDrawQueue() const { return m_meshQueue; }
		/* 每帧提交的实例统计 */
		inline const uint32 GetInstanceCount() const { return m_instanceCount; }

	private:
		UnorderedMap<Mesh*, Vector<ModelConstant>> m_meshQueue = {};
		uint32 m_instanceCount = 0;								// 每帧提交的实例统计
	};
}