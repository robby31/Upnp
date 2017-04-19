import QtQuick 2.0
import QtQuick.Controls 2.0

ListView {
    id: listview
    ScrollBar.vertical: ScrollBar { }
    clip: true

    delegate: MessageDelegate { }

    function selectMessage(index, message) {
        setMessageDetails(message)
    }
}

