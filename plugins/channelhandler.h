class ChannelHandler : public Tp::AbstractClientHandler
{
public:
    ChannelHandler(const Tp::ChannelClassSpecList &channelFilter);
    ~ChannelHandler() { }
    void bypassApproval() const;
    void handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                        const Tp::AccountPtr &account,
                        const Tp::ConnectionPtr &connection,
                        const QList<Tp::ChannelPtr> &channels,
                        const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                        const QDateTime &userActionTime,
                        const Tp::AbstractClientHandler::HandlerInfo &handlerInfo);
};

