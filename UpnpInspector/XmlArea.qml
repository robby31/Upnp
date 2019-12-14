import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import MyComponents 1.0

Item {
    id: item

    property string text

    state: "TREE"

    ColumnLayout {
        anchors.fill: parent

        Row {
            Layout.fillWidth: true

            spacing: 10

            RadioButton {
                text: "Text"
                onCheckedChanged: {
                    if (checked)
                        item.state = "TEXT"
                }
            }

            RadioButton {
                text: "Tree"
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

    states: [
        State {
            name: "TEXT"
            PropertyChanges { target: loader; sourceComponent: textArea }
        },

        State {
            name: "TREE"
            PropertyChanges { target: loader; sourceComponent: null; source: "TreeArea.qml" }
        }
    ]
}
