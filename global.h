#ifndef _global
#define _global


typedef unsigned char BYTE_T;
typedef unsigned int SIZE_T;
typedef int ERROR_T;


// Shared by all
const ERROR_T ERROR_NOERROR=0;
const ERROR_T ERROR_GENERAL=-1;
const ERROR_T ERROR_NOMEM=-2;
const ERROR_T ERROR_NOSUCHBLOCK=-3;
const ERROR_T ERROR_WRONGSIZEBLOCK=-4;
const ERROR_T ERROR_NOFETCH=-5;
const ERROR_T ERROR_NOTANINDEX=-6;
const ERROR_T ERROR_NOSPACE=-7;
const ERROR_T ERROR_CONFLICT=-8;
const ERROR_T ERROR_NONEXISTENT=-9;
const ERROR_T ERROR_SIZE=-10;
const ERROR_T ERROR_IMPLBUG=-11;
const ERROR_T ERROR_BADCONFIG=-12;
const ERROR_T ERROR_NOFILE=-13;
const ERROR_T ERROR_UNIMPL=-14;
const ERROR_T ERROR_INSANE=-15;

struct GenericException {};


// The following two are used to print allocation sanity checks
// The disk info includes a private allocation bitmap
// that is modified through advisory functions.  Unfortunately,
// students often get the use of the advisory functions wrong,
// resulting in lots of errors they can't figure out.  Hence
// they default to off.
#define PRINT_DISKSYSTEM_ALLOCATION_ERRORS 0
#define PRINT_BUFFERCACHE_ALLOCATION_ERRORS 0

#endif
