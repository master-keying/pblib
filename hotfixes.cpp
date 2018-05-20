/*
  This is a workaround for the issue described here:
  https://reviews.llvm.org/D37830
  With the fix, Travis can build the code.
*/
#ifdef PBLIB_OSX_VIRTUAL_DESTRUCTOR_WORKAROUND
#include <stdlib.h>
extern "C" void __cxa_deleted_virtual()
{
    abort();
}
#endif
