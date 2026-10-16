#include "VnKeyApplication.h"
#include "VnKeyDocument.h"

TUid CVnKeyApplication::AppDllUid() const
    {
    return KUidVnKeyApp;
    }

CApaDocument* CVnKeyApplication::CreateDocumentL()
    {
    return CVnKeyDocument::NewL(*this);
    }
