import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import MyComponents 1.0

Item {
    id: item

    property var service
    property string rootIconUrl

    property string sourceProtocol: ""
    property string sinkProtocol: ""

    ColumnLayout {
        anchors.fill: parent
        spacing: 20

        Row {
            Layout.fillWidth: true
            Layout.margins: 10
            height: backButton.height + 20
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
                source: rootIconUrl
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
                text: qsTr("SourceProtocolInfo")
            }
            TabButton {
                text: qsTr("SinkProtocolInfo")
            }
            TabButton {
                text: qsTr("CurrentConnectionIDs")
            }
        }

        SwipeView {
            id: swipeView
            currentIndex: tabBar.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
                clip: true
                text: sourceProtocol
            }

            ListView {
                clip: true
                model: sinkProtocolModel
                delegate: Text { text: protocol }
                ScrollBar.vertical: ScrollBar { }
            }

            Column {
                ComboBox {
                    model: connectionId
                    textRole: "value"
                    onCurrentIndexChanged: {
                        var idSelected = model.get(currentIndex).value
                        if (idSelected)
                            service.runAction("GetCurrentConnectionInfo", { ConnectionID: idSelected })
                        else
                            console.log("invalid id")
                    }
                }

                ListView {
                    clip: true
                    width: parent.width
                    height: 300
                    model: connectionInfoModel
                    delegate: Text { text: param + " : " + value }
                    ScrollBar.vertical: ScrollBar { }
                }
            }
        }
    }

    ListModel {
        id: connectionId

        function updateIds(value) {
            clear()
            var ids = value.split(",")
            for (var i=0;i<ids.length;++i) {
                append({value: ids[i]})
            }
        }

        Component.onCompleted: {
            // by default: one connection id equal to zero
            append({value: "0"})
        }
    }

    ListModel {
        id: connectionInfoModel
    }

    ListModel {
        id: sinkProtocolModel

        function updateSinkProtocol(value) {
            clear()
            var protocols = value.split(",")
            for (var i=0;i<protocols.length;++i) {
                append({protocol: protocols[i]})
            }
        }
    }

    onSinkProtocolChanged: sinkProtocolModel.updateSinkProtocol(sinkProtocol)

    Connections {
        target: service.stateVariablesModel

        onDataChanged: {
            var index = topLeft.row
            var name = target.get(index, "name")
            var value = target.get(index, "value")

            if (name === "SourceProtocolInfo") {
                sourceProtocol = value
            } else if (name === "SinkProtocolInfo") {
                sinkProtocol = value
            } else if (name === "CurrentConnectionIDs") {
                connectionId.updateIds(value)
            } else {
                console.log("unknown data name", name)
            }

        }

    }

    Connections {
        target: service

        onActionAnswer: {
            connectionInfoModel.clear()
            for (var param in data)
                connectionInfoModel.append({ param: param, value: data[param] })
        }
    }

    Component.onCompleted: {
        service.subscribeEventing()
    }
}
