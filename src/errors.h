#pragma once

#include <stdint.h>
#include "preprocessor.h"

// result code

typedef uint32_t result_code;

#define __RESULT_CODE_1(type)			((result_code) (type))
#define __RESULT_CODE_2(type, value)	((result_code) (((value) << 16) | (type)))

#define RESULT_CODE(...)				VA_SELECT(__RESULT_CODE, __VA_ARGS__)
#define RESULT_CODE_TYPE(err)			((err) & 0x0000FFFF)
#define RESULT_CODE_VALUE(err)			(((err) >> 16) & 0x0000FFFF)

#define IS_SUCCESS_RESULT(result)	(RESULT_CODE_TYPE(result) == 0)
#define IS_ERROR_RESULT(result) 	(RESULT_CODE_TYPE(result) > 0)

// default result codes

#define SUCCESS			RESULT_CODE(0x00000000)
#define FAILURE			RESULT_CODE(0x0000FFFF)

// try/catch

#define TRY \
    result_code __result = 0;

#define __CATCH_0() \
    __catch_block: ; \
    if (IS_ERROR_RESULT(__result))

#define __CATCH_1(err) \
    __catch_block: ; \
    const result_code err = __result; \
    if (IS_ERROR_RESULT(__result))

#define CATCH(...) \
    VA_SELECT(__CATCH, __VA_ARGS__)

#define THROW(err) \
    do { \
        __result = __RESULT_CODE_1(err); \
        goto __catch_block; \
    } while(0)

#define CHECK_RESULT(err) \
    if (IS_ERROR_RESULT(err)) \
    { THROW(err); }
