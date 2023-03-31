/*
    Conversion header by Ryan Tio
    
    Provides various useful conversion macros for converting to units useful for the nanosleep() function.
        
*/
#ifndef rtioconversions
#define rtioconversions

#define NS_PER_SECOND (long long)1000000000
#define NS_PER_MS (long long)1000000
#define MS_PER_SECOND (long long)1000

#define MS_TO_NS(ms) (ms) * NS_PER_MS
#define MS_TO_SEC(ms) (ms) / (double)MS_PER_SECOND

#define SEC_TO_NS(sec) (sec) * NS_PER_SECOND

#define NS_TO_SEC(ns) (ns) / (double)NS_PER_SECOND
#endif