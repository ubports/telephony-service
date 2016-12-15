import QtQuick 2.0
import QtQuick.Layouts 1.1

import Ubuntu.Components 0.1
import Ubuntu.OnlineAccounts 0.1

Item {
    id: root

    signal finished
    readonly property string keyPrefix: "telepathy/"
    readonly property var accountObjectHandle: account.objectHandle

    anchors {
        left: parent.left
        right: parent.right
        verticalCenter: parent.verticalCenter
    }

    height: fields.childrenRect.height +
            units.gu(10) +
            (Qt.inputMethod.visible ? Qt.inputMethod.keyboardRectangle.height : 0)

    function getAccountService() {
        var service = serviceModel.get(0, "accountServiceHandle")
        if (!service) {
            console.warn("No service handle from model")
            return null
        }

        return accountServiceComponent.createObject(null,
                                                    {"objectHandle": service})
    }

    function saveServiceSettings(serviceIM) {
        var settingsIM = serviceIM.settings

        settingsIM[root.keyPrefix + 'manager'] = 'sofiasip'
        settingsIM[root.keyPrefix + 'protocol'] = 'sip'
        settingsIM[root.keyPrefix + 'param-account'] = sipId.text
        settingsIM[root.keyPrefix + 'Icon'] = 'sip'

        serviceIM.updateSettings(settingsIM)
        serviceIM.credentials = creds
        serviceIM.updateServiceEnabled(true)
    }

    function credentialsStored() {
        if (creds.credentialsId == 0) {
            console.warn("Credentials not stored correct")
            return
        }

        var imService = root.getAccountService()
        if (!imService) {
            console.warn("Fail to retrieve account service")
            return
        }

        root.saveServiceSettings(imService)

        globalAccountService.updateServiceEnabled(true)
        globalAccountService.credentials = creds

        account.synced.connect(root.finished)
        account.sync()
    }

    Column {
        id: fields

        anchors {
            top: parent.top
            topMargin: units.gu(5)
            left: parent.left
            right: parent.right
        }
        spacing: units.gu(2)

        Label {
            anchors {
                left: parent.left
                right: parent.right
            }

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: i18n.tr("SIP Id:")
        }
        TextField {
            id: sipId

             anchors.horizontalCenter: parent.horizontalCenter
             width: units.gu(30)
        }

        Label {
            anchors {
                left: parent.left
                right: parent.right
            }

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: i18n.tr("Password")
        }
        TextField {
            id: password

            anchors.horizontalCenter: parent.horizontalCenter
            width: units.gu(30)
            echoMode: TextInput.Password
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: units.gu(2)

            Button {
                id: btnContinue

                text: i18n.tr("Continue")
                enabled: sipId.text != "" && password.text != ""
                onClicked: {
                    // save account
                    account.updateDisplayName(sipId.text)
                    creds.userName = sipId.text
                    creds.secret = password.text
                    creds.sync()
                }
            }

            Button {
                id: btnCancel
                text: i18n.tr("Cancel")
                onClicked: {
                    account.removed.connect(root.finished)
                    account.remove(Account.RemoveCredentials);
                }
            }
        }
    }

    AccountService {
        id: globalAccountService

        objectHandle: account.accountServiceHandle
        autoSync: false
    }

    Credentials {
        id: creds

        caption: account.provider.id
        acl: "*" // untill later
        storeSecret: true
        onCredentialsIdChanged: {
            console.debug("Credetials id changed")
            root.credentialsStored()
        }
    }

    // necessary to store settings on the "IM" service
    AccountServiceModel {
        id: serviceModel

        includeDisabled: true
        account: root.accountObjectHandle
        serviceType: "IM"
    }

    Component {
        id: accountServiceComponent

        AccountService {
            autoSync: false
        }
    }
}
