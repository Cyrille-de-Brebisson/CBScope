import QtQuick 2.9
import QtQuick.Controls 2.4
import Qt.labs.settings 1.0
import QtQuick.Window 2.11
import QtGraphicalEffects 1.0
import "qrc:/qml/" //import the qml folder
import CBSModel 1.0
import CBSModelHoggingWork 1.0
import CBSModelParabolizingWork 1.0
import CBSModelScope 1.0
import QtQuick.Layouts 1.3
import CBScopeIlumination 1.0
import CBScopeMesure 1.0

ApplicationWindow {
    visible: true
    width: AppTheme.mainPageWidth
    height: AppTheme.mainPageHeight
    id: window
    property int fontSize: 10

    SwipeView {
        id: scopeView
        property CBSModelScope model
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        //********************************
        // Main page: scope definition!!!!
        //********************************
        Column {
            width: window.width
            MyInput { fontSize: window.fontSize; caption: "Name"; text: scopeView.model.name; onTextChanged: scopeView.model.name= text }
            Text { id: t1; font.pointSize: fontSize;  text: qsTr("Comments") }
            Rectangle {
                id: textField
                width:  parent.width
                height: t1.height*0.8+t2.height
                clip: true
                border.width: 3
                border.color: "lightgrey"
                radius: 4
                smooth: true
                TextEdit { id: t2;
                    width: parent.width - (2 * font.pointSize)
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "black"
                    font.pointSize: fontSize; text: scopeView.model.comments; onTextChanged: scopeView.model.comments= text; }
            }
            MyInput { fontSize: window.fontSize; caption: "Diametre"; text: scopeView.model.diametre; onTextChanged: scopeView.model.diametre= Number(text); }
            MyInput { fontSize: window.fontSize; caption: "focal length"; text: scopeView.model.focal; onTextChanged: scopeView.model.focal= Number(text); }
            MyInput { fontSize: window.fontSize; caption: "focal ratio"; text: scopeView.model.focal/scopeView.model.diametre; onTextChanged: scopeView.model.focal= Number(text)*scopeView.model.diametre; }
            MyInput { fontSize: window.fontSize; caption: "Mirror ROC"; text: scopeView.model.focal*2; onTextChanged: scopeView.model.focal= Number(text)/2; }
            MyProperty { fontSize: window.fontSize; caption: "sagita"; text: scopeView.model.sagita; }
            MyInput { fontSize: window.fontSize; caption: "secondary small diametre"; text: scopeView.model.secondary; onTextChanged: scopeView.model.secondary= Number(text); }
            MyInput { fontSize: window.fontSize; caption: "secondary to focal plane"; text: scopeView.model.secondaryToFocal; onTextChanged: scopeView.model.secondaryToFocal= Number(text); }
            MyProperty { id: mainoffset; fontSize: window.fontSize; caption: "secondary offset toward primary"; text: scopeView.model.secondaryOffset; }
        }

        //********************************
        // EyePeice page
        //********************************
        Column {
            id: ePPage
            Row {
                id: epCol
                clip: true
                width: parent.width
                height: tabBar.height
                spacing: 20
                Button { font.pointSize: window.fontSize; text: qsTr("New eyepeice");
                    onClicked: epListView.currentIndex= scopeView.model.addEp("Ep Name", 20, 70);
                }
                Button { font.pointSize: window.fontSize; text: qsTr("Delete")+" "+scopeView.model.eps.get(epListView.currentIndex).name;
                    enabled: epListView.count!=0
                    onClicked: epListView.currentIndex= scopeView.model.deleteEp(epListView.currentIndex);
                }
            }

            Rectangle { height: 10; width: parent.width; }

            Rectangle {
                width: parent.width
                border.width: 3
                radius: 2
                height: parent.height-epCol.height-10
                border.color: "Blue"
                ListView {
                    id: epListView
                    clip: true
                    model: scopeView.model.eps
                    width: parent.width-2*parent.border.width
                    height: parent.height-2*parent.border.width
                    x: parent.border.width
                    y: parent.border.width
                    focus: true
                    delegate: Item {
                        width: parent.width; height: rect2.height
                        focus: true
                        MouseArea {
                            anchors.fill: parent
                            onClicked: epListView.currentIndex = index
                        }
                        Rectangle {
                            id: rect2
                            width: parent.width
                            border.width: 3
                            radius: 4
                            border.color: "Black"
                            height: col2.height+2*border.width
                            Column {
                                id: col2
                                clip: true
                                x: parent.border.width
                                y: parent.border.width
                                width: parent.width-2*parent.border.width
                                height: i12.height+i22.height+i62.height+i63.height
                                MyInput { id: i12; fontSize: window.fontSize; caption: qsTr("name"); text: name; onTextChanged: name= text; }
                                MyInput2 { id: i22; fontSize: window.fontSize; caption1: qsTr("focal"); text1: focal; onT1Changed: focal= Number(text1); caption2: qsTr("angle"); text2: angle; onT2Changed: angle= Number(text2); }
                                MyProperty { id: i62; fontSize: window.fontSize; caption: qsTr("field"); text: field; }
                                MyProperty2 { id: i63; fontSize: window.fontSize; caption1: qsTr("zoom"); text1: zoom; caption2: qsTr("view Angle"); text2: viewAngle }
                            }
                        }
                    }
                    highlight: Rectangle { color: "lightsteelblue"; radius: 4 }
                }
            }
        }

        //********************************
        // Secondary sizing page
        //********************************
        Column {
            MyInput { id: secSizes;
                fontSize: window.fontSize;
                caption: qsTr("Secondary sizes to study"); text: scopeView.model.secondariesToConcider;
                onTextChanged: scopeView.model.secondariesToConcider= text;
            }
            CBScopeIlumination {
                id: scopeIlumination
                width: parent.width; height: parent.height-secSizes.height;
                scope: scopeView.model
            }
            Connections {
                target: scopeView.model
                onDiametreChanged: { scopeIlumination.update(); }
                onFocalChanged: { scopeIlumination.update(); }
                onSecondaryToFocalChanged: { scopeIlumination.update(); }
                onSecondaryChanged: { scopeIlumination.update(); }
                onSecondariesToConciderChanged: { scopeIlumination.update(); }
                onEpsChanged: { scopeIlumination.update(); }
            }
        }

        //********************************
        // Hogging page
        //********************************
        Column {
            id: hogPage
            MyInput { fontSize: window.fontSize; id: sphsetup; caption: qsTr("Spherometer average leg distance"); text: scopeView.model.spherometerLegDistances; onTextChanged: scopeView.model.spherometerLegDistances= text; }
            Rectangle { height: 10; width: parent.width; }
            Rectangle {
                width: parent.width
                border.width: 3
                radius: 2
                height: parent.height-newSphReading.height-sphsetup.height-bottomInfo.height-20
                border.color: "Blue"
                ListView {
                    id: hoggingListView
                    clip: true
                    model: scopeView.model.hoggings
                    width: parent.width-2*parent.border.width
                    height: parent.height-2*parent.border.width
                    x: parent.border.width
                    y: parent.border.width
                    focus: true
                    delegate: Item {
                        width: parent.width; height: rect.height
                        focus: true
                        MouseArea {
                            anchors.fill: parent
                            onClicked: hoggingListView.currentIndex = index
                        }
                        Rectangle {
                            id: rect
                            width: parent.width
                            border.width: 3
                            radius: 4
                            border.color: "Black"
                            height: col.height+2*border.width
                            focus: true
                            Column {
                                id: col
                                clip: true
                                x: parent.border.width
                                y: parent.border.width
                                width: parent.width-2*parent.border.width
                                height: i1.height+i2.height+i4.height+i6.height
                                focus: true
                                MyInput { id: i1; fontSize: window.fontSize; caption: qsTr("comments"); text: comments; onTextChanged: comments= text; }
                                MyInput2 { id: i2; fontSize: window.fontSize; caption1: qsTr("start radius"); text1: startSphere; onT1Changed: startSphere= Number(text1); caption2: qsTr("end radius"); text2: endSphere; onT2Changed: endSphere= Number(text2); }
                                MyInput2 { id: i4; fontSize: window.fontSize; caption1: qsTr("time"); text1: time; onT1Changed: time= Number(text1); caption2: qsTr("grit"); text2: grit; onT2Changed: grit= Number(text2); }
                                MyProperty2 { id: i6; fontSize: window.fontSize; caption1: qsTr("hog/time"); text1: hogSpeed; caption2: qsTr("End Sagita"); text2: endSagita; }
                            }
                        }
                    }
                    highlight: Rectangle { color: "lightsteelblue"; radius: 4 }
                }
            }
            Rectangle { height: 10; width: parent.width; }
            Rectangle {
                id: newSphReading
                width: parent.width
                border.width: 3
                radius: 2
                border.color: "Black"
                height: sphCol.height+2*border.width
                Column {
                    id: sphCol
                    clip: true
                    x: parent.border.width
                    y: parent.border.width
                    width: parent.width-2*parent.border.width
                    height: sphColt1.height+sphColi1.height+sphColi3.height+sphColr.height
                    Text { id: sphColt1; font.pointSize: window.fontSize; text: qsTr("New grinding work") }
                    MyInput2 { id: sphColi1; fontSize: window.fontSize; text1: "0"; caption1: qsTr("time"); text2: "0"; caption2: qsTr("reading"); }
                    MyInput { id: sphColi3; fontSize: window.fontSize; text: "80"; caption: qsTr("grit"); onTextChanged: sphColi4.text1= scopeView.model.getHogTimeWithGrit(Number(sphColi3.text)); }
                    Row { id: sphColr; width: parent.width; height: tabBar.height
                        spacing: 20
                        Button { font.pointSize: window.fontSize; text: qsTr("Spherometer reading");
                            onClicked: hoggingListView.currentIndex= scopeView.model.addSpherometer(Number(sphColi1.text1), Number(sphColi1.text2), false, Number(sphColi3.text));
                        }
                        Button { font.pointSize: window.fontSize; text: qsTr("radius reading");
                            onClicked: hoggingListView.currentIndex= scopeView.model.addSpherometer(Number(sphColi1.text1), Number(sphColi1.text2), true, Number(sphColi3.text));
                        }
                        Button { font.pointSize: window.fontSize; text: qsTr("delete last reading");
                            onClicked: hoggingListView.currentIndex= scopeView.model.removeSpherometer();
                        }
                    }
                }
            }
            Rectangle {
                id: bottomInfo
                width: parent.width
                border.width: 3
                radius: 2
                border.color: "Black"
                height: bottomInfoCol.height+2*border.width
                Column {
                    id: bottomInfoCol
                    clip: true
                    x: parent.border.width
                    y: parent.border.width
                    width: parent.width-2*parent.border.width
                    height: sphColi4.height+2*parent.border.width+sphColi5.heigh
                    MyProperty2 { id: sphColi4; fontSize: window.fontSize; text1: scopeView.model.getHogTimeWithGrit(Number(sphColi3.text)); caption1: qsTr("time to hog with current grit"); caption2: qsTr("%done"); text2: (1-scopeView.model.leftToHog/scopeView.model.toHog)*100; }
                    MyProperty2 { id: sphColi5; fontSize: window.fontSize; text1: scopeView.model.toHog; caption1: qsTr("total to hog"); text2: scopeView.model.leftToHog; caption2: qsTr("left to hog");}
                    Connections {
                        target: scopeView.model
                        onHogTimeWithGritChanged: sphColi4.text1= scopeView.model.getHogTimeWithGrit(Number(sphColi3.text));
                    }
                }
            }
        }

        //********************************
        // parabolizing page
        //********************************
        Column {
            Popup { id: popup; x: 100; y: 100; width: parent.width-200; height: parent.height-200; modal: true; focus: true; clip: true;
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
                    ColumnLayout {
                        Text { Layout.fillWidth: true; text: qsTr("Are you sure you want to change your zones?") }
                        RowLayout {
                            Layout.fillWidth: true;
                            Button { text: qsTr("Yes!"); onPressed: { scopeView.model.autoZones(); popup.close(); } }
                            Button { text: qsTr("NO! ABORT!"); onPressed: popup.close(); }
                        }
                    }
                  }
            MyInput { id: parabolizingInput1; fontSize: window.fontSize; caption: qsTr("nb zones (")+scopeView.model.nbZonesSuggested+qsTr(" suggested)"); text: scopeView.model.nbZones; onTextChanged: scopeView.model.nbZones= Number(text); }
            Row { id: paraRow1
                Button { text: qsTr("Auto calc zones"); onPressed: popup.open() } //scopeView.model.autoZones() }
                CheckBox { text: "Slit is moving"; checked: scopeView.model.slitIsMoving; onCheckedChanged: scopeView.model.slitIsMoving= checked }
                Text { font.pointSize: window.fontSize; text: qsTr("enter the zones boundaries (from 0 (or more) to mirror radius)") }
            }
            Rectangle {
                id: paraRec1
                width: parent.width
                height: parabolizingInput1.height+border.width*2
                border.width: 3; radius: 4; border.color: "Blue";
                ListView {
                    id: zones; model: scopeView.model.zones; orientation: Qt.Horizontal;
                    width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width; x: parent.border.width; y: parent.border.width;
                    delegate: Rectangle {
                        id: zonesRect; height: parent.height;
                        border.width: 3; radius: 4; border.color: "Black"
                        TextInput {
                            id: zonesRectti; font.pointSize: window.fontSize; text: val; onTextChanged: val= Number(text);
                            clip: true
                            x: parent.border.width
                            y: parent.border.width
                            width: Math.max(50,contentWidth)
                        }
                        width: zonesRectti.width+2*border.width
                    }
                }
            }
            Rectangle {
                width: parent.width
                height: parent.height-parabolizingInput1.height-paraRow1.height-paraRec1.height
                border.width: 3; radius: 4; border.color: "Blue";
                Button { id: addMesBut; x: parent.border.width; y: parent.border.width; text: qsTr("add mesure"); onPressed: scopeView.model.addMesure() }
                ListView {
                    id: parabolizingList
                    clip: true;
                    model: scopeView.model.parabolizings;
                    x: parent.border.width; y: parent.border.width+addMesBut.height;
                    width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width-addMesBut.height;
                    delegate: Rectangle {
                        border.width: 3; radius: 4; border.color: "Blue";
                        width: parent.width;
                        height: 180;
                        color: index==parabolizingList.currentIndex ? "lightsteelblue" : "White"
                        MouseArea { anchors.fill: parent; onClicked: parabolizingList.currentIndex=index }
                        Column {
                            clip: true;
                            x: parent.border.width; y: parent.border.width; width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width;
                            ListView {
                                id: scopeMesureSubListView
                                model: mesures; orientation: Qt.Horizontal;
                                width: parent.width; height: 28;
                                delegate: Rectangle {
                                    clip: true
                                    height: mesRects.height+border.width*2;
                                    border.width: 3; radius: 4; border.color: "Black"
                                    TextInput {
                                        id: mesRects; font.pointSize: window.fontSize; text: val; onTextChanged: val= Number(text);
                                        x: parent.border.width
                                        y: parent.border.width
                                        width: Math.max(50,contentWidth)
                                    }
                                    width: mesRects.width+2*border.width
                                }
                            }
                            MyInput2 { fontSize: window.fontSize;
                                       id: mesureComment; text1: comments; caption1: qsTr("Comments"); onText1Changed: comments= text1; text2: time; caption2: qsTr("time");  onText2Changed: time= text2; }
                            CBScopeMesure {
                                id: couderDisplay
                                scope: scopeView.model; mesure: scopeView.model.getParabolizing(index);
                                width: parent.width; height: parent.height-(mesureComment.y+mesureComment.height);
                            }
                            Connections {
                                target: couderDisplay.mesure
                                onMesuresChanged: couderDisplay.update() // redraw when mesure change...
                            }
                        }
                    }
                }
            }
        }
    }

    //********************************
    // footer page
    //********************************
    footer: Row {
            ComboBox {
                width: parent.width/5
                height: parent.height
                id: scopes
                textRole: "name"
                model: CBSModel.scopes
                onCurrentIndexChanged: scopeView.model= CBSModel.getScope(scopes.currentIndex)
            }
            Button {
                width: parent.width/8
                height: parent.height
                text: qsTr("add")
                onClicked: scopes.currentIndex= CBSModel.addScope();
            }
            Button {
                width: parent.width/8
                enabled: CBSModel.scopes.count!==1
                height: parent.height
                text: qsTr("Delete")
                onClicked: { CBSModel.removeScope(scopes.currentIndex);  scopeView.model= CBSModel.getScope(scopes.currentIndex); }
            }

            Rectangle { height: parent.height; width: 10; color: "Blue" }

            TabBar {
            id: tabBar
            currentIndex: scopeView.currentIndex
            TabButton { text: qsTr("Scope") }
            TabButton { text: qsTr("EP") }
            TabButton { text: qsTr("Secondary") }
            TabButton { text: qsTr("Hogging") }
            TabButton { text: qsTr("Parabolizing") }
        }
    }

}
