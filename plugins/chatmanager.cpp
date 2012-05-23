#include "chatmanager.h"
#include "telepathyhelper.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

ChatManager::ChatManager(QObject *parent)
: QObject(parent)
{
}

bool ChatManager::isChattingToContact(const QString &contactId)
{
    return mChannels.contains(contactId);
}

void ChatManager::startChat(const QString &contactId)
{
    if (!mChannels.contains(contactId)) {
        // Request the contact to start chatting to
        Tp::AccountPtr account = TelepathyHelper::instance()->account();
        connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << contactId),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onContactsAvailable(Tp::PendingOperation*)));
    }
}

void ChatManager::endChat(const QString &contactId)
{
    if (!mChannels.contains(contactId))
        return;

    mChannels[contactId]->requestClose();
    mChannels.remove(contactId);
    mContacts.remove(contactId);
}

void ChatManager::sendMessage(const QString &contactId, const QString &message)
{
    if (!mChannels.contains(contactId))
        return;

    mChannels[contactId]->send(message);
}

void ChatManager::onTextChannelAvailable(Tp::TextChannelPtr channel)
{
    mChannels[channel->targetContact()->id()] = channel;

    emit chatReady(channel->targetContact()->id());
}

void ChatManager::onContactsAvailable(Tp::PendingOperation *op)
{
    Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);
    Q_ASSERT(pc);

    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    // start chatting to the contacts
    foreach(Tp::ContactPtr contact, pc->contacts()) {
        account->ensureTextChat(contact);

        // hold the ContactPtr to make sure its refcounting stays bigger than 0
        mContacts[contact->id()] = contact;
    }
}
