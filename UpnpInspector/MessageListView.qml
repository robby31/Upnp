import QtQuick 2.0

ListView {
    id: listview

    clip: true

    delegate: MessageDelegate { }

    function selectMessage(index, message) {
        setMessageDetails(message)
    }
}
