import QtQuick 2.0

Row {
    id: r1
    property alias caption: t1.text
    property alias text: textField.text
    property int fontSize: 17
//    property alias readOnly: textField.readOnly

    width: parent.width
    spacing: 20
    Text { anchors.verticalCenter: parent.verticalCenter; id: t1; font.pointSize: r1.fontSize; }
    Rectangle {
        id: textField
        width:  r1.width-t1.width-r1.spacing;
        height: textInput.height * 1.8
        clip: true
        border.width: 3
        border.color: "lightgrey"
        radius: 4
        smooth: true
        property alias text: textInput.text
        //signal textChanged(string text)
        TextInput {
            id: textInput
            clip: true
            width: parent.width - (2 * font.pointSize)
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.pointSize: r1.fontSize
            color: "black"
            onTextChanged: textField.textChanged(text)
            onFocusChanged: {
                if(focus){
                    textField.border.color = "#569ffd"
                }else{
                    textField.border.color = "lightgray"
                }
            }
        }
    }
}
