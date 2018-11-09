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
import CBScopeCouder 1.0
import CBScopeMes 1.0

ApplicationWindow {
    visible: true
    width: AppTheme.mainPageWidth
    height: AppTheme.mainPageHeight
    id: window
    property int fontSize: 10
    property alias editTextHeight: scopeView.editTextHeight

    SwipeView {
        id: scopeView
        property CBSModelScope model
        anchors.fill: parent
        currentIndex: tabBar.currentIndex
        property alias editTextHeight: mainPageCol.editTextHeight

        //********************************
        // Main page: scope definition!!!!
        //********************************
        Column {
            id: mainPageCol
            spacing: 10
            width: window.width
            property alias editTextHeight: scopeDiametreEdit.height
            Row { spacing: 10;
                width: parent.width
                Text { id: scr11; y: (parent.height-height)/2; text: qsTr("scope Name") }
                MyText { id: scr12; y: (parent.height-height)/2; text: scopeView.model.name; onTextChanged: scopeView.model.name= text }
                Rectangle { height: parent.height; width: parent.width-scr11.width-scr12.width-scr13.width-3*parent.spacing; }
                Button { id: scr13;
                    visible: CBSModel.scopes.count>1
                    text: qsTr("Delete")
                    onClicked: CBSModel.removeScope(scopes.currentIndex);
                }
            }
            Text { text: qsTr("Comments") }
            MyMultiText { text: scopeView.model.comments; onTextChanged: scopeView.model.comments= text; }
            Row { spacing: 10;  id: scopeDiametreEdit;
                Text { y: (parent.height-height)/2; text: qsTr("Diametre") }
                MyText { text: scopeView.model.diametre; onTextChanged: scopeView.model.diametre= Number(text); }
                Text { y: (parent.height-height)/2; text: qsTr("Thicnkess") }
                MyText { text: scopeView.model.thickness; onTextChanged: scopeView.model.thickness= Number(text); }
            }
            Row { spacing: 10;
                Text { y: (parent.height-height)/2; text: qsTr("focal length") }
                MyText { text: scopeView.model.focal; onTextChanged: scopeView.model.focal= Number(text); }
                Text { y: (parent.height-height)/2; text: qsTr("focal ratio") }
                MyText { text: scopeView.model.focal/scopeView.model.diametre; onTextChanged: scopeView.model.focal= Number(text)*scopeView.model.diametre; }
                Text { y: (parent.height-height)/2; text: qsTr("Mirror ROC") }
                MyText { text: scopeView.model.focal*2; onTextChanged: scopeView.model.focal= Number(text)/2; }
                MyOText { y: (parent.height-height)/2; caption: qsTr("sagita"); text: scopeView.model.sagita.toFixed(1); }
            }
            Row { spacing: 10;
                Text { y: (parent.height-height)/2; text: qsTr("secondary small diametre") }
                MyText { text: scopeView.model.secondary; onTextChanged: scopeView.model.secondary= Number(text); }
                Text { y: (parent.height-height)/2; text: qsTr("secondary to focal plane") }
                MyText { text: scopeView.model.secondaryToFocal; onTextChanged: scopeView.model.secondaryToFocal= Number(text); }
            }
            MyOText { caption: qsTr("secondary offset toward primary"); text: scopeView.model.secondaryOffset.toFixed(1); }
        }

        //********************************
        // EyePeice page
        //********************************
        Column {
            width: window.width; height: parent.height; spacing: 10
            Button { x: (parent.width-width)/2; text: qsTr("New eyepeice"); onClicked: epListView.currentIndex= scopeView.model.addEp("Ep Name", 20, 70); }

            Rectangle {
                width: parent.width
                height: parent.height-y-10
                border.width: 3; radius: 2; border.color: "Blue"
                ListView {
                    id: epListView
                    clip: true
                    model: scopeView.model.eps
                    width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width
                    x: parent.border.width; y: parent.border.width
                    delegate: Rectangle {
                            width: parent.width
                            border.width: 3; radius: 2; border.color: "Black"
                            height: col2.height+3*border.width
                            Column {
                                id: col2
                                clip: true
                                x: parent.border.width*1.5; y: parent.border.width*1.5
                                width: parent.width-3*parent.border.width
                                height: i12.height+i13.height+i14.height
                                Row {
                                    spacing: 10; width: parent.width; id: i12;
                                    Text   { id: rowEp1; y: (parent.height-height)/2; text: qsTr("name"); }
                                    MyText { id: rowEp2; y: (parent.height-height)/2; text: name; onTextChanged: name= text; }
                                    Rectangle { height: parent.height; width: parent.width-rowEp1.width-rowEp2.width-rowEp5.width-3*parent.spacing; }
                                    Button { id: rowEp5; y: (parent.height-height)/2; text: qsTr("Delete"); onPressed: scopeView.model.deleteEp(index); }
                                }
                                Row { spacing: 10; id: i13;
                                    Text { y: (parent.height-height)/2; text: qsTr("focal") }
                                    MyText { text: focal; onTextChanged: focal= Number(text); }
                                    Text { y: (parent.height-height)/2; text: qsTr("angle") }
                                    MyText { text: angle; onTextChanged: angle= Number(text); }
                                }
                                Row { spacing: 10; id: i14;
                                    MyOText { caption: qsTr("zoom");       text: zoom.toFixed(0); }
                                    MyOText { caption: qsTr("view Angle"); text: viewAngle.toFixed(2)+"°"; }
                                    MyOText { caption: qsTr("field radius");      text: field.toFixed(1); }
                                }
                            }
                     }
                }
            }
        }

        //********************************
        // Secondary sizing page
        //********************************
        Column {
            width: window.width; height: parent.height;
            Row { spacing: 10; width: parent.width;
                Text   { y: (parent.height-height)/2; text: qsTr("Secondary sizes to study"); }
                MyText { y: (parent.height-height)/2; text: scopeView.model.secondariesToConcider; onTextChanged: scopeView.model.secondariesToConcider= text; }
            }
            CBScopeIlumination {
                id: scopeIlumination
                width: parent.width; height: parent.height-y;
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
            width: window.width; height: parent.height; spacing: 10; id: hogPage
            Row { spacing: 10; width: parent.width;
                Text   { y: (parent.height-height)/2; text: qsTr("Spherometer average leg distance"); }
                MyText { y: (parent.height-height)/2; text: scopeView.model.spherometerLegDistances; onTextChanged: scopeView.model.spherometerLegDistances= text; }
            }
            Rectangle {
                width: parent.width;
                height: parent.height-y-bottomInfo.height-10
                border.width: 3; radius: 4; border.color: "Blue";
                Column {
                    width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width; x: parent.border.width; y: parent.border.width
                    clip: true;
                    Button { x: (parent.width-width)/2; id: addHogBut; text: qsTr("add measure"); onPressed: hoggingListView.currentIndex= scopeView.model.addSpherometer() }
                    ListView {
                        id: hoggingListView
                        clip: true
                        model: scopeView.model.hoggings
                        width: parent.width; height: parent.height;
                        delegate:  Rectangle {
                                width: parent.width
                                border.width: 3; radius: 4; border.color: "Black";
                                height: col.height+2*border.width
                                Column {
                                    id: col
                                    clip: true
                                    width: parent.width-2*parent.border.width; x: parent.border.width; y: parent.border.width
                                    height: row1Hog.height+row2Hog.height+row3Hog.height+row4Hog.height
                                    Row {
                                        spacing: 10; width: parent.width; id: row1Hog;
                                        Text   { id: rowHog1; y: (parent.height-height)/2; text: qsTr("comments"); }
                                        MyMultiText { id: rowHog2; y: (parent.height-height)/2; text: comments; onTextChanged: comments= text; }
                                        Rectangle { height: parent.height; width: parent.width-rowHog1.width-rowHog2.width-rowHog5.width-3*parent.spacing; }
                                        Button { id: rowHog5; y: (parent.height-height)/2; text: qsTr("Delete"); onPressed: scopeView.model.deleteEp(index); }
                                    }
                                    Row { spacing: 10; id: row2Hog;
                                        Text { y: (parent.height-height)/2; text: qsTr("start ROC") }
                                        MyText { text: startSphere; onTextChanged: startSphere= Number(text); }
                                        Text { y: (parent.height-height)/2; text: qsTr("end ROC") }
                                        MyText { text: endSphere; onTextChanged: endSphere= Number(text); }
                                        Text { y: (parent.height-height)/2; text: qsTr("end ROC spherometer") }
                                        MyText { onTextChanged: { if (text!=="") scopeView.model.getSpherometer(index).setEndSphereSpherometer(Number(text)); } }
                                    }
                                    Row { spacing: 10; id: row3Hog;
                                        Text { y: (parent.height-height)/2; text: qsTr("time") }
                                        MyText { text: time; onTextChanged: time= Number(text); }
                                        Text { y: (parent.height-height)/2; text: qsTr("grit") }
                                        MyText { text: grit; onTextChanged: grit= Number(text); }
                                    }
                                    Row { spacing: 10; id: row4Hog;
                                        MyOText { caption: qsTr("hog/time");   text: hogSpeed.toFixed(1); }
                                        MyOText { caption: qsTr("end sagita"); text: endSagita.toFixed(2); }
                                    }
                                }
                        }
                    }
                }
            }
            Rectangle {
                id: bottomInfo
                border.width: 3; radius: 4; border.color: "Black";
                width: parent.width; height: bottomInfoCol.height+2*border.width
                Column {
                    id: bottomInfoCol
                    clip: true
                    width: parent.width-2*parent.border.width; x: parent.border.width; y: parent.border.width
                    height: sphColi4.height+sphColi5.height
                    Row { spacing: 10; id: sphColi4;
                        MyOText { caption: qsTr("time to hog with current grit");   text: scopeView.model.hogTimeWithGrit.toFixed(2); }
                        MyOText { caption: qsTr("%done"); text: ((1-scopeView.model.leftToHog/scopeView.model.toHog)*100).toFixed(2); }
                    }
                    Row { spacing: 10; id: sphColi5;
                        MyOText { caption: qsTr("total to hog");   text: scopeView.model.toHog.toFixed(2); }
                        MyOText { caption: qsTr("left to hog"); text: scopeView.model.leftToHog.toFixed(2); }
                    }
                }
            }
        }

        //********************************
        // parabolizing page
        //********************************
        Column { spacing: 5
            // Popup asking if the user wants to recalc zones...
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
            Row {
                spacing: 10
                Text     { y: (parent.height-height)/2; text: qsTr("nb zones (")+scopeView.model.nbZonesSuggested+qsTr(" suggested)"); }
                MyText   { y: (parent.height-height)/2; text: scopeView.model.nbZones; onTextChanged: scopeView.model.nbZones= Number(text); }
                Button   { y: (parent.height-height)/2; text: qsTr("Auto calc zones"); onPressed: popup.open() }
                CheckBox { y: (parent.height-height)/2; text: "Slit is moving"; checked: scopeView.model.slitIsMoving; onCheckedChanged: scopeView.model.slitIsMoving= checked }
                Button   { y: (parent.height-height)/2; text: qsTr("Couder Mask"); onPressed: scopeView.model.printCouder() }
            }
            Text { text: qsTr("Enter zones boundaries from 0 (or more if there is a hole) to mirror radius)") }
            Rectangle {
                border.width: 3; radius: 4; border.color: "Blue";
                width: parent.width
                height: editTextHeight+2*border.width
                ListView {
                    id: zones; model: scopeView.model.zones; orientation: Qt.Horizontal;
                    width: parent.width-2*parent.border.width; height: editTextHeight; x: parent.border.width; y: parent.border.width;
                    delegate: MyText { text: val; onTextChanged: val= Number(text); }
                }
            }
            Rectangle {
                width: parent.width
                height: parent.height-y
                border.width: 3; radius: 4; border.color: "Black";
                Button { x: (parent.width-width)/2; id: addMesBut; y: parent.border.width; text: qsTr("add measure"); onPressed: scopeView.model.addMesure() }
                ListView {
                    clip: true;
                    model: scopeView.model.parabolizings;
                    x: parent.border.width; y: parent.border.width+addMesBut.height;
                    width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width-addMesBut.height;
                    delegate: Rectangle {
                        border.width: 3; radius: 4; border.color: "Blue";
                        width: parent.width; height: 180;
                        Column {
                            clip: true;
                            x: parent.border.width; y: parent.border.width; width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width;
                            ListView {
                                model: mesures; orientation: Qt.Horizontal;
                                width: parent.width; height: editTextHeight;
                                delegate: MyText { text: val; onTextChanged: val= text; }
                            }
                            Row {
                                spacing: 10; width: parent.width
                                Text   { id: rowMes1; y: (parent.height-height)/2; text: qsTr("time"); }
                                MyText { id: rowMes2; y: (parent.height-height)/2; text: time; onTextChanged: time= text; }
                                Text   { id: rowMes3; y: (parent.height-height)/2; text: qsTr("Comments"); }
                                MyText { id: rowMes4; y: (parent.height-height)/2; text: comments; onTextChanged: comments= text; }
                                Rectangle { height: parent.height; width: parent.width-rowMes1.width-rowMes2.width-rowMes3.width-rowMes4.width-rowMes5.width-5*parent.spacing; }
                                Button { id: rowMes5; y: (parent.height-height)/2; text: qsTr("Delete"); onPressed: scopeView.model.removeMesure(index); }
                            }
                            CBScopeMesure {
                                id: couderDisplay
                                scope: scopeView.model; mesure: scopeView.model.getParabolizing(index);
                                width: parent.width; height: parent.height-y;
                            }
                            Connections { target: couderDisplay.mesure; onMesuresChanged: couderDisplay.update(); } // redraw when mesure change...
                        }
                    }
                }
            }
        }

        //********************************
        // Couder page
        //********************************
        Column {
            width: window.width; height: parent.height;
            CBScopeCouder {
                id: scopeCouder
                width: parent.width; height: parent.height;
                scope: scopeView.model
                onScopeChanged: update()
            }
            Connections {
                target: scopeView.model
                onDiametreChanged: { scopeCouder.update(); }
                onNbZonesChanged: { scopeCouder.update(); }
            }
        }

        //********************************
        // Support page
        //********************************
        Column {
            width: window.width; height: parent.height;
            Button { text: "calc"; onClicked: scopeView.model.doMesSolve(); }
            CBScopeMes {
                id: scopeSupport
                width: parent.width; height: parent.height;
                scope: scopeView.model
                onScopeChanged: update()
            }
            Connections {
                target: scopeView.model
                onDiametreChanged: { scopeSupport.update(); }
                onNbZonesChanged: { scopeSupport.update(); }
            }
        }
    }

    //********************************
    // footer page
    //********************************
    footer: Row {
        width: parent.width
            ComboBox { id: scopes
                width: parent.width/5; height: parent.height
                textRole: "name";
                model: CBSModel.scopes
                onCurrentIndexChanged: scopeView.model= CBSModel.getScope(scopes.currentIndex)
            }
            Button {
                width: parent.width/8;
                text: qsTr("add")
                onClicked: scopes.currentIndex= CBSModel.addScope();
            }
            Button {
                width: parent.width/8;
                text: qsTr("help");
                onClicked: CBSModel.help()
            }

            Rectangle { height: parent.height; width: 10; }

            TabBar {
                width: parent.width-x
                id: tabBar
                currentIndex: scopeView.currentIndex
                TabButton { text: qsTr("Scope") }
                TabButton { text: qsTr("EP") }
                TabButton { text: qsTr("Secondary") }
                TabButton { text: qsTr("Hogging") }
                TabButton { text: qsTr("Parabolizing") }
                TabButton { text: qsTr("Couder") }
                TabButton { text: qsTr("Support") }
            }
    }

}
