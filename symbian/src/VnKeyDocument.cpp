#include "VnKeyDocument.h"
#include "VnKeyAppUi.h"

CVnKeyDocument* CVnKeyDocument::NewL(CEikApplication& aApp)
    {
    return new (ELeave) CVnKeyDocument(aApp);
    }

CVnKeyDocument::CVnKeyDocument(CEikApplication& aApp)
    : CAknDocument(aApp)
    {
    }

CEikAppUi* CVnKeyDocument::CreateAppUiL()
    {
    return new (ELeave) CVnKeyAppUi;
    }
