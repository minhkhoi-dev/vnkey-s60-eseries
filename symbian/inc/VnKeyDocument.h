#ifndef VNKEYDOCUMENT_H
#define VNKEYDOCUMENT_H

#include <akndoc.h>

class CVnKeyDocument : public CAknDocument
    {
public:
    static CVnKeyDocument* NewL(CEikApplication& aApp);
    CEikAppUi* CreateAppUiL();
private:
    CVnKeyDocument(CEikApplication& aApp);
    };

#endif // VNKEYDOCUMENT_H
