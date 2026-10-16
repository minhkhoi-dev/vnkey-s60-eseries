#include <es_enum.h>
#include "VnLogSender.h"

_LIT8(KHeadFmt, "POST /log HTTP/1.0\r\nHost: %S\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %d\r\n\r\n");
_LIT8(KStatusOk, "200");
_LIT8(KHeadEnd, "\r\n\r\n");

const TInt KTimeout = 20000000; // microseconds by the phone's clock, for connect, send and answer

CVnLogSender* CVnLogSender::NewL(MVnLogSenderObserver& aObserver)
    {
    CVnLogSender* self = new (ELeave) CVnLogSender(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CVnLogSender::CVnLogSender(MVnLogSenderObserver& aObserver)
    : CActive(EPriorityStandard), iObserver(aObserver)
    {
    CActiveScheduler::Add(this);
    }

void CVnLogSender::ConstructL()
    {
    iTimer = CPeriodic::NewL(CActive::EPriorityStandard);
    }

CVnLogSender::~CVnLogSender()
    {
    Cancel();
    CloseAll();
    delete iTimer;
    }

void CVnLogSender::SendL(const TDesC& aServer, const TDesC8& aBody)
    {
    if (iState != EIdle)
        {
        User::Leave(KErrInUse);
        }
    const TInt colon = aServer.LocateReverse(':');
    if (colon <= 0 || aServer.Length() > 60)
        {
        User::Leave(KErrArgument);
        }
    TUint port = 0;
    TLex lex(aServer.Mid(colon + 1));
    if (lex.Val(port, EDecimal) != KErrNone || port == 0 || port > 65535)
        {
        User::Leave(KErrArgument);
        }
    TInetAddr addr;
    if (addr.Input(aServer.Left(colon)) != KErrNone)
        {
        User::Leave(KErrArgument);
        }
    addr.SetPort(port);
    iAddr = addr;

    TBuf8<64> host;
    host.Copy(aServer);
    TBuf8<200> head;
    head.Format(KHeadFmt, &host, aBody.Length());
    delete iRequest;
    iRequest = NULL;
    iRequest = HBufC8::NewL(head.Length() + aBody.Length());
    TPtr8 request(iRequest->Des());
    request.Copy(head);
    request.Append(aBody);

    TInt err = iSocketServ.Connect();
    if (err == KErrNone)
        {
        iServOpen = ETrue;
        err = iConnection.Open(iSocketServ);
        }
    if (err != KErrNone)
        {
        CloseAll();
        User::Leave(err);
        }
    iConnOpen = ETrue;
    // Join a connection that is up (the WLAN the browser uses), else ask.
    iConnPref = TCommDbConnPref();
    iConnPref.SetDirection(ECommDbConnectionDirectionOutgoing);
    TUint count = 0;
    TPckgBuf<TConnectionInfo> info;
    if (iConnection.EnumerateConnections(count) == KErrNone && count > 0
        && iConnection.GetConnectionInfo(1, info) == KErrNone)
        {
        iConnPref.SetIapId(info().iIapId);
        iConnPref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
        }
    else
        {
        iConnPref.SetDialogPreference(ECommDbDialogPrefPrompt);
        }
    iConnection.Start(iConnPref, iStatus);
    iState = EStarting;
    SetActive();
    }

void CVnLogSender::RunL()
    {
    const TInt status = iStatus.Int();
    if (iState == EReading && status == KErrEof)
        {
        Finish(KErrNone); // the stub closes the connection after its answer
        return;
        }
    if (status != KErrNone)
        {
        Finish(status);
        return;
        }
    switch (iState)
        {
        case EStarting:
            {
            const TInt err = iSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp, iConnection);
            if (err != KErrNone)
                {
                Finish(err);
                return;
                }
            iSocketOpen = ETrue;
            iSocket.Connect(iAddr, iStatus);
            iState = EConnecting;
            SetActive();
            iTimer->Start(KTimeout, KTimeout, TCallBack(TimedOut, this));
            break;
            }
        case EConnecting:
            iSocket.Write(*iRequest, iStatus);
            iState = EWriting;
            SetActive();
            break;
        case EWriting:
            iReply.Zero();
            iChunk.Zero();
            iSocket.RecvOneOrMore(iChunk, 0, iStatus, iChunkLength);
            iState = EReading;
            SetActive();
            break;
        case EReading:
            iReply.Append(iChunk.Left(iReply.MaxLength() - iReply.Length()));
            iChunk.Zero();
            iSocket.RecvOneOrMore(iChunk, 0, iStatus, iChunkLength);
            SetActive();
            break;
        default:
            break;
        }
    }

void CVnLogSender::DoCancel()
    {
    switch (iState)
        {
        case EStarting:
            // RConnection has no CancelStart in this API; closing completes it.
            iConnection.Close();
            iConnOpen = EFalse;
            break;
        case EConnecting:
            iSocket.CancelConnect();
            break;
        case EWriting:
            iSocket.CancelWrite();
            break;
        case EReading:
            iSocket.CancelRecv();
            break;
        default:
            break;
        }
    }

TInt CVnLogSender::RunError(TInt aError)
    {
    Finish(aError);
    return KErrNone;
    }

TInt CVnLogSender::TimedOut(TAny* aSelf)
    {
    CVnLogSender* self = static_cast<CVnLogSender*>(aSelf);
    self->Cancel();
    self->Finish(KErrTimedOut);
    return KErrNone;
    }

// "HTTP/1.0 200 OK ... saved vnkey-<date>-<time>.log <bytes>": the error, and the stub's
// last words for the note.
void CVnLogSender::Finish(TInt aError)
    {
    iTimer->Cancel();
    TInt err = aError;
    if (err == KErrNone && iReply.Mid(Min(9, iReply.Length())).Left(3) != KStatusOk)
        {
        err = KErrGeneral;
        }
    TPtrC8 said(iReply);
    const TInt end = iReply.Find(KHeadEnd);
    if (end >= 0)
        {
        said.Set(iReply.Mid(end + KHeadEnd().Length()));
        }
    TBuf<128> reply;
    reply.Copy(said.Left(reply.MaxLength()));
    reply.TrimAll();
    CloseAll();
    iState = EIdle;
    iObserver.LogSent(err, reply);
    }

void CVnLogSender::CloseAll()
    {
    if (iSocketOpen)
        {
        iSocket.Close();
        iSocketOpen = EFalse;
        }
    if (iConnOpen)
        {
        iConnection.Close();
        iConnOpen = EFalse;
        }
    if (iServOpen)
        {
        iSocketServ.Close();
        iServOpen = EFalse;
        }
    delete iRequest;
    iRequest = NULL;
    }
