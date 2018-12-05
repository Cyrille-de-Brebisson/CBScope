import QtQuick 2.0

Row { id: root
    spacing: caption==="" ? 0 : 10
    property alias text: textField.text
    property alias caption: captionField.text
    signal onTextChanged(var text)
    property alias fontcol: textInput.color
    y: (parent.height-height)/2;
    Text {
        id: captionField
        y: (parent.height-height)/2
    }
    Rectangle {
        id: textField
        border.width: 3; border.color: textInput.focus ? "#569ffd" : "lightgrey"; radius: 4;
        height: textInput.height+border.width*4
        width: textInput.width+border.width*4
        clip: true
        smooth: true
        property alias text: textInput.text
        property alias fontcol: textInput.color
        TextInput {
            id: textInput
            clip: true
            x: parent.border.width*2; y: parent.border.width*2;
            width: Math.max(20,contentWidth)
            color: "black"
            onTextChanged: root.textChanged(text)
        }
    }
}
