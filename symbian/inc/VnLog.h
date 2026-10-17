#ifndef VNLOG_H
#define VNLOG_H

#include <e32base.h>
#include <f32file.h>

// The test log: one line per call, with the time, in UTF-8, appended to vnkey.log in
// E:\VnKey\ on the memory card (C:\Data\VnKey\ without one), which VnKey sends to the web
// stub on the Mac (docs/TESTING.md). Each line is written through at once, so the log
// keeps what came before a crash. VnKey and its plugin in every application write to the
// same file: each line goes at the end as the file is then, after a prefix that names
// the application. The line buffers are members, not on the stack: the plugin's log is
// written in applications that may have an 8 KB stack.
class CVnLog : public CBase
    {
public:
    static CVnLog* NewL();
    ~CVnLog();
    // Does not leave: a line that cannot be written is lost.
    void Write(const TDesC& aLine);
    // Put before every line, e.g. "mce: " (at most 24 characters).
    void SetPrefix(const TDesC& aPrefix);
    // The log as it is, its last aMax bytes at most, for Options > Send log.
    HBufC8* ContentL(TInt aMax);
    // Empties the log (Options > Clear log).
    void Clear();
    // The folder of the log, with a trailing backslash; other files of VnKey go there too.
    const TDesC& Folder() const { return iFolder; }
    RFs& Fs() { return iFs; }

private:
    enum { KMaxLine = 400, KMaxPrefix = 24 };   // characters; longer lines are cut
    void ConstructL();

private:
    RFs iFs;
    RFile iFile;
    TBool iOpen;
    TFileName iFolder;
    TBuf<KMaxPrefix> iPrefix;
    TBuf<KMaxLine + KMaxPrefix + 16> iLine;
    TBuf8<(KMaxLine + KMaxPrefix + 16) * 3 + 2> iUtf8;
    };

#endif // VNLOG_H
