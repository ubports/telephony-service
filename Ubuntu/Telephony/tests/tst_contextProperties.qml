import QtQuick 2.0
import QtTest 1.0
import Ubuntu.Telephony 0.1

TestCase {
    id: contextPropertiesTest
    name: "ContextPropertiesTest"

    function test_applicationUtils() {
        verify(applicationUtils != undefined, "applicationUtils is not defined");
    }

    function test_telepathyHelper() {
        verify(telepathyHelper != undefined, "telepathyHelper is not defined");
    }

    function test_chatManager() {
        verify(chatManager != undefined, "chatManager is not defined");
    }

    function test_callManager() {
        verify(callManager != undefined, "callManager is not defined");
    }
}
