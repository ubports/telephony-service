#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>

class ProtocolManager : public QObject
{
    Q_OBJECT
public:
    enum Feature {
        TextChats = 0x1,
        VoiceCalls = 0x2
    };
    Q_DECLARE_FLAGS(Features, Feature)


    struct Protocol {
        QString name;
        Features features;
    };

    typedef QList<Protocol> Protocols;

    static ProtocolManager *instance();

    Protocols protocols() const;
    Protocols textProtocols() const;
    Protocols voiceProtocols() const;

protected Q_SLOTS:
    void loadSupportedProtocols();

private:
    explicit ProtocolManager(QObject *parent = 0);

};

#endif // PROTOCOLMANAGER_H
