import QtQuick 2.12
import QtQuick.Controls 2.4

CheckBox {
    indicator: Rectangle {
         implicitWidth: 26
         implicitHeight: 26
         x: 0
         y: parent.height / 2 - height / 2
         radius: 3
         border.color: "gray"
         Rectangle {
             width: 14
             height: 14
             x: 6
             y: 6
             radius: 2
             color: "Black"
             visible: parent.parent.checked
         }
    }
    contentItem: Text {
        color: "white";
        text: parent.text;
        leftPadding: parent.indicator.width;
        verticalAlignment: Text.AlignVCenter;
    }
}
