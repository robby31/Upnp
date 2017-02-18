import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import MyComponents 1.0

ColumnLayout {
    id: item

    property var service
    property int rootIndex
    property string rootIconUrl

    Row {
        anchors { left: parent.left; leftMargin: 10; right: parent.right; rightMargin: 10 }
        height: backButton.height + 20
        Layout.preferredHeight: height
        spacing: 10

        MyButton {
            id: backButton
            anchors { verticalCenter: parent.verticalCenter }
            sourceComponent: Text { id: text; text: "< Back" }
            onButtonClicked: setRootDeviceDetails(rootIndex, rootIconUrl)
        }

        Text {
            width: contentWidth
            anchors.verticalCenter: parent.verticalCenter
            text: service.serviceType
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
            text: qsTr("Actions")
        }
    }

    SwipeView {
        id: swipeView
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabBar.currentIndex

        Flickable {
            id: flickable

            TextArea.flickable: TextArea {
                text: item.service.description
                wrapMode: TextArea.Wrap
            }

            ScrollBar.vertical: ScrollBar { }
        }

        ListView {
            id: actionsView
            model: service.actionsModel

            delegate: ServiceActionsDelegate { }

            function runAction(index) {
                service.runAction(index)
            }
        }
    }
}
