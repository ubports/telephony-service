#include "contactutils.h"

QTCONTACTS_USE_NAMESPACE

namespace ContactUtils
{

QContactManager *sharedManager()
{
    static QContactManager *instance = new QContactManager("galera");
    return instance;
}

}
