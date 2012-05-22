ChannelHandler::ChannelHandler(const Tp::ChannelClassSpecList &channelFilter)
    : Tp::AbstractClientHandler(channelFilter)
{
}

void ChannelHandler::bypassApproval() const
{
    return false;
}

void ChannelHandler::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                               const Tp::AccountPtr &account,
                               const Tp::ConnectionPtr &connection,
                               const QList<Tp::ChannelPtr> &channels,
                               const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                               const QDateTime &userActionTime,
                               const Tp::AbstractClientHandler::HandlerInfo &handlerInfo)
{
    foreach(const Tp::ChannelPtr &channel, channels) {
        QVariantMap properties = channel->immutableProperties();
        if (properties[TP_QT_IFACE_CHANNEL ".ChannelType"] ==
                TP_QT_IFACE_CHANNEL_TYPE_CALL) {
            if (channel->isRequested()) {
            } else {
            }
        }
    }
    context->setFinished();
}

