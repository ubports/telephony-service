import QtQuick 2.0
import Ubuntu.Components 1.3
import Ubuntu.OnlineAccounts 0.1

Item {
    id: root

    readonly property string keyPrefix: "telepathy/"
    readonly property var accountObjectHandle: account ? account.objectHandle : undefined
    readonly property alias isValid: paramsRepeater.fieldHasValues

    property string manager
    property string protocol
    property string icon
    property var params
    property var advancedParams
    property bool hasCrendentials: true


    signal finished
    height: fields.childrenRect.height +
            units.gu(10)

    function getAccountService() {
        var service = serviceModel.get(0, "accountServiceHandle")
        if (!service) {
            console.warn("No service handle from model")
            return null
        }

        return accountServiceComponent.createObject(null,
                                                    {"objectHandle": service})
    }

    // virual
    function extendedSettings(inputFields)
    {
        return {}
        //Helper class to be extended by derived class
    }

    // virtual
    function formatDisplayName(inputFields)
    {
        return inputFields['account']
        // Helper function that allow the derived class to format a different display name
    }

    function saveServiceSettings(serviceIM, creds) {
        var settingsIM = serviceIM.settings
        var inputFields = {}

        settingsIM[root.keyPrefix + 'manager'] = root.manager
        settingsIM[root.keyPrefix + 'protocol'] = root.protocol
        settingsIM[root.keyPrefix + 'Icon'] = root.icon

        // basic fields
        for (var i=0; i < paramsRepeater.count; i++) {
            var fieldData = root.params[i]
            var field = paramsRepeater.itemAt(i)
            var fieldParamName = root.keyPrefix + 'param-' + fieldData.name

            if (field.isEmpty) {
                delete settingsIM[fieldParamName]
            } else {
                inputFields[fieldData.name] = field.value
                if (fieldData.store) {
                    settingsIM[fieldParamName] = field.value
                }
            }
        }

        // advanced fields
        for (var i=0; i < advancedParamsRepeater.count; i++) {
            var xFieldData = root.advancedParams[i]
            var xField = advancedParamsRepeater.itemAt(i)
            var xFieldParamName = root.keyPrefix + 'param-' + xFieldData.name

            if (xField.isEmpty) {
                delete settingsIM[xFieldParamName]
            } else {
                inputFields[xFieldData.name] = xField.value

                if (xFieldData.store) {
                    settingsIM[xFieldParamName] = xField.value
                }
            }
        }


        var xSettings = extendedSettings(inputFields)
        for (var key in xSettings) {
            settingsIM[root.keyPrefix + key] = xSettings[key]
        }

        account.updateDisplayName(formatDisplayName(inputFields))

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

    function cancel() {
        account.removed.connect(root.finished)
        account.remove(Account.RemoveCredentials)
    }

    function confirm() {
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

    Column {
        id: fields

        anchors {
            top: parent.top
            topMargin: units.gu(5)
            left: parent.left
            right: parent.right
        }
        height: childrenRect.height
        spacing: units.gu(2)

        Icon {
            anchors.horizontalCenter: fields.horizontalCenter
            name: root.icon
        }

        Repeater {
            id: paramsRepeater

            property bool fieldHasValues: false

            function checkFieldsHasValues()
            {
                var hasEmptyField = false
                for (var i = 0; i < paramsRepeater.count; i++) {

                    var child = paramsRepeater.itemAt(i)
                    if (child && child.isEmpty) {
                        hasEmptyField = true
                        break
                    }
                }
                fieldHasValues = !hasEmptyField
            }

            width: parent.width
            model: root.params
            DynamicField {
                model: modelData
                anchors{
                    left: parent.left
                    right: parent.right
                    margins: units.gu(4)
                }
                onValueChanged: paramsRepeater.checkFieldsHasValues()
            }
        }

        Item {
            id: div

            anchors{
                left: parent.left
                right: parent.right
            }
            height: units.gu(3)
            visible: root.advancedParams.length > 0
        }

        Label {
            id: advancedParamsTitle

            anchors{
                left: parent.left
                right: parent.right
                margins: units.gu(4)
            }
            visible: root.advancedParams.length > 0
            text: i18n.tr("Advanced Options")
            textSize: Label.Medium
        }

        Repeater {
            id: advancedParamsRepeater

            width: parent.width
            model: root.advancedParams
            DynamicField {
                model: modelData
                anchors{
                    left: parent.left
                    right: parent.right
                    margins: units.gu(4)
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
