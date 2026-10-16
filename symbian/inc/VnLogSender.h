#ifndef VNLOGSENDER_H
#define VNLOGSENDER_H

#include <e32base.h>
#include <es_sock.h>
#include <in_sock.h>
#include <commdbconnpref.h>

class MVnLogSenderObserver
    {
public:
    // aError is KErrNone when the stub answered 200; aReply is what it said. The sender
    // must not be deleted from here.
    virtual void LogSent(TInt aError, const TDesC& aReply) = 0;
    };

// Sends the test log to the web stub on the developer's Mac (symbian/group/webstub.py), as
// one HTTP/1.0 POST to http://<address:port>/log over the Wi-Fi: the E72's files cannot be
// copied off it by cable or Bluetooth (docs/TESTING.md). It joins a network connection
// that is up already and asks for the access point otherwise, as OttPilot does: on the
// E72 a plain RConnection::Start fails without asking when nothing is connected.
class CVnLogSender : public CActive
    {
public:
    static CVnLogSender* NewL(MVnLogSenderObserver& aObserver);
    ~CVnLogSender();
    // aServer is "a.b.c.d:port"; aBody is copied. Leaves with KErrInUse while sending and
    // KErrArgument for an address it cannot read.
    void SendL(const TDesC& aServer, const TDesC8& aBody);
    TBool IsSending() const { return iState != EIdle; }

private:
    enum TState { EIdle, EStarting, EConnecting, EWriting, EReading };

    CVnLogSender(MVnLogSenderObserver& aObserver);
    void ConstructL();
    void RunL();
    void DoCancel();
    TInt RunError(TInt aError);
    void Finish(TInt aError);
    void CloseAll();
    static TInt TimedOut(TAny* aSelf);

private:
    MVnLogSenderObserver& iObserver;
    TState iState;
    RSocketServ iSocketServ;
    RConnection iConnection;
    RSocket iSocket;
    TBool iServOpen;
    TBool iConnOpen;
    TBool iSocketOpen;
    TCommDbConnPref iConnPref;      // a member: it must outlive the request
    TInetAddr iAddr;
    HBufC8* iRequest;
    TBuf8<256> iReply;              // the stub's answer, as much as fits
    TBuf8<128> iChunk;
    TSockXfrLength iChunkLength;
    CPeriodic* iTimer;              // from the connect on; the access point query has none
    };

#endif // VNLOGSENDER_H
