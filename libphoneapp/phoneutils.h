#ifndef PHONEUTILS_H
#define PHONEUTILS_H

#include <QObject>

class PhoneUtils : public QObject
{
    Q_OBJECT
public:
    explicit PhoneUtils(QObject *parent = 0);
    Q_INVOKABLE static bool comparePhoneNumbers(const QString &number1, const QString &number2);
    
};

#endif // PHONEUTILS_H
