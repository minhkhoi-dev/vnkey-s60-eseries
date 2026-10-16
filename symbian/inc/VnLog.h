#ifndef VNLOG_H
#define VNLOG_H

#include <e32base.h>
#include <f32file.h>

// The test log: one line per call, with the time, in UTF-8, appended to vnkey.log in
// E:\VnKey\ on the memory card (C:\Data\VnKey\ without one), which the developer copies
// off the phone over USB (docs/TESTING.md). Each line is written through at once, so the
// log keeps what came before a crash.
class CVnLog : public CBase
    {
public:
    static CVnLog* NewL();
    ~CVnLog();
    // Does not leave: a line that cannot be written is lost.
    void Write(const TDesC& aLine);
    // The folder of the log, with a trailing backslash; other files of VnKey go there too.
    const TDesC& Folder() const { return iFolder; }
    RFs& Fs() { return iFs; }

private:
    void ConstructL();

private:
    RFs iFs;
    RFile iFile;
    TBool iOpen;
    TFileName iFolder;
    };

#endif // VNLOG_H
