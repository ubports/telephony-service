#include "telephonyappapprover.h"
#include <QApplication>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AbstractClient>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    Tp::registerTypes();
    Tp::ClientRegistrarPtr registrar = Tp::ClientRegistrar::create();
    Tp::AbstractClientPtr approver = Tp::AbstractClientPtr::dynamicCast(
          Tp::SharedPtr<TelephonyAppApprover>(new TelephonyAppApprover()));
    registrar->registerClient(approver, "TelephonyAppApprover");
    return app.exec();
}
