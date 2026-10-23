#pragma once

#define DECLARE_CLASS(THISCLASS_NAME, SUPER_CLASS_NAME) \
	using ThisClass = THISCLASS_NAME; \
	using Super = SUPER_CLASS_NAME;