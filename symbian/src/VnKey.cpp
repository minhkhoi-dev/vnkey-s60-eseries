// VnKey.cpp - process entry point
#include <eikstart.h>
#include "VnKeyApplication.h"

LOCAL_C CApaApplication* NewApplication()
    {
    return new CVnKeyApplication;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }
