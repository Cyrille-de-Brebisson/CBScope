import QtQuick 2.0

Row {
    id: r1
    property alias caption: t1.text
    property alias text: textField.text
    property int fontSize: 17

    width: parent.width
    spacing: 20
    Text { anchors.verticalCenter: parent.verticalCenter; id: t1; font.pointSize: r1.fontSize; }
    Rectangle {
        id: textField
        width:  r1.width-t1.width-r1.spacing;
        height: textInput.height * 1.8
        clip: true
        border.width: 3
        color: "lightgrey"
        border.color: color
        radius: 4
        smooth: true
        property alias text: textInput.text
        Text {
            anchors.verticalCenter: parent.verticalCenter; id: textInput; font.pointSize: r1.fontSize;
            clip: true
            width: parent.width - (2 * font.pointSize)
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
