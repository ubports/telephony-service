import QtQuick 2.0
import Ubuntu.Components 1.3
import Ubuntu.OnlineAccounts 0.1

Item {
    id: root

    readonly property string keyPrefix: "telepathy/"
    readonly property var accountObjectHandle: account ? account.objectHandle : undefined

    property string manager
    property string protocol
    property string icon
    property var params
    property var advancedParams
    property bool hasCrendentials: true

    signal finished

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

    function extendedSettings(inputFields)
    {
        return {}
        //Helper class to be extended by derived class
    }

    function saveServiceSettings(serviceIM, creds) {
        var settingsIM = serviceIM.settings
        var inputFields = {}

        settingsIM[root.keyPrefix + 'manager'] = root.manager
        settingsIM[root.keyPrefix + 'protocol'] = root.protocol
        settingsIM[root.keyPrefix + 'Icon'] = root.icon

        for (var i=0; i < paramsRepeater.count; i++) {
            var fieldData = root.params[i]
            var field = paramsRepeater.itemAt(i)

            inputFields[fieldData.name] = field.value

            if (fieldData.store) {
                settingsIM[root.keyPrefix + 'param-' + fieldData.name] = field.value
            }
        }

        var xSettings = extendedSettings(inputFields)
        for (var key in xSettings) {
            settingsIM[root.keyPrefix + key] = xSettings[key]
        }

        serviceIM.updateSettings(settingsIM)
        //serviceIM.credentials = creds
        //serviceIM.updateServiceEnabled(true)
    }

    function continueAccountSave(creds)  {
        var imService = root.getAccountService()
        if (!imService) {
            console.warn("Fail to retrieve account service")
            return
        }

        root.saveServiceSettings(imService, creds)
        if (creds)
            globalAccountService.credentials = creds
        globalAccountService.updateServiceEnabled(true)

        account.synced.connect(root.finished)
        account.sync()
    }

    function credentialsStored() {
        if (creds.credentialsId === 0) {
            console.warn("Credentials not stored correct")
            return
        }

        var imService = root.getAccountService()
        if (!imService) {
            console.warn("Fail to retrieve account service")
            return
        }

        continueAccountSave(creds)
    }

    function parseCrendentials() {
        var credentials = {'userName': '', 'password': ''}

        for (var i=0; i < paramsRepeater.count; i++) {
            var fieldData = root.params[i]
            var field = paramsRepeater.itemAt(i)

            if (fieldData.name === 'account')
                credentials['userName'] = field.value

            if (fieldData.name === 'password')
                credentials['password'] = field.value
        }

        return credentials
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

        Icon {
            anchors.horizontalCenter: fields.horizontalCenter
            name: root.icon
        }

        Repeater {
            id: paramsRepeater

            width: parent.width
            model: root.params
            DynamicField {
                model: modelData
                anchors.horizontalCenter: fields.horizontalCenter
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: units.gu(2)

            Button {
                id: btnContinue

                text: i18n.tr("Continue")
                onClicked: {
                    var info = root.parseCrendentials()
                    // save account
                    account.updateDisplayName(info.userName)
                    if (root.hasCrendentials) {
                        creds.userName = info.userName
                        creds.secret = info.password
                        creds.sync()
                    } else {
                        continueAccountSave(null)
                    }
                }
            }

            Button {
                id: btnCancel
                text: i18n.tr("Cancel")
                onClicked: {
                    account.removed.connect(root.finished)
                    account.remove(Account.RemoveCredentials)
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
