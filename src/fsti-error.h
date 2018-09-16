#ifndef FSTI_ERROR_H
#define FSTI_ERROR_H

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdatomic.h>

#define FSTI_NAME "FastSTI"
#define FSTI_ERROR_STRING_LEN 200

extern char fsti_error_string[FSTI_ERROR_STRING_LEN + 1];

#define FSTI_MSG(msg1, msg2)                                            \
    strncat(strncpy(fsti_error_string, msg1, FSTI_ERROR_STRING_LEN / 2), \
           msg2, FSTI_ERROR_STRING_LEN / 2)

#define FSTI_ERROR_SET(errnum, msg)					\
	do {					 			\
		fsti_error = errnum;					\
		fsti_error_msg(errnum, __FILE__, __LINE__, msg);	\
	} while (0)

#define FSTI_ERROR(cond, errnum, msg)					\
	(cond) ?							\
	       ( (fsti_error = errnum) &&				\
		       fsti_error_msg(errnum, __FILE__, __LINE__, msg))	\
	       :							\
	       0

#ifndef FSTI_ASSERT
#define FSTI_ASSERT(cond, errnum, msg)                                  \
    do {                                                                \
        if( ((cond) == 0)) {                                            \
            fsti_error_msg(errnum, __FILE__, __LINE__, msg);            \
            exit(1);                                                    \
        }                                                               \
    } while(0)

#endif

enum {
    FSTI_SUCCESS = 0,
    FSTI_FAILURE = -1,
    FSTI_ERR_KEY_FILE_OPEN = 1,
    FSTI_ERR_KEY_FILE_LOAD,
    FSTI_ERR_KEY_FILE_SYNTAX,
    FSTI_ERR_KEY_FILE_GROUP_EMPTY,
    FSTI_ERR_NOMEM,
    FSTI_ERR_THREAD_FAIL,
    FSTI_ERR_CONFIG_ADD,
    FSTI_ERR_NO_VALUE_FOR_KEY,
    FSTI_ERR_KEY_NOT_FOUND,
    FSTI_ERR_OUT_OF_BOUNDS,
    FSTI_ERR_STR_EXPECTED,
    FSTI_ERR_LONG_EXPECTED,
    FSTI_ERR_DBL_EXPECTED,
    FSTI_ERR_WRONG_TYPE,
    FSTI_ERR_EVENT,
    FSTI_ERR_EVENT_NOT_FOUND,
    FSTI_ERR_AGENT_FILE,
    FSTI_ERR_FILE,
    FSTI_ERR_INVALID_VALUE
};

extern atomic_int fsti_error;

extern bool fsti_output_error_messages;

int fsti_error_msg(int errnum, const char *file, int line,
		   const char *extra_message);

#endif
