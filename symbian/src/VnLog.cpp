#include <utf.h>
#include "VnLog.h"
#include "VnShared.h"

_LIT(KLogName, "vnkey.log");
_LIT(KTimeFmt, "%02d:%02d:%02d.%03d ");
_LIT8(KNewLine, "\r\n");

CVnLog* CVnLog::NewL()
    {
    CVnLog* self = new (ELeave) CVnLog;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void CVnLog::ConstructL()
    {
    User::LeaveIfError(iFs.Connect());
    VnFiles::Folder(iFs, iFolder);
    TFileName name(iFolder);
    name.Append(KLogName);
    TInt err = iFile.Open(iFs, name, EFileWrite | EFileShareAny | EFileStreamText);
    if (err == KErrNotFound)
        {
        err = iFile.Create(iFs, name, EFileWrite | EFileShareAny | EFileStreamText);
        }
    iOpen = err == KErrNone; // without it the app goes on, with no log
    }

CVnLog::~CVnLog()
    {
    if (iOpen)
        {
        iFile.Close();
        }
    iFs.Close();
    }

HBufC8* CVnLog::ContentL(TInt aMax)
    {
    if (!iOpen)
        {
        User::Leave(KErrNotReady);
        }
    TInt size = 0;
    User::LeaveIfError(iFile.Size(size));
    const TInt length = Min(size, aMax);
    HBufC8* content = HBufC8::NewLC(length);
    TPtr8 ptr(content->Des());
    User::LeaveIfError(iFile.Read(size - length, ptr, length));
    CleanupStack::Pop(content);
    return content;
    }

void CVnLog::Clear()
    {
    if (iOpen)
        {
        iFile.SetSize(0);
        TInt start = 0;
        iFile.Seek(ESeekStart, start);
        }
    }

void CVnLog::SetPrefix(const TDesC& aPrefix)
    {
    iPrefix.Copy(aPrefix.Left(KMaxPrefix));
    }

void CVnLog::Write(const TDesC& aLine)
    {
    if (!iOpen)
        {
        return;
        }
    TTime now;
    now.HomeTime();
    const TDateTime t = now.DateTime();
    iLine.Format(KTimeFmt, t.Hour(), t.Minute(), t.Second(), t.MicroSecond() / 1000);
    iLine.Append(iPrefix);
    iLine.Append(aLine.Left(KMaxLine));
    if (CnvUtfConverter::ConvertFromUnicodeToUtf8(iUtf8, iLine) != KErrNone)
        {
        return;
        }
    iUtf8.Append(KNewLine);
    TInt end = 0;
    iFile.Seek(ESeekEnd, end); // other applications write to it too
    iFile.Write(iUtf8);
    iFile.Flush();
    }
