#if defined(_MSC_VER)
#define DO_PRAGMA(X) __pragma(#X)
#define DISABLE_WARNINGS_PUSH           DO_PRAGMA(warning( push, 1 )) DO_PRAGMA(warning(disable : 4371 4365 4626 5027 4100 4946 4371 5267))	    
#define DISABLE_WARNINGS_POP            DO_PRAGMA(warning( pop )) 
// other warnings you want to deactivate...

#elif defined(__GNUC__) || defined(__clang__)
#define DO_PRAGMA(X) _Pragma(#X)
#define DISABLE_WARNINGS_PUSH           DO_PRAGMA(GCC diagnostic push) DO_PRAGMA(GCC diagnostic ignored "-Wpedantic")
#define DISABLE_WARNINGS_POP            DO_PRAGMA(GCC diagnostic pop) 
// other warnings you want to deactivate... 

#else
#define DISABLE_WARNINGS_PUSH
#define DISABLE_WARNINGS_POP
// other warnings you want to deactivate... 

#endif