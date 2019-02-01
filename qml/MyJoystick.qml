import QtQuick 2.0
import QtQuick.Controls 2.0

Rectangle { id: root
    property alias caption: txt.text
    property double xpos: 0.5
    property double ypos: 0.5
    property double overallow: 0.4
    property double zpos: 0.5
    property double speed: 0.1
    property double xspeed: 0
    property double yspeed: 0

    Timer {
        interval: 100; running: root.xspeed!=0 || root.yspeed!=0; repeat: true
        onTriggered: {
            if (xpos+xspeed>=-overallow && xpos+xspeed<=1+overallow) xpos+= xspeed;
            if (ypos+yspeed>=-overallow && ypos+yspeed<=1+overallow) ypos+= yspeed;
        }
    }

    border.color: "Black"
    border.width: 1
    color: "White"

    Text { id: txt; x: parent.border.width*2; y: parent.border.width; }

    Button {
        property bool noAdaptFontSize: true
        font.pixelSize: txt.font.pixelSize*3/4
        x: txt.width+txt.x*2; y: parent.border.width; width: parent.width-parent.border.width*2-x; height: txt.height
        text: qsTr("Speed");
        onClicked: {
            if (Math.abs(root.speed-0.1 )<1e-6) { root.speed= 0.01; text= qsTr("slow"); }
            else if (Math.abs(root.speed-0.01)<1e-6) { root.speed= 0.05; text= qsTr("medium"); }
            else if (Math.abs(root.speed-0.05)<1e-6) { root.speed= 0.1; text= qsTr("fast"); }
        }

    }

    Rectangle { id: stick
         width: parent.height-parent.border.width*4-txt.height
         height: width
         x: parent.border.width*2
         y: parent.border.width*2+txt.height
         border.color: "Black"
         color: "Gray"
         border.width: 1
         radius: width*0.5

         MouseArea {
             anchors.fill: parent
             acceptedButtons: Qt.LeftButton
             hoverEnabled: true
             function doMouseChange() {
                 if (pressed) {
                    joy.x= Math.min(width-joy.width, Math.max(0, mouseX-joy.width/2))
                    joy.y= Math.min(height-joy.height, Math.max(0, mouseY-joy.height/2))
                    var x= mouseX-width/2
                    var y= mouseY-height/2
                    var a= ((Math.atan2(x, y)+Math.PI)*4/Math.PI+0.5)%8;
                    var d= Math.sqrt(x*x+y*y);
                    if (d>width/8)
                    {
                        var spd= d/width*speed;
                        if (a<1)      { root.xspeed=    0; root.yspeed=  spd; }
                        else if (a<2) { root.xspeed= -spd; root.yspeed=  spd; }
                        else if (a<3) { root.xspeed= -spd; root.yspeed=   0; }
                        else if (a<4) { root.xspeed= -spd; root.yspeed= -spd; }
                        else if (a<5) { root.xspeed=    0; root.yspeed= -spd; }
                        else if (a<6) { root.xspeed=  spd; root.yspeed= -spd; }
                        else if (a<7) { root.xspeed=  spd; root.yspeed=    0; }
                        else if (a<8) { root.xspeed=  spd; root.yspeed=  spd; }
                        else { root.xspeed= 0; root.yspeed= 0; }
                    } else { root.xspeed= 0; root.yspeed= 0; }
                } else {
                    joy.x= (parent.height-joy.height)/2; joy.y= (parent.width-joy.width)/2;
                    root.xspeed= 0; root.yspeed= 0;
                }
             }
             onPositionChanged: doMouseChange()
             onReleased: doMouseChange()
             onPressed: doMouseChange()
             onWheel: { wheel.accepted= true; if (wheel.angleDelta.y>0) scopeView.model.couderz+= 0.05; if (wheel.angleDelta.y<0) scopeView.model.couderz-= 0.05; }
         }

         Rectangle { id: joy
              width: parent.height/3
              height: width
              x: (parent.height-height)/2
              y: (parent.width-width)/2
              color: "Black"
              radius: width*0.5
         }
    }
}
