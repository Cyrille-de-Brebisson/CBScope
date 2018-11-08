import QtQuick 2.0

Rectangle {
    id: textField
    border.width: 3; border.color: textInput.focus ? "#569ffd" : "lightgrey"; radius: 4;
    height: textInput.height+border.width*4
    width: textInput.width+border.width*4
    clip: true
    smooth: true
    property alias text: textInput.text
    TextInput {
        id: textInput
        clip: true
        x: parent.border.width*2; y: parent.border.width*2;
//        font.pointSize: r1.fontSize
        width: Math.max(20,contentWidth)
        color: "black"
        onTextChanged: textField.textChanged(text)
    }
}
