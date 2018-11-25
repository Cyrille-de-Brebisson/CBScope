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
import QtMultimedia 5.8
import CBScopeVirtualCouder 1.0

ApplicationWindow {
    visible: true
    width: AppTheme.mainPageWidth
    height: AppTheme.mainPageHeight
    id: window
    property alias editTextHeight: scopeView.editTextHeight
    property int fontSize: CBSModel.windowsFont
    property bool fontBold: CBSModel.windowsFontBold

    // recursive set font to all controls
    function setFont(parentElement) {
        for (var i = 0; i < parentElement.children.length; ++i)
        {
            if (parentElement.children[i].font)
            {
                parentElement.children[i].font.bold = Qt.binding(function() { return window.fontBold })
                parentElement.children[i].font.pixelSize = Qt.binding(function() { return window.fontSize })
            }
            setFont(parentElement.children[i]);
        }
    }
    Component.onCompleted: { // force font change on all controls
        setFont(header);
        setFont(footer);
        setFont(scopeView);
    }

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

    //********************************
    // Header: scope selection, new scope creation, and font control
    //********************************
    header: Row {
        width: parent.width; height: headerCb.height*1.5; spacing: 10
        ComboBox { id: scopes
            textRole: "name";
            model: CBSModel.scopes
            onCurrentIndexChanged: scopeView.model= CBSModel.scopes.get(scopes.currentIndex)
        }
        Button {
            text: qsTr("New Scope")
            onClicked: scopes.currentIndex= CBSModel.addScope(scopes.currentIndex);
        }
        Button {
            text: qsTr("Help");
            onClicked: CBSModel.help()
        }
        ComboBox {
            textRole: "text";
            model: ListModel { ListElement { text: "font 10"; } ListElement { text: "font 11"; } ListElement { text: "font 12"; } ListElement { text: "font 13"; } ListElement { text: "font 14"; } ListElement { text: "font 15"; } ListElement { text: "font 16"; } }
            currentIndex: window.fontSize-10
            onCurrentIndexChanged: CBSModel.windowsFont= window.fontSize= currentIndex+10;
        }
        CheckBox { id: headerCb; text: qsTr("Bold"); checked: window.fontBold; onCheckedChanged: CBSModel.windowsFontBold= window.fontBold= checked }
    }

    //*******************************************************
    // The main control. A list of pages with all we need...
    StackLayout {
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
                    onClicked: CBSModel.scopes.remove(scopes.currentIndex);
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
            Row { spacing: 10;
                Text { y: (parent.height-height)/2; text: qsTr("Density (g/cm^3)") }
                MyText { y: (parent.height-height)/2; text: scopeView.model.density; onTextChanged: scopeView.model.density= Number(text); }
                Text { y: (parent.height-height)/2; text: qsTr("Weight (Kg)") }
                MyOText { y: (parent.height-height)/2; text: scopeView.model.weight.toFixed(3); }
                Text { y: (parent.height-height)/2; text: qsTr("Young (KgF/mm²)") }
                MyText { y: (parent.height-height)/2; text: scopeView.model.young; onTextChanged: scopeView.model.young= Number(text); }
                Text { y: (parent.height-height)/2; text: qsTr("Poisson") }
                MyText { y: (parent.height-height)/2; text: scopeView.model.poisson; onTextChanged: scopeView.model.poisson= Number(text); }
                ComboBox { y: (parent.height-height)/2; 
                           model: ListModel { ListElement { name: "Pyrex 7740" } 
                                              ListElement { name: "Zerodur" } 
                                              ListElement { name: "Plate Glass" } 
                                              ListElement { name: "Fused scilica" } 
                                              ListElement { name: "Duran 50" } }
                            textRole: "name";
                            onCurrentIndexChanged: { scopeView.model.density= CBSModel.materials(currentIndex, 2); scopeView.model.poisson= CBSModel.materials(currentIndex, 1); scopeView.model.young= CBSModel.materials(currentIndex, 0); }
                }
            }
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
                        Component.onCompleted: setFont(this)
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
                                Button { id: rowEp5; y: (parent.height-height)/2; text: qsTr("Delete"); onPressed: scopeView.model.eps.remove(index); }
                            }
                            Row { spacing: 10; id: i13;
                                Text { y: (parent.height-height)/2; text: qsTr("focal") }
                                MyText { text: focal; onTextChanged: focal= Number(text); }
                                Text { y: (parent.height-height)/2; text: qsTr("angle") }
                                MyText { text: angle; onTextChanged: angle= Number(text); }
                            }
                            Row { spacing: 10; id: i14;
                                MyOText { caption: qsTr("zoom");         text: zoom.toFixed(0); }
                                MyOText { caption: qsTr("view Angle");   text: viewAngle.toFixed(2)+"°"; }
                                MyOText { caption: qsTr("field radius"); text: field.toFixed(1); }
                                MyOText { caption: qsTr("exit pupil");   text: pupil.toFixed(1); }
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
                CheckBox { y: (parent.height-height)/2; text: qsTr("2\" focusser"); checked: scopeIlumination.twoInches; onCheckedChanged: scopeIlumination.twoInches= checked; }
            }
            CBScopeIlumination {
                id: scopeIlumination
                width: parent.width; height: parent.height-y;
                scope: scopeView.model
                onTwoInchesChanged: update();
            }
            Connections {
                target: scopeView.model
                onDiametreChanged: { scopeIlumination.update(); }
                onFocalChanged: { scopeIlumination.update(); }
                onSecondaryToFocalChanged: { scopeIlumination.update(); }
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
                    Button { x: (parent.width-width)/2; id: addHogBut; text: qsTr("add measure"); onPressed: hoggingListView.currentIndex= scopeView.model.addHogging() }
                    ListView {
                        id: hoggingListView
                        clip: true
                        model: scopeView.model.hoggings
                        width: parent.width; height: parent.height-y;
                        delegate:  Rectangle {
                            Component.onCompleted: setFont(this)
                            width: parent.width
                            border.width: 3; radius: 4; border.color: "Black";
                            height: col.height+2*border.width
                            Column {
                                id: col
                                clip: true
                                width: parent.width-2*parent.border.width; x: parent.border.width; y: parent.border.width
                                height: row1Hog.height+row2Hog.height+row3Hog.height+row4Hog.height
                                Row {
                                    spacing: 10; width: parent.width; id: row1Hog; height: rowHog2.height
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
                                    MyText { onTextChanged: { if (text!=="") scopeView.model.hoggings.get(index).setEndSphereSpherometer(Number(text)); } }
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
                                    MyOText { caption: qsTr("% done in this run"); text: percentDone.toFixed(2); }
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
        Column { spacing: 5; id: parabolizingColumn
            property int mesureHeight: 180
            Row {
                spacing: 10
                Text     { y: (parent.height-height)/2; text: qsTr("nb zones (")+scopeView.model.nbZonesSuggested+qsTr(" suggested)"); }
                MyText   { y: (parent.height-height)/2; text: scopeView.model.nbZones; onTextChanged: scopeView.model.nbZones= Number(text); }
                Button   { y: (parent.height-height)/2; text: qsTr("Auto calc zones"); onPressed: popup.open() }
                CheckBox { y: (parent.height-height)/2; text: qsTr("Slit is moving"); checked: scopeView.model.slitIsMoving; onCheckedChanged: scopeView.model.slitIsMoving= checked }
                CheckBox { y: (parent.height-height)/2; text: qsTr("large View"); checked: false; onCheckedChanged: { parabolizingColumn.mesureHeight= checked?250:180; } }
            }
            Text { text: qsTr("Enter zones boundaries from 0 (or more if there is a hole) to mirror radius") }
            Rectangle {
                border.width: 3; radius: 4; border.color: "Blue";
                width: parent.width
                height: editTextHeight+2*border.width
                ListView {
                    id: zones; model: scopeView.model.zones; orientation: Qt.Horizontal;
                    width: parent.width-2*parent.border.width; height: editTextHeight; x: parent.border.width; y: parent.border.width;
                    delegate: MyText {
                        Component.onCompleted: setFont(this)
                        text: val; onTextChanged: val= Number(text);
                    }
                }
            }
            Rectangle {
                width: parent.width
                height: parent.height-y
                border.width: 3; radius: 4; border.color: "Black";
                Button { x: (parent.width-width)/2; id: addMesBut; y: parent.border.width; text: qsTr("add measure"); onPressed: scopeView.model.addParabolizing() }
                ListView { id: parabolizingList
                    clip: true;
                    model: scopeView.model.parabolizings;
                    x: parent.border.width; y: parent.border.width+addMesBut.height;
                    width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width-addMesBut.height;
                    delegate: Rectangle {
                        Component.onCompleted: setFont(this)
                        border.width: 3; radius: 4; border.color: "Blue";
                        width: parent.width; height: parabolizingColumn.mesureHeight
                        Column {
                            clip: true;
                            x: parent.border.width; y: parent.border.width; width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width;
                            ListView {
                                model: mesures; orientation: Qt.Horizontal;
                                width: parent.width; height: editTextHeight;
                                delegate: MyText { text: val; onTextChanged: val= text; fontcol: error ? "Red" : "Black"; }
                            }
                            Row {
                                spacing: 10; width: parent.width
                                Text   { id: rowMes1; y: (parent.height-height)/2; text: qsTr("time"); }
                                MyText { id: rowMes2; y: (parent.height-height)/2; text: time; onTextChanged: time= text; }
                                Text   { id: rowMes3; y: (parent.height-height)/2; text: qsTr("Comments"); }
                                MyText { id: rowMes4; y: (parent.height-height)/2; text: comments; onTextChanged: comments= text; }
                                Rectangle { height: parent.height; width: parent.width-rowMes1.width-rowMes2.width-rowMes3.width-rowMes4.width-rowMes5.width-5*parent.spacing; }
                                Button { id: rowMes5; y: (parent.height-height)/2; text: qsTr("Delete"); onPressed: scopeView.model.parabolizings.remove(index); }
                            }
                            CBScopeMesure {
                                id: couderDisplay
                                mesure: scopeView.model.parabolizings.get(index);
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
        Rectangle {
            width: window.width; height: parent.height;
            CBScopeCouder {
                id: scopeCouder
                width: parent.width; height: parent.height;
                scope: scopeView.model
                onScopeChanged: update()
                onZoomChanged: update()
            }
            Row {
                spacing: 10; y: 1;
                CheckBox { y: (parent.height-height)/2; text: qsTr("Show Red Zones"); checked: scopeView.model.showCouderRed; onCheckedChanged: scopeView.model.showCouderRed= checked }
                CheckBox { y: (parent.height-height)/2; text: qsTr("Show Blue Zones"); checked: scopeView.model.showCouderBlue; onCheckedChanged: scopeView.model.showCouderBlue= checked }
                //CheckBox { y: (parent.height-height)/2; text: qsTr("Show Orange Zones"); checked: scopeView.model.showCouderOrange; onCheckedChanged: scopeView.model.showCouderOrange= checked }
                CheckBox { y: (parent.height-height)/2; text: qsTr("zoom/2"); checked: scopeCouder.zoom; onCheckedChanged: scopeCouder.zoom= checked }
                Button   { y: (parent.height-height)/2; text: qsTr("Print Couder Mask"); onPressed: scopeView.model.printCouder() }
            }
            Connections {
                target: scopeView.model
                onDiametreChanged: scopeCouder.update();
                onNbZonesChanged: scopeCouder.update();
                onShowCouderRedChanged: scopeCouder.update()
                onShowCouderBlueChanged: scopeCouder.update()
                onShowCouderOrangeChanged: scopeCouder.update()
            }
        }

        //********************************
        // Support page
        //********************************
        Rectangle {
            width: window.width; height: parent.height;
            CBScopeMes {
                id: scopeSupport
                width: parent.width; height: parent.height;
                scope: scopeView.model
            }
            Row {
                spacing: 10; y:1;
                CheckBox { y: (parent.height-height)/2; text: qsTr("Show Forces"); checked: scopeSupport.showForces; onCheckedChanged: scopeSupport.showForces= checked }
                ComboBox { y: (parent.height-height)/2; 
                           model: ListModel { ListElement { name: "3 points" } 
                                              ListElement { name: "6 points" } 
                                              ListElement { name: "9 points" } 
                                              ListElement { name: "18 points" } 
                                              ListElement { name: "27 points" } 
                                              ListElement { name: "36 points" } }
                            textRole: "name";
                            currentIndex: scopeView.model.cellType
                            onCurrentIndexChanged: { scopeView.model.cellType= currentIndex; scopeSupport.update(); }
                 }
                ComboBox { y: (parent.height-height)/2; 
                           model: ListModel { ListElement { name: "100%" } 
                                              ListElement { name: "75%" } 
                                              ListElement { name: "50%" } 
                                              ListElement { name: "25%" } 
                                              ListElement { name: "10%" } 
                                              ListElement { name: "5%" } }
                            textRole: "name";
                            currentIndex: scopeSupport.zoom
                            onCurrentIndexChanged: { scopeSupport.zoom= currentIndex; scopeSupport.update(); }
                 }
                Button { text: "Calc"; onClicked: { scopeView.model.doMesSolve(); scopeSupport.update(); } }
            }
        }
        //********************************
        // webcam page
        //********************************
        Column {
            width: window.width; height: parent.height;
            Row {
                spacing: 10;
                Text { y: (parent.height-height)/2; text: qsTr("Camera") }
                ComboBox { y: (parent.height-height)/2;
                           model: QtMultimedia.availableCameras
                           textRole: "displayName";
                           onCurrentIndexChanged: {
                                console.log("curent index ", currentIndex);
                                if (currentIndex!=-1) { camera.deviceId= QtMultimedia.availableCameras[currentIndex].deviceId; camera.start(); }
                                else { camera.stop(); camera.deviceId= "";  }
                           }
                           currentIndex: -1
                 }
                ComboBox { id: zoneToUse; y: (parent.height-height)/2;
                           model: vcouder.zoneModel
                           textRole: "val";
                           currentIndex: vcouder.zone
                           onCurrentIndexChanged: vcouder.zone= currentIndex
                 }
                ComboBox { y: (parent.height-height)/2; 
                           model: ListModel { ListElement { name: qsTr("Grayscale") } 
                                              ListElement { name: qsTr("Red+Blue+Green") } 
                                              ListElement { name: qsTr("Blue") } 
                                              ListElement { name: qsTr("Green") } 
                                              ListElement { name: qsTr("Red") } }
                            textRole: "name";
                            currentIndex: scopeView.model.virtualCouderType
                            onCurrentIndexChanged: scopeView.model.virtualCouderType= currentIndex; 
                 }
                 CheckBox { y: (parent.height-height)/2; text: qsTr("Pause"); checked: vcouder.pause; onCheckedChanged: vcouder.pause= checked }
            }
            Rectangle {
                width: window.width; height: 180; 
                property CBSModelParabolizingWork mesure: scopeView.model.parabolizings.get(scopeView.model.parabolizings.count-1)
                border.width: 3; radius: 4; border.color: "Blue";
                Column {
                    clip: true;
                    x: parent.border.width; y: parent.border.width; width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width;
                    ListView {
                        model: parent.parent.mesure.mesures; orientation: Qt.Horizontal;
                        width: parent.width; height: editTextHeight;
                        delegate: MyText { text: val; onTextChanged: val= text; fontcol: error ? "Red" : "Black"; }
                    }
                    Row {
                        spacing: 10; width: parent.width
                        Text   { y: (parent.height-height)/2; text: qsTr("time"); }
                        MyText { y: (parent.height-height)/2; text: parent.parent.parent.mesure.time; onTextChanged: parent.parent.parent.mesure.time= text; }
                        Text   { y: (parent.height-height)/2; text: qsTr("Comments"); }
                        MyText { y: (parent.height-height)/2; text: parent.parent.parent.mesure.comments; onTextChanged: parent.parent.parent.mesure.comments= text; }
                    }
                    CBScopeMesure {
                        id: couderDisplay2
                        mesure: parent.parent.mesure
                        width: parent.width; height: parent.height-y;
                    }
                    Connections { target: couderDisplay2.mesure; onMesuresChanged: couderDisplay2.update(); } // redraw when mesure change...
                }
            }    
            Camera { id: camera; deviceId: "nocamera"; Component.onCompleted: camera.stop(); }
            VideoOutput {
                width: window.width; height: parent.height-y;
                source: camera.deviceId!=="nocamera" ? camera : undefined
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: { var p= parent.mapPointToSourceNormalized(Qt.point(mouseX, mouseY)); vcouder.userclick(p.x, p.y); }
                    onPositionChanged: if (pressed) { var p2= parent.mapPointToSourceNormalized(Qt.point(mouseX, mouseY)); scopeView.model.couderx= p2.x; scopeView.model.coudery= p2.y; }
                    onWheel: { wheel.accepted= true; if (wheel.angleDelta.y>0) scopeView.model.couderz+= 0.05; if (wheel.angleDelta.y<0) scopeView.model.couderz-= 0.05; }
                }
                filters: [ CBScopeVirtualCouder { id: vcouder; scope: scopeView.model; } ]
            }
        }
        //********************************
        // COG page
        //********************************
        Rectangle {
            width: parent.width; height: parent.height;
            Column { spacing: 10
              width: parent.width; height: parent.height;
              Row { spacing: 10
                Text   { y: (parent.height-height)/2; text: qsTr("Cog (mirror front to cog"); }
                MyOText { y: (parent.height-height)/2; text: scopeView.model.cog }
              }
              Row { spacing: 10
                Text    { y: (parent.height-height)/2; text: qsTr("bottom Len"); }
                MyText  { y: (parent.height-height)/2; text: scopeView.model.cogBotLen; onTextChanged: scopeView.model.cogBotLen= text; }
                Text    { y: (parent.height-height)/2; text: qsTr("mid Len"); }
                MyOText { y: (parent.height-height)/2; text: scopeView.model.focal-scopeView.model.secondaryToFocal }
                Text    { y: (parent.height-height)/2; text: qsTr("top Len"); }
                MyText  { y: (parent.height-height)/2; text: scopeView.model.cogTopLen; onTextChanged: scopeView.model.cogTopLen= text; }
                Text    { y: (parent.height-height)/2; text: qsTr("weight/mm"); }
                MyText  { y: (parent.height-height)/2; text: scopeView.model.cogWeight; onTextChanged: scopeView.model.cogWeight= text; }
              }
              Row { spacing: 10
                height: parent.height-y; width: parent.width
                Column { spacing: 10
                  height: parent.height; width: parent.width/2
                  Row { spacing: 10
                    Text    { y: (parent.height-height)/2; text: qsTr("Total bottom weight:"); }
                    MyOText { y: (parent.height-height)/2; text: scopeView.model.cogBottomWeights }
                    Button  { y: (parent.height-height)/2; text: qsTr("add"); onPressed: cogListViewBottom.currentIndex= scopeView.model.addBottomWeight(); }
                  }
                  ListView { id: cogListViewBottom
                      model: scopeView.model.bottomWeights
                      width: parent.width; height: parent.height-y;
                      delegate: Row { spacing: 10; Component.onCompleted: setFont(this)
                        Text   { y: (parent.height-height)/2; text: qsTr("text"); }
                        MyText { y: (parent.height-height)/2; text: comment; onTextChanged: comment= text; }
                        Text   { y: (parent.height-height)/2; text: qsTr("weight"); }
                        MyText { y: (parent.height-height)/2; text: val; onTextChanged: val= text; fontcol: error ? "Red" : "Black"; }
                        Button { y: (parent.height-height)/2; text: qsTr("delete"); onPressed: scopeView.model.bottomWeights.remove(index); }
                      }
                  }
                }
                Column { spacing: 10
                  height: parent.height; width: parent.width/2
                  Row { spacing: 10
                    Text    { y: (parent.height-height)/2; text: qsTr("Total top weight:"); }
                    MyOText { y: (parent.height-height)/2; text: scopeView.model.cogTopWeights }
                    Button  { y: (parent.height-height)/2; text: qsTr("add"); onPressed: cogListViewTop.currentIndex= scopeView.model.addTopWeight(); }
                  }
                  ListView { id: cogListViewTop
                      model: scopeView.model.topWeights
                      width: parent.width; height: parent.height-y;
                      delegate: Row { spacing: 10; Component.onCompleted: setFont(this)
                        Text   { y: (parent.height-height)/2; text: qsTr("text"); }
                        MyText { y: (parent.height-height)/2; text: comment; onTextChanged: comment= text; }
                        Text   { y: (parent.height-height)/2; text: qsTr("weight"); }
                        MyText { y: (parent.height-height)/2; text: val; onTextChanged: val= text; fontcol: error ? "Red" : "Black"; }
                        Button { y: (parent.height-height)/2; text: qsTr("delete"); onPressed: scopeView.model.topWeights.remove(index); }
                      }
                  }
                }
              }
            }
        }
        //********************************
        // comments page
        //********************************
        Column {
            width: window.width; height: parent.height;
            Text   { id: commentLabelCommentsPage; text: qsTr("Comments"); }
            MyMultiText { y: commentLabelCommentsPage.height; width: parent.width; text: scopeView.model.notes; onTextChanged: scopeView.model.notes= text; }
        }
    }

    //********************************
    // footer page
    //********************************
    footer: Row { id: footer
            TabBar {
                width: parent.width
                id: tabBar
                currentIndex: scopeView.currentIndex
                TabButton { text: qsTr("Scope") }
                TabButton { text: qsTr("EP") }
                TabButton { text: qsTr("Secondary") }
                TabButton { text: qsTr("Hogging") }
                TabButton { text: qsTr("Parabolizing") }
                TabButton { text: qsTr("Couder") }
                TabButton { text: qsTr("Support") }
                TabButton { text: qsTr("Webcam") }
                TabButton { text: qsTr("COG") }
                TabButton { text: qsTr("Notes") }
            }
    }
}
