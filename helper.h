#ifndef _HELPER
#define _HELPER

#define bail_assert(s, error_code)					\
	do {											\
		if ( !(s) ) {								\
            ret = error_code;                       \
    		fprintf(								\
				stderr,								\
				"Error in %s at %d with code %d.",	\
				__FILE__,							\
				__LINE__,							\
				error_code);						\
			goto exit;								\
		}											\
	} while (0)

#endif