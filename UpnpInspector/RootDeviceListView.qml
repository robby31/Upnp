import QtQuick 2.0
import QtQuick.Controls 2.0

ListView {
    id: listview

    ScrollBar.vertical: ScrollBar { }

    clip: true

    delegate: RootDeviceDelegate { }

    function selectDetails(index, iconurl) {
        setRootDeviceDetails(index, iconurl)
    }
}
