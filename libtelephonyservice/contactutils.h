#ifndef CONTACTUTILS_H
#define CONTACTUTILS_H

#include <QContactManager>
#include <QContact>

QTCONTACTS_USE_NAMESPACE

namespace ContactUtils
{
    QContactManager *sharedManager();
    QString formatContactName(const QContact &contact);
}

#endif // CONTACTUTILS_H
