// VnKeyBoot.cpp - the launcher that the phone starts at boot (roadmap 4). The startup list
// entry (data/vnkey_startup.rss) names this executable, because such an entry can name
// nothing else and takes no arguments. Unless VnKey's settings.txt says boot=0 (Start at
// boot: off), it starts VnKey through the application server with the background command,
// so that VnKey comes up behind the idle screen and turns Telex in all apps back on, and
// exits. Made after OttPilot's launcher, which the E72 runs at boot (its docs/DEVICE.md).
#include <e32base.h>
#include <e32std.h>
#include <f32file.h>
#include <apgcli.h>
#include <apacmdln.h>
#include <apgtask.h>
#include <w32std.h>

const TUid KVnKeyUid = { 0xE7A10001 };

// The application list may not be complete this early after a boot, so the start is
// tried again.
const TInt KStartTries = 10;
const TInt KStartRetryMicroSeconds = 3000000;

// VnKey's folder, as VnFiles::Folder finds it (VnShared.cpp): the memory card when there
// is one.
_LIT(KCardFolder, "E:\\VnKey\\");
_LIT(KPhoneFolder, "C:\\Data\\VnKey\\");
_LIT(KSettingsFile, "settings.txt");
_LIT(KSettingsTemp, "settings.tmp");    // what VnKey writes before it becomes settings.txt
_LIT8(KBootOff, "boot=0");

// EFalse when settings.txt has the line boot=0. No file, or no such line: start.
static TBool BootStartWanted()
    {
    RFs fs;
    if (fs.Connect() != KErrNone)
        {
        return ETrue;
        }
    TFileName name;
    TDriveInfo drive;
    if (fs.Drive(drive, EDriveE) == KErrNone && drive.iType != EMediaNotPresent
        && fs.MkDirAll(KCardFolder) != KErrNotReady)
        {
        name.Copy(KCardFolder);
        }
    else
        {
        name.Copy(KPhoneFolder);
        }
    const TInt folder = name.Length();
    name.Append(KSettingsFile);
    TBool wanted = ETrue;
    RFile file;
    TInt err = file.Open(fs, name, EFileRead | EFileShareReadersOnly);
    if (err != KErrNone)
        {
        name.SetLength(folder);
        name.Append(KSettingsTemp); // VnKey's save was cut short by a restart
        err = file.Open(fs, name, EFileRead | EFileShareReadersOnly);
        }
    if (err == KErrNone)
        {
        TBuf8<1024> raw;
        if (file.Read(raw) == KErrNone)
            {
            TPtrC8 rest(raw);
            while (rest.Length() > 0)
                {
                const TInt end = rest.Locate('\n');
                TPtrC8 line(end < 0 ? rest : rest.Left(end));
                rest.Set(end < 0 ? TPtrC8() : rest.Mid(end + 1));
                while (line.Length() > 0
                       && (line[line.Length() - 1] == '\r' || line[line.Length() - 1] == ' '))
                    {
                    line.Set(line.Left(line.Length() - 1));
                    }
                if (line == KBootOff)
                    {
                    wanted = EFalse;
                    }
                }
            }
        file.Close();
        }
    fs.Close();
    return wanted;
    }

// ETrue when VnKey runs already: started by hand before the startup list got here.
static TBool AppRunningL()
    {
    RWsSession ws;
    User::LeaveIfError(ws.Connect());
    CleanupClosePushL(ws);
    TApaTaskList tasks(ws);
    const TBool running = tasks.FindApp(KVnKeyUid).Exists();
    CleanupStack::PopAndDestroy(&ws);
    return running;
    }

static void StartAppL()
    {
    if (AppRunningL())
        {
        return;
        }
    RApaLsSession ls;
    User::LeaveIfError(ls.Connect());
    CleanupClosePushL(ls);
    TApaAppInfo info;
    const TInt err = ls.GetAppInfo(info, KVnKeyUid);
    if (err != KErrNone)
        {
        User::Leave(err < 0 ? err : KErrNotReady); // not in the list yet, or the list is not ready
        }
    CApaCommandLine* cmd = CApaCommandLine::NewLC();
    cmd->SetExecutableNameL(info.iFullName);
    cmd->SetCommandL(EApaCommandBackground);
    User::LeaveIfError(ls.StartApp(*cmd));
    CleanupStack::PopAndDestroy(2, &ls); // cmd, ls
    }

GLDEF_C TInt E32Main()
    {
    CTrapCleanup* cleanup = CTrapCleanup::New();
    if (!cleanup)
        {
        return KErrNoMemory;
        }
    TInt err = KErrNone;
    if (BootStartWanted())
        {
        for (TInt i = 0; i < KStartTries; ++i)
            {
            TRAP(err, StartAppL());
            if (err == KErrNone)
                {
                break;
                }
            User::After(KStartRetryMicroSeconds);
            }
        }
    delete cleanup;
    return err;
    }
