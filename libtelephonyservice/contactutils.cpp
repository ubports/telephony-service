#include "contactutils.h"
#include <QContactName>

QTCONTACTS_USE_NAMESPACE

namespace ContactUtils
{

QContactManager *sharedManager()
{
    static QContactManager *instance = new QContactManager("galera");
    return instance;
}

QString formatContactName(const QContact &contact)
{
    QContactName name = contact.detail<QContactName>();

    QString formattedName = name.firstName();

    // now check if we need an extra space to separate the first and last names
    if (!formattedName.isEmpty() && !name.lastName().isEmpty()) {
        formattedName.append(" ");
    }

    formattedName.append(name.lastName());

    return formattedName;
}

}
