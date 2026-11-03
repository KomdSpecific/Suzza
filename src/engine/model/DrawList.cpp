#include "DrawList.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	void DrawList::SubmitMesh(Mesh* TargetMesh, const ModelConstant& Constant)
	{
		if (!TargetMesh)
		{
			SUZZA_LOG_ERROR("Mesh指针为空, 以丢弃本次提交!");
			return;
		}

		m_meshQueue[TargetMesh].push_back(Constant);
		m_instanceCount++;
	}

	void DrawList::Clear()
	{
		m_meshQueue.clear();
		m_instanceCount = 0;
	}
}