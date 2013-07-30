#ifndef CONTACTUTILS_H
#define CONTACTUTILS_H

#include <QContactManager>

QTCONTACTS_USE_NAMESPACE

namespace ContactUtils
{
    QContactManager *sharedManager();
}

#endif // CONTACTUTILS_H
