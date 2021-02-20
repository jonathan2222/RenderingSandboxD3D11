#pragma once

#ifdef RS_CONFIG_DEVELOPMENT
#include "comdef.h"
#define RS_D311_ASSERT_CHECK(hr, msg, ...) \
	{ \
		_com_error err(HRESULT_CODE(hr)); \
		const char* errMsg = (const char*)err.ErrorMessage(); \
		const unsigned int SIZE = 512; \
		char str[SIZE]; \
		strcpy_s(str, "[Error: "); \
		strcat_s(str, SIZE, errMsg); \
		strcat_s(str, SIZE, "  (0x{})] "); \
		strcat_s(str, SIZE, msg); \
		RS_ASSERT(SUCCEEDED(hr), str, (unsigned int)hr, __VA_ARGS__); \
	}

#define RS_D311_CHECK(hr, msg) \
	{ \
		_com_error err(HRESULT_CODE(hr)); \
		const char* errMsg = (const char*)err.ErrorMessage(); \
		const unsigned int SIZE = 512; \
		char str[SIZE]; \
		strcpy_s(str, "[Error: "); \
		strcat_s(str, SIZE, errMsg); \
		strcat_s(str, SIZE, "  (0x{})] "); \
		strcat_s(str, SIZE, msg); \
		if(FAILED(hr)) \
			LOG_ERROR(str, (unsigned int)hr); \
	}

#else
#define RS_D311_ASSERT_CHECK(hr, msg, ...) RS_ASSERT(SUCCEEDED(hr), msg, __VA_ARGS__);
#define RS_D311_CHECK(hr, msg, ...)	{ if(FAILED(hr)) LOG_ERROR(msg, __VA_ARGS__); }
#endif