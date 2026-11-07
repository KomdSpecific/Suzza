#pragma once

#if defined(_DEBUG) || PRODUCTION_BUILD == 0
#define SUZZA_DEBUG
#else
#define SUZZA_RELEASE
#endif