import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import MyComponents 1.0

Item {
    id: item

    property string text

    state: "TREE"

    ExclusiveGroup { id: stateSelection }

    ColumnLayout {
        anchors.fill: parent

        Row {
            Layout.fillWidth: true

            spacing: 10

            RadioButton {
                text: "Text"
                exclusiveGroup: stateSelection
                onCheckedChanged: {
                    if (checked)
                        item.state = "TEXT"
                }
            }

            RadioButton {
                text: "Tree"
                exclusiveGroup: stateSelection
                checked: true
                onCheckedChanged: {
                    if (checked)
                        item.state = "TREE"
                }
            }
        }

        Loader {
            id: loader
            Layout.fillHeight: true
            Layout.fillWidth: true
            sourceComponent: textArea
        }
    }

    Component {
        id: textArea

        TextArea {
            id: area
            text: docModel.xmlString
        }
    }

    MarkupDocModel {
        id: docModel
        xmlString: item.text
    }

    Component {
        id: treeArea

        TreeView {
            id: view

            TableViewColumn {
                title: "Tag"
                role: "display"
                width: 300
            }

            model: docModel

            itemDelegate: Item {
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: styleData.textColor
                    elide: styleData.elideMode
                    text: styleData.value
                    textFormat: Text.PlainText
                }
            }
        }
    }

    states: [
        State {
            name: "TEXT"
            PropertyChanges { target: loader; sourceComponent: textArea }
        },

        State {
            name: "TREE"
            PropertyChanges { target: loader; sourceComponent: treeArea }
        }
    ]
}
