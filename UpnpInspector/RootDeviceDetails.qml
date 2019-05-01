import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import MyComponents 1.0

ColumnLayout {
    id: item

    property var device
    property string iconurl

    Row {
        width: parent.width - 20
        Layout.preferredWidth: width
        Layout.alignment: Qt.AlignHCenter

        height: backButton.height + 20
        Layout.preferredHeight: height
        spacing: 10

        MyButton {
            id: backButton
            anchors { verticalCenter: parent.verticalCenter }
            sourceComponent: Text { id: text; text: "< Back" }
            onButtonClicked: goBack()
        }

        Image {
            anchors { verticalCenter: parent.verticalCenter }
            width: parent.height*0.9
            height: width
            sourceSize.width: width
            sourceSize.height: width
            fillMode: Image.PreserveAspectFit
            source: iconurl
        }

        Text {
            width: contentWidth
            anchors.verticalCenter: parent.verticalCenter
            text: device.deviceType
            color: device.available ? "black" : "red"
            clip: true
        }
    }

    TabBar {
        id: tabBar
        Layout.fillWidth: true
        currentIndex: swipeView.currentIndex
        TabButton {
            text: qsTr("Description")
        }
        TabButton {
            text: qsTr("Devices")
        }
        TabButton {
            text: qsTr("Services")
        }
    }

    SwipeView {
        id: swipeView
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabBar.currentIndex

        XmlArea {
            text: item.device.description
        }

        ListView {
            id: devicesView

            ScrollBar.vertical: ScrollBar { }

            model: item.device.devicesModel

            delegate: DeviceDelegate { }

            clip: true

            function selectDevice(index) {
                setDeviceDetails(item.device.devicesModel.at(index), item.iconurl)
            }
        }

        ListView {
            id: servicesView

            ScrollBar.vertical: ScrollBar { }

            model: item.device.servicesModel

            delegate: ServiceDelegate { }

            clip: true

            function selectService(index) {
                setServiceDetails(item.device.servicesModel.at(index), item.iconurl)
            }
        }
    }
}
