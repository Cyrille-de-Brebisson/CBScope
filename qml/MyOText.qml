import QtQuick 2.0

Row { spacing: 10
    y: (parent.height-height)/2;
    property alias caption: textField.text
    property alias text: textRect.text
    Text {
        color: "white"
        id: textField
        y: (parent.height-height)/2
    }
    Rectangle {
        id: textRect
        color: "black"
        border.width: 3; border.color: "lightgrey"; radius: 4;
        height: textInput.height+4*border.width
        width: textInput.width+4*border.width
        smooth: true
        property alias text: textInput.text
        Text {
            color: "white"
            id: textInput
            clip: true
            x: parent.border.width*2; y: parent.border.width*2;
        }
    }
}
