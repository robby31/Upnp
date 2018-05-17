import QtQuick 2.9
import QtQuick.Controls 2.2

ListView {
    id: listview
    ScrollBar.vertical: ScrollBar { }
    clip: true

    delegate: MessageDelegate { }

    function selectMessage(index, message) {
        setMessageDetails(message)
    }
}

