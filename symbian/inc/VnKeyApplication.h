#ifndef VNKEYAPPLICATION_H
#define VNKEYAPPLICATION_H

#include <aknapp.h>

const TUid KUidVnKeyApp = { 0xE7A10001 };

class CVnKeyApplication : public CAknApplication
    {
public:
    TUid AppDllUid() const;
protected:
    CApaDocument* CreateDocumentL();
    };

#endif // VNKEYAPPLICATION_H
