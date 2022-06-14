#ifndef __POSTGRESQL_PROXY_MACROS_H__
#define __POSTGRESQL_PROXY_MACROS_H__

#include <assert.h>

#define BAIL_ON_ERROR(retval) \
    if (retval)               \
    {                         \
        goto error;           \
    }

#define BAIL_ON_ERROR_IF(condition) \
    if (condition)                  \
    {                               \
        goto error;                 \
    }

#define BAIL_WITH_ERROR(retval, err) \
    do                               \
    {                                \
        retval = err;                \
        assert(retval != 0);         \
        goto error;                  \
    } while (0)

#endif
