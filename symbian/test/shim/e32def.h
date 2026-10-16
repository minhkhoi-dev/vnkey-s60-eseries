#ifndef E32DEF_SHIM_H
#define E32DEF_SHIM_H

// Stand-in for the Symbian header of the same name, so that the pure logic in symbian/src
// (VnTelex.cpp) builds with the host's compiler. Test only: this folder is not part of the
// app (no .mmp names it).

#include <stddef.h>

typedef int TInt;
typedef unsigned int TUint;
typedef unsigned char TUint8;
typedef unsigned short TUint16;
typedef unsigned int TUint32;
typedef int TBool;
typedef TUint16 TText;

#define ETrue 1
#define EFalse 0

#endif
