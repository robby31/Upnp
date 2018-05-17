import QtQuick 2.9
import QtQuick.Controls 2.2

ListView {
    id: listview

    ScrollBar.vertical: ScrollBar { }

    clip: true

    delegate: RootDeviceDelegate { }

    function selectDetails(index, iconurl) {
        setRootDeviceDetails(index, iconurl)
    }
}
