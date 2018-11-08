import QtQuick 2.0

Rectangle {
    id: textField
    border.width: 3; border.color: textInput.focus ? "#569ffd" : "lightgrey"; radius: 4;
    height: textInput.height+border.width*4
    width: Math.max(20,textInput.width)+border.width*4
    clip: true
    smooth: true
    property alias text: textInput.text
    TextEdit {
        id: textInput
        clip: true
        x: parent.border.width*2; y: parent.border.width*2;
//        font.pointSize: r1.fontSize
        color: "black"
        onTextChanged: textField.textChanged(text)
    }
}
