import QtQuick 2.0

Rectangle {
    id: textField
    border.width: 3; border.color: textInput.focus ? "#569ffd" : "lightgrey"; radius: 4;
    height: textInput.height+border.width*4
    width: textInput.width+border.width*4
    color: "black"
    clip: true
    smooth: true
    property alias text: textInput.text
    TextEdit {
        id: textInput
        clip: true
        x: parent.border.width*2; y: parent.border.width*2;
        width: Math.max(20,contentWidth)
        color: "white"
        onTextChanged: textField.textChanged(text)
    }
}
