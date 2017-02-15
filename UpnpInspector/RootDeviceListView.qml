import QtQuick 2.0

ListView {
    id: listview

    clip: true

    delegate: RootDeviceDelegate { }

    function selectDetails(index, iconurl) {
        setRootDeviceDetails(index, iconurl)
    }
}
