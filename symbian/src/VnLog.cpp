#include <utf.h>
#include "VnLog.h"

_LIT(KCardFolder, "E:\\VnKey\\");
_LIT(KPhoneFolder, "C:\\Data\\VnKey\\");
_LIT(KLogName, "vnkey.log");
_LIT(KTimeFmt, "%02d:%02d:%02d.%03d ");
_LIT8(KNewLine, "\r\n");

const TInt KMaxLine = 400; // characters; longer lines are cut

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
    // the memory card when there is one, for USB mass storage
    TDriveInfo drive;
    if (iFs.Drive(drive, EDriveE) == KErrNone && drive.iType != EMediaNotPresent
        && iFs.MkDirAll(KCardFolder) != KErrNotReady)
        {
        iFolder.Copy(KCardFolder);
        }
    else
        {
        iFolder.Copy(KPhoneFolder);
        }
    TInt err = iFs.MkDirAll(iFolder);
    if (err != KErrNone && err != KErrAlreadyExists)
        {
        return; // no log, the app goes on
        }
    TFileName name(iFolder);
    name.Append(KLogName);
    err = iFile.Open(iFs, name, EFileWrite | EFileShareAny | EFileStreamText);
    if (err == KErrNotFound)
        {
        err = iFile.Create(iFs, name, EFileWrite | EFileShareAny | EFileStreamText);
        }
    if (err == KErrNone)
        {
        TInt end = 0;
        iFile.Seek(ESeekEnd, end);
        iOpen = ETrue;
        }
    }

CVnLog::~CVnLog()
    {
    if (iOpen)
        {
        iFile.Close();
        }
    iFs.Close();
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
    TBuf<KMaxLine + 16> line;
    line.Format(KTimeFmt, t.Hour(), t.Minute(), t.Second(), t.MicroSecond() / 1000);
    line.Append(aLine.Left(KMaxLine));
    TBuf8<(KMaxLine + 16) * 3 + 2> utf8;
    if (CnvUtfConverter::ConvertFromUnicodeToUtf8(utf8, line) != KErrNone)
        {
        return;
        }
    utf8.Append(KNewLine);
    iFile.Write(utf8);
    iFile.Flush();
    }
