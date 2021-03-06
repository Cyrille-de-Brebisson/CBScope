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
import CBScopeCouderOverlay 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 1000
    height: 800
    minimumWidth: 300
    minimumHeight: 500
    property int fontSize: CBSModel.windowsFont
    property bool fontBold: CBSModel.windowsFontBold
    color: "black"

    // recursive set font to all controls
    function setFont(parentElement) {
        for (var i = 0; i < parentElement.children.length; ++i)
        {
            if (!("noAdaptFontSize" in parentElement) && parentElement.children[i].font)
            {
                parentElement.children[i].font.bold = Qt.binding(function() { return window.fontBold })
                parentElement.children[i].font.pixelSize = Qt.binding(function() { return window.fontSize })
            }
            if (parentElement.children[i].font) parentElement.children[i].font.color= "white"
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
        Button { id: chrono
            property double startTime
            property double time: 0
            property bool  mode: false
            text: (mode ? qsTr("Stop ") : qsTr("Start "))+Math.floor(time/60000)+":"+Math.floor((time/1000)%60);
            onClicked: { mode= !mode; if (mode) { startTime= new Date().getTime(); time= 0; } }
            Timer { running: chrono.mode
                interval: 1000; repeat: true;
                onTriggered: chrono.time= CBSModel.shouldBeep(new Date().getTime()-chrono.startTime, beepAt.text);
            }
        }
        MyText { id: beepAt; caption: qsTr("beepAt"); }
    }

    //*******************************************************
    // The main control. A list of pages with all we need...
    StackLayout {
        id: scopeView
        property CBSModelScope model
        width: parent.width; height: parent.height;
        currentIndex: tabBar.currentIndex

        //********************************
        // Main page: scope definition!!!!
        //********************************
        Flickable {
            width: parent.width; height: parent.height; clip: true;
            contentHeight: mainPageCol.height
            Column { id: mainPageCol; spacing: 10; width: scopeView.width;
                Row { spacing: 10;
                    width: parent.width
                    MyText { caption: qsTr("scope Name"); text: scopeView.model.name; onTextChanged: scopeView.model.name= text }
                    Rectangle { color: "black"; width: parent.width-scopeDelete.width-x-parent.spacing; height: parent.height }
                    Button { id: scopeDelete
                        visible: CBSModel.scopes.count>1
                        text: qsTr("Delete")
                        onClicked: { CBSModel.scopes.remove(scopes.currentIndex); scopes.currentIndex= 0; }
                    }
                }
                Text { color: "white"; text: qsTr("Comments") }
                MyMultiText { width: parent.width; text: scopeView.model.comments; onTextChanged: scopeView.model.comments= text; }
                Flow { spacing: 10;  id: scopeDiametreEdit; width: parent.width
                    MyText  { caption: qsTr("Diametre"); text: scopeView.model.diametre; onTextChanged: scopeView.model.diametre= Number(text); }
                    MyText  { caption: qsTr("Thickness"); text: scopeView.model.thickness; onTextChanged: scopeView.model.thickness= Number(text); }
                    MyOText { caption: qsTr("max zoom"); text: (50/25.4*scopeView.model.diametre).toFixed(0); }
                    MyOText { caption: qsTr("limiting magnitude"); text: (8.8+(5*Math.log(scopeView.model.diametre/25.4)/Math.log(10))).toFixed(0); }
                    MyOText { caption: qsTr("Resolution (ArcSec)"); text: (4.56/(scopeView.model.diametre/25.4)).toFixed(2); }
                    MyOText { caption: qsTr("Nb Times Eye (5mm pupille)"); text: ((scopeView.model.diametre*scopeView.model.diametre-scopeView.model.secondary*scopeView.model.secondary)/25).toFixed(0); }
                }
                Flow { spacing: 10; width: parent.width
                    MyText { caption: qsTr("focal length"); text: scopeView.model.focal; onTextChanged: scopeView.model.focal= Number(text); }
                    MyText { caption: qsTr("focal ratio");  text: scopeView.model.focal/scopeView.model.diametre; onTextChanged: scopeView.model.focal= Number(text)*scopeView.model.diametre; }
                    MyText { caption: qsTr("Mirror ROC");   text: scopeView.model.focal*2; onTextChanged: scopeView.model.focal= Number(text)/2; }
                    MyOText { caption: qsTr("sagita"); text: scopeView.model.sagita.toFixed(1); }
                }
                Flow { spacing: 10; width: parent.width
                    MyText { caption: qsTr("secondary small diametre"); text: scopeView.model.secondary; onTextChanged: scopeView.model.secondary= Number(text); }
                    MyText { caption: qsTr("secondary to focal plane"); text: scopeView.model.secondaryToFocal; onTextChanged: scopeView.model.secondaryToFocal= Number(text); }
                    MyOText { caption: qsTr("secondary offset toward primary"); text: scopeView.model.secondaryOffset.toFixed(1); }
                    MyOText { caption: qsTr("Obstruction"); text: ((scopeView.model.secondary*scopeView.model.secondary)/(scopeView.model.diametre*scopeView.model.diametre)*100).toFixed(0)+"%"; }
                }
                Flow { spacing: 10; width: parent.width
                    MyText  { caption: qsTr("Density (g/cm^3)"); text: scopeView.model.density; onTextChanged: scopeView.model.density= Number(text); }
                    MyOText { caption: qsTr("Weight (Kg)"); text: scopeView.model.weight.toFixed(3); }
                    MyText  { caption: qsTr("Young (KgF/mm²)"); text: scopeView.model.young; onTextChanged: scopeView.model.young= Number(text); }
                    MyText  { caption: qsTr("Poisson"); text: scopeView.model.poisson; onTextChanged: scopeView.model.poisson= Number(text); }
                    ComboBox { model: ListModel { ListElement { name: "select" }
                                                  ListElement { name: "Pyrex 7740" }
                                                  ListElement { name: "Zerodur" }
                                                  ListElement { name: "Plate Glass" }
                                                  ListElement { name: "Fused scilica" }
                                                  ListElement { name: "Duran 50" } }
                                textRole: "name";
                                onCurrentIndexChanged: { scopeView.model.density= CBSModel.materials(currentIndex-1, 2); scopeView.model.poisson= CBSModel.materials(currentIndex-1, 1); scopeView.model.young= CBSModel.materials(currentIndex-1, 0); }
                    }
                }
                Flow { spacing: 10; width: parent.width
                    MyText  { caption: qsTr("top Len"); text: scopeView.model.cogTopLen; onTextChanged: scopeView.model.cogTopLen= Number(text); }
                    MyText  { caption: qsTr("View angle"); text: scopeView.model.viewAngle; onTextChanged: scopeView.model.viewAngle= Number(text); }
                    MyOText { caption: qsTr("radius top"); text: (Math.tan(scopeView.model.viewAngle/360*Math.PI)*(scopeView.model.focal-scopeView.model.secondaryToFocal+scopeView.model.cogTopLen)+scopeView.model.diametre/2).toFixed(0); }
                    MyText  { caption: qsTr("Focusser height"); text: scopeView.model.focusserHeight; onTextChanged: scopeView.model.focusserHeight= text; }
                    MyOText { caption: qsTr("Focusser play"); text: (scopeView.model.secondaryToFocal-scopeView.model.focusserHeight-(Math.tan(scopeView.model.viewAngle/360*Math.PI)*(scopeView.model.focal-scopeView.model.secondaryToFocal)+scopeView.model.diametre/2)).toFixed(0); }
                }

                Rectangle {
                    width: parent.width;
                    height: (scopeView.height-y) > (scopeTextArea.height+scopeBottomRow.height+2*parent.spacing) ? (scopeView.height-y) - (scopeTextArea.height+scopeBottomRow.height+2*parent.spacing) : 1;
                    color: height!==1 ? "Black" : "white"
                }
                TextArea { id: scopeTextArea; width: parent.width;
                    color: "white"
                    text: qsTr("CBScope is an application designed by Cyrille.de.brebisson@gmail.com to help amateur telescope makers build telescopes.\n"+
                               "Parts of the application code comes from other similar apps and I wich to thank: Etienne de Foras (Foucault), David Lewis (Plop) and Mel Bartels (Secondary illumination and Ronchi).\n"+
                               "Click on the Help button bellow for help on how to use this app");
                    wrapMode: Text.Wrap; readOnly: true;
                }
                Flow { id: scopeBottomRow; width: parent.width; spacing: 10;
                    Button { text: "help"; onClicked: CBSModel.help(); }
                    Button { text: qsTr("Email scope"); onClicked: scopeView.model.email(); }
                    Button { text: qsTr("Receive scope"); onClicked: receiveScopePopup.open(); }
                    ComboBox {
                        textRole: "text";
                        model: ListModel { ListElement { text: "font 10"; } ListElement { text: "font 11"; } ListElement { text: "font 12"; } ListElement { text: "font 13"; } ListElement { text: "font 14"; } ListElement { text: "font 15"; } ListElement { text: "font 16"; } }
                        currentIndex: window.fontSize-10
                        onCurrentIndexChanged: CBSModel.windowsFont= window.fontSize= currentIndex+10;
                    }
                    MyCheckBox { id: headerCb; text: qsTr("Bold"); checked: window.fontBold; onCheckedChanged: CBSModel.windowsFontBold= window.fontBold= checked; }
                }
                Popup { id: receiveScopePopup; x: 50; y: 50; width: parent.width-2*x; height: parent.height-2*y; modal: true; focus: true; clip: true;
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
                        ScrollView { // Scrollable text area...
                            width: parent.width; height: receiveScopePopupBottom.y-10
                            TextArea { id: popupText; text: qsTr("Paste scope definition here, then press Load!"); }
                        }
                        Row { spacing: 10; id: receiveScopePopupBottom; x: parent.width-width; y: parent.height-height-10
                            Button { text: qsTr("Load!"); onPressed: { scopes.currentIndex= CBSModel.loadScope(popupText.text); receiveScopePopup.close(); } }
                            Button { text: qsTr("cancel"); onPressed: receiveScopePopup.close(); }
                        }
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
                color: "black"
                ListView {
                    id: epListView
                    clip: true
                    model: scopeView.model.eps
                    width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width
                    x: parent.border.width; y: parent.border.width
                    delegate: Rectangle {
                        Component.onCompleted: setFont(this)
                        width: parent.width
                        border.width: 3; radius: 2; border.color: "white"
                        height: col2.height+3*border.width
                        color: "black"
                        Column {
                            id: col2
                            clip: true
                            x: parent.border.width*1.5; y: parent.border.width*1.5
                            width: parent.width-3*parent.border.width
                            Row {
                                spacing: 10; width: parent.width;
                                MyText { caption: qsTr("name"); text: name; onTextChanged: name= text; }
                                Rectangle { color: "black"; height: parent.height; width: parent.width-x-rowEp5.width-parent.spacing; }
                                Button { id: rowEp5; text: qsTr("Delete"); onPressed: scopeView.model.epRemove(index); }
                            }
                            Row { spacing: 10; width: parent.width;
                                MyText { caption: qsTr("focal"); text: focal; onTextChanged: focal= Number(text); }
                                MyText { caption: qsTr("angle"); text: angle; onTextChanged: angle= Number(text); }
                            }
                            Flow { spacing: 10; width: parent.width;
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
            Flow { spacing: 10; width: parent.width;
                MyText { caption: qsTr("possible Secondaries"); text: scopeView.model.secondariesToConcider; onTextChanged: scopeView.model.secondariesToConcider= text; }
                MyCheckBox { text: qsTr("2\" focusser");  checked: scopeIlumination.twoInches; onCheckedChanged: scopeIlumination.twoInches= checked;  }
                MyText { caption: qsTr("secondary to focal plane"); text: scopeView.model.secondaryToFocal; onTextChanged: scopeView.model.secondaryToFocal= Number(text); }
            }
            CBScopeIlumination { id: scopeIlumination
                width: parent.width; height: parent.height-y;
                scope: scopeView.model
                onTwoInchesChanged: update();
				Connections {
					target: scopeView.model
					onDiametreChanged:				scopeIlumination.update();
					onFocalChanged:					scopeIlumination.update();
					onSecondaryToFocalChanged:		scopeIlumination.update();
					onSecondariesToConciderChanged: scopeIlumination.update();
					onEpsChanged:					scopeIlumination.update();
                    onSecondaryChanged:             scopeIlumination.update();
				}
            }
        }

        //********************************
        // Hogging page
        //********************************
        Column {
            width: window.width; height: parent.height; spacing: 10; id: hogPage
            MyText { caption: qsTr("Spherometer leg distance"); text: scopeView.model.spherometerLegDistances; onTextChanged: scopeView.model.spherometerLegDistances= text; }
            Rectangle {
                width: parent.width;
                height: parent.height-y-bottomInfo.height-10
                border.width: 3; radius: 4; border.color: "Blue";
                color: "black"
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
                            border.width: 3; radius: 4; border.color: "white";
                            height: col.height+2*border.width
                            color: "black"
                            Column {
                                id: col
                                clip: true
                                width: parent.width-2*parent.border.width; x: parent.border.width; y: parent.border.width
                                Row {
                                    spacing: 10; width: parent.width; //height: rowHog2.height
                                    Text   { color: "white"; text: qsTr("comments"); }
                                    MyMultiText { text: comments; onTextChanged: comments= text; }
                                    Rectangle { color: "black"; height: 10; width: parent.width-x-deleteHog.width-parent.spacing; }
                                    Button { id: deleteHog; text: qsTr("Delete"); onPressed: scopeView.model.hoggingsRemove(index); }
                                }
                                Flow { spacing: 10; width: parent.width
                                    MyText { caption: qsTr("start ROC"); text: startSphere; onTextChanged: startSphere= Number(text); }
                                    MyText { caption: qsTr("end ROC"); text: endSphere; onTextChanged: endSphere= Number(text); }
                                    MyText { caption: qsTr("end ROC spherometer"); onTextChanged: { if (text!=="") scopeView.model.hoggings.get(index).setEndSphereSpherometer(Number(text)); } }
                                }
                                Flow { spacing: 10; width: parent.width
                                    MyText { caption: qsTr("time"); text: time; onTextChanged: time= Number(text); }
                                    MyText { caption: qsTr("grit"); text: grit; onTextChanged: grit= Number(text); }
                                }
                                Flow { spacing: 10; width: parent.width
                                    MyOText { caption: qsTr("hog/time");   text: hogSpeed.toFixed(1); }
                                    MyOText { caption: qsTr("end sagita"); text: endSagita.toFixed(2); }
                                    MyOText { caption: qsTr("% done in this run"); text: percentDone.toFixed(2); }
                                    MyOText { caption: qsTr("hogged"); text: hogged.toFixed(0); }
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
                color: "black"
                Column {
                    id: bottomInfoCol
                    clip: true
                    width: parent.width-2*parent.border.width; x: parent.border.width; y: parent.border.width
                    Flow { spacing: 10; width: parent.width; 
                        MyOText { caption: qsTr("time to hog with current grit");   text: scopeView.model.hogTimeWithGrit.toFixed(2); }
                        MyOText { caption: qsTr("%done"); text: ((1-scopeView.model.leftToHog/scopeView.model.toHog)*100).toFixed(2); }
                    }
                    Flow { spacing: 10; width: parent.width; 
                        MyOText { caption: qsTr("total to hog");   text: scopeView.model.toHog.toFixed(2); }
                        MyOText { caption: qsTr("left to hog"); text: scopeView.model.leftToHog.toFixed(2); }
                        MyOText { caption: qsTr("Total spent time"); text: scopeView.model.totalHogTime; }
                    }
                }
            }
        }

        //********************************
        // parabolizing page
        //********************************
        Column { spacing: 5; id: parabolizingColumn
            property int mesureHeight: 180
            Flow { width: parent.width
                spacing: 10
                MyText   { id: nbZonesSuggest; caption: qsTr("nb zones (Suggest ")+scopeView.model.nbZonesSuggested+")"; text: scopeView.model.nbZones; onTextChanged: scopeView.model.nbZones= Number(text); }
                Button   { y: (parent.height-height)/2; text: qsTr("Auto calc zones"); onPressed: popup.open() }
                MyCheckBox { text: qsTr("Slit is moving"); y: (parent.height-height)/2; checked: scopeView.model.slitIsMoving; onCheckedChanged: scopeView.model.slitIsMoving= checked }
                MyCheckBox { text: qsTr("large View"); y: (parent.height-height)/2; checked: false; onCheckedChanged: { parabolizingColumn.mesureHeight= checked?250:180; } }
            }
            Text { color: "white"; text: qsTr("Enter zones boundaries from 0 (or hole radius) to mirror radius") }
            Rectangle {
                border.width: 3; radius: 4; border.color: "Blue";
                width: parent.width
                height: nbZonesSuggest.height+2*border.width
                color: "black";
                ListView {
                    model: scopeView.model.zones; orientation: Qt.Horizontal;
                    width: parent.width-2*parent.border.width; 
					height: parent.height-2*parent.border.width
					x: parent.border.width; y: parent.border.width;
                    delegate: MyText {
                        Component.onCompleted: setFont(this)
                        text: val; onTextChanged: val= Number(text);
                    }
                }
            }
            Rectangle {
                width: parent.width
                height: parent.height-y
                border.width: 3; radius: 4; border.color: "White";
                color: "black";
                Row { x: (parent.width-width)/2; spacing: 10;
                    Button { id: addMesBut; y: parent.border.width; text: qsTr("add measure"); onPressed: scopeView.model.addParabolizing() }
                    Button { text: qsTr("add foucaultless"); onPressed: scopeView.model.addParabolizing2() }
                    MyText { id: parabtype1r1; caption: "pos";     visible: parabolizingWorks.model.get(parabolizingWorks.currentIndex).type===1; onEnter: parabolizingWorks.model.get(parabolizingWorks.currentIndex).addReading(Number(parabtype1r1.text), Number(parabtype1r2.text)); }
                    MyText { id: parabtype1r2; caption: "reading"; visible: parabolizingWorks.model.get(parabolizingWorks.currentIndex).type===1; onEnter: parabolizingWorks.model.get(parabolizingWorks.currentIndex).addReading(Number(parabtype1r1.text), Number(parabtype1r2.text)); }
                }
                ListView { id: parabolizingWorks
                    clip: true;
                    model: scopeView.model.parabolizings;
                    x: parent.border.width; y: parent.border.width+addMesBut.height;
                    width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width-addMesBut.height;
                    delegate: Rectangle {
                        Component.onCompleted: setFont(this)
                        border.width: 3; radius: 4; border.color: "Blue";
                        color: "black";
                        width: parent.width; height: parabolizingColumn.mesureHeight
                        Column {
                            clip: true;
                            x: parent.border.width; y: parent.border.width; width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width;
                            ListView { 
                                model: mesures; orientation: Qt.Horizontal;
                                width: parent.width; height: nbZonesSuggest.height
                                delegate: MyText { text: val; onTextChanged: val= text; fontcol: error ? "Red" : "White"; }
                            }
                            Row {
                                spacing: 10; width: parent.width
                                MyText { caption: qsTr("time"); text: time; onTextChanged: time= text; }
                                Text   { color: "white"; y: (parent.height-height)/2; text: qsTr("Comments"); }
                                MyMultiText { y: (parent.height-height)/2; text: comments; onTextChanged: comments= text; }
                                MyCheckBox { text: qsTr("fixTo"); checked: scopeView.model.fixedFocal===index; onClicked: scopeView.model.fixedFocal= (checked ? index : -1); }
                                Rectangle { color: "black"; height: parent.height; width: parent.width-x-rowMes5.width-parent.spacing; }
                                Button { id: rowMes5; y: (parent.height-height)/2; text: qsTr("Delete"); onPressed: scopeView.model.parabolizings.remove(index); }
                            }
                            CBScopeMesure {
                                id: couderDisplay
                                mesure: scopeView.model.parabolizings.get(index);
                                width: parent.width; height: parent.height-y;
                            }
                            Connections { target: couderDisplay.mesure; onMesuresChanged: couderDisplay.update(); } // redraw when mesure change...
                        }
						MouseArea {
							anchors.fill: parent
							acceptedButtons: Qt.LeftButton | Qt.RightButton
							onClicked: { mouse.accepted= false; }
                            propagateComposedEvents: true
                            onPressed:         { mouse.accepted = false; console.log("change focus", index); parabolizingWorks.currentIndex= index; }
                            onReleased:        { mouse.accepted = false; }
                            onDoubleClicked:   { mouse.accepted = false; }
                            onPressAndHold:    { mouse.accepted = false; }
                            onPositionChanged: { mouse.accepted = false; }
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
            Flow {
                spacing: 10; y: 1; width: parent.width; 
                CheckBox { y: (parent.height-height)/2; text: qsTr("Show Red Zones"); checked: scopeView.model.showCouderRed; onCheckedChanged: scopeView.model.showCouderRed= checked }
                CheckBox { y: (parent.height-height)/2; text: qsTr("Show Blue Zones"); checked: scopeView.model.showCouderBlue; onCheckedChanged: scopeView.model.showCouderBlue= checked }
                CheckBox { y: (parent.height-height)/2; text: qsTr("zoom/2"); checked: scopeCouder.zoom; onCheckedChanged: scopeCouder.zoom= checked }
                CheckBox { y: (parent.height-height)/2; text: qsTr("Rotate"); checked: scopeView.model.couderRotate; onCheckedChanged: scopeView.model.couderRotate= checked }
                Button   { visible: CBSModel.canPrint; y: (parent.height-height)/2; text: qsTr("Print Couder Mask"); onPressed: scopeView.model.printCouder() }
            }
            Connections {
                target: scopeView.model
                onDiametreChanged: scopeCouder.update();
                onNbZonesChanged: scopeCouder.update();
                onShowCouderRedChanged: scopeCouder.update()
                onShowCouderBlueChanged: scopeCouder.update()
                onShowCouderOrangeChanged: scopeCouder.update()
                onCouderRotateChanged: scopeCouder.update()
            }
        }

        //********************************
        // webcam page
        //********************************
        DropArea { 
			onEntered: drag.accept (Qt.CopyAction); onDropped: 
			{	
				cameras.currentIndex= -1;
				userImagecouder.source= drop.text; 
			}
            width: window.width; height: parent.height;
			Column {
				width: window.width; height: parent.height;
				Flow {
					spacing: 10; width: parent.width
                    Text { color: "white"; y: (parent.height-height)/2; text: qsTr("Camera") }
					ComboBox { id: cameras; y: (parent.height-height)/2;
							   model: QtMultimedia.availableCameras
							   textRole: "displayName";
							   onCurrentIndexChanged: {
									console.log("curent index ", currentIndex);
									if (currentIndex!=-1) { camera.deviceId= QtMultimedia.availableCameras[currentIndex].deviceId; camera.start(); userImagecouder.source= ""; }
									else { camera.stop(); camera.deviceId= ""; }
							   }
							   currentIndex: -1
                    }
					ComboBox { visible: !ronchi.checked; id: zoneToUse; y: (parent.height-height)/2;
							   model: scopeView.model.zoneModel
							   textRole: "val";
							   currentIndex: scopeView.model.zone
							   onCurrentIndexChanged: scopeView.model.zone= currentIndex
                    }
					ComboBox { visible: !ronchi.checked; y: (parent.height-height)/2; 
							   model: ListModel { ListElement { name: qsTr("Grayscale") } 
												  ListElement { name: qsTr("Red+Blue+Green") } 
												  ListElement { name: qsTr("Blue") } 
												  ListElement { name: qsTr("Green") } 
												  ListElement { name: qsTr("Red") }
                                                  }
								textRole: "name";
								currentIndex: scopeView.model.virtualCouderType
								onCurrentIndexChanged: scopeView.model.virtualCouderType= currentIndex; 
					 }
                     MyCheckBox { id: hideMask; text: qsTr("mask"); y: (parent.height-height)/2; }
                     MyCheckBox { id: hide_rest; text: qsTr("zones"); y: (parent.height-height)/2; }
                     MyCheckBox { text: qsTr("Pause"); y: (parent.height-height)/2; checked: scopeView.model.pause; onCheckedChanged: scopeView.model.pause= checked }
                     MyCheckBox { text: qsTr("Ronchi"); id: ronchi; y: (parent.height-height)/2; checked: scopeView.model.ronchi; onCheckedChanged: scopeView.model.ronchi= checked }
                     // The next 2 ones are only visible in Ronchi mode...
					 MyText { caption: qsTr("defocal"); visible: ronchi.checked; text: scopeView.model.ronchiOffset; onTextChanged: scopeView.model.ronchiOffset= Number(text); }
					 MyText { caption: qsTr("grading"); visible: ronchi.checked; text: scopeView.model.grading; onTextChanged: scopeView.model.grading= Number(text); }
				}
				Rectangle { visible: !ronchi.checked;
					width: parent.width; height: 180; 
                    color: "black";
                    id: webcamedParabolizing
					property CBSModelParabolizingWork mesure: scopeView.model.parabolizings.get(scopeView.model.parabolizings.count-1)
					border.width: 3; radius: 4; border.color: "Blue";
					Column {
						clip: true;
						x: parent.border.width; y: parent.border.width; width: parent.width-2*parent.border.width; height: parent.height-2*parent.border.width;
						ListView { id: currentMesureWebcam
							model: parent.parent.mesure.mesures; orientation: Qt.Horizontal;
							width: parent.width; height: nbZonesSuggest.height
                            delegate: MyText { text: val; onTextChanged: val= text; fontcol: error ? "Red" : "white"; }
						}
						Row {
							spacing: 10; width: parent.width
                            Text   { color: "white"; y: (parent.height-height)/2; text: qsTr("time"); }
							MyText { y: (parent.height-height)/2; text: parent.parent.parent.mesure.time; onTextChanged: parent.parent.parent.mesure.time= text; }
                            Text   { color: "white"; y: (parent.height-height)/2; text: qsTr("Comments"); }
							MyText { y: (parent.height-height)/2; text: parent.parent.parent.mesure.comments; onTextChanged: parent.parent.parent.mesure.comments= text; }
						}
            CBScopeMesure { // couder mesure display in couder mode (not Ronchi)
							id: couderDisplay2
							mesure: parent.parent.mesure
							width: parent.width; height: parent.height-y;
						}
						Connections { target: couderDisplay2.mesure; onMesuresChanged: couderDisplay2.update(); } // redraw when mesure change...
					}
				}    
                Camera {
                    id: camera; deviceId: "nocamera";
                    Component.onCompleted: camera.stop();
                    exposure {
                        exposureCompensation: -1.0
                        manualIso: 1200
                        manualShutterSpeed: 0.1
                    }
                    imageProcessing {
                        brightness: 0
                        contrast: 0
                        saturation: 0
                    }
                }
				Rectangle { id: virtualCouderControl
					width: parent.width; height: parent.height-y;
                    color: "black";
                    function viewer()
					{
						if (cameras.currentIndex!=-1) return vcouder;
						if (userImagecouder.source!="") return userImagecouder;
						return undefined;
					}
                    CBScopeCouderOverlay { // This is used when the user drags/drop an image here....
						visible: source!="" 
						id: userImagecouder; scope: scopeView.model;
						width: videoOutput.width; height: videoOutput.height;
                        x: videoOutput.x
						MouseArea {
							anchors.fill: parent
							acceptedButtons: Qt.LeftButton | Qt.RightButton
							onClicked: { var p= parent.mapPointToSourceNormalized(Qt.point(mouseX, mouseY)); parent.parent.viewer().userclick(p.x, p.y); }
							onWheel: { wheel.accepted= true; if (wheel.angleDelta.y>0) scopeView.model.couderz+= 0.05; if (wheel.angleDelta.y<0) scopeView.model.couderz-= 0.05; }
						}
					}
                    VideoOutput { id: videoOutput // This is used for live video display
						width: parent.width*4/5; height: parent.height;
                        x: parent.width/5;
						source: camera
						visible: cameras.currentIndex!=-1
						filters: [ CBScopeVirtualCouder { enabled: !hideMask.checked; hide_rest: hide_rest.checked; id: vcouder; scope: scopeView.model; 
                                              onLastRadiusChanged: { console.log("last radius changed"); couderDisplay2.setIndicatedRadius(lastRadius); couderDisplay2.update(); } } ]
						MouseArea {
							anchors.fill: parent
							acceptedButtons: Qt.LeftButton | Qt.RightButton
							onClicked: { var p= parent.mapPointToSourceNormalized(Qt.point(mouseX, mouseY)); parent.parent.viewer().userclick(p.x, p.y); }
							onWheel: { wheel.accepted= true; if (wheel.angleDelta.y>0) scopeView.model.couderz+= 0.05; if (wheel.angleDelta.y<0) scopeView.model.couderz-= 0.05; }
						}
					}
                    Column { width: parent.width/5;
                        MyJoystick {
                            x:0; width:100; height:80; caption: qsTr("mirror");
                            onXposChanged: scopeView.model.couderx= xpos;
                            onYposChanged: scopeView.model.coudery= ypos;
                            onZposChanged: scopeView.model.couderz= zpos;
                            xpos: scopeView.model.couderx;
                            ypos: scopeView.model.coudery;
                            zpos: scopeView.model.couderz;
                        }
                        MyJoystick {
                            x:0; width:100; height:80; caption: qsTr("image");
                            onXposChanged: scopeView.model.imgcouderx= xpos;
                            onYposChanged: scopeView.model.imgcoudery= ypos;
                            onZposChanged: scopeView.model.imgcouderz= zpos;
                            xpos: scopeView.model.imgcouderx;
                            ypos: scopeView.model.imgcoudery;
                            zpos: scopeView.model.imgcouderz;
                        }
                        Button { text: qsTr("Save"); onPressed: scopeView.model.takePicture(); visible: videoOutput.visible; }
                        MyCheckBox { id: camSet; text: qsTr("Camera Settings"); }
                        Row { visible: camSet.checked
                            Text { color: "white"; text: "brightness" }
                            Slider { from: -1; to: 1; value: camera.imageProcessing.brightness; onValueChanged: camera.imageProcessing.brightness= value; }
                        }
                        Row { visible: camSet.checked
                            Text { color: "white"; text: "contrast" }
                            Slider { from: -1; to: 1; value: camera.imageProcessing.contrast; onValueChanged: camera.imageProcessing.contrast= value; }
                        }
                        Row { visible: camSet.checked
                            Text { color: "white"; text: "saturation" }
                            Slider { from: -1; to: 1; value: camera.imageProcessing.saturation; onValueChanged: camera.imageProcessing.saturation= value; }
                        }
                        Row { visible: camSet.checked
                            Text { color: "white"; text: "exposureCompensation" }
                            Slider { from: -1; to: 1; value: camera.exposure.exposureCompensation; onValueChanged: camera.exposure.exposureCompensation= value; }
                        }
                        Row { visible: camSet.checked
                            Text { color: "white"; text: "manualShutterSpeed" }
                            Slider { from: 0.01; to: 1; value: camera.exposure.manualShutterSpeed; onValueChanged: camera.exposure.manualShutterSpeed= value; }
                        }
                        Row { visible: camSet.checked
                            Text { color: "white"; text: "manualIso" }
                            Slider { from: 100; to: 1600; stepSize: 100; value: camera.exposure.manualIso; onValueChanged: camera.exposure.manualIso= value; }
                        }
                        MyCheckBox { id: table; text: qsTr("table Settings"); }
                        Flow { visible: table.checked; spacing: 10; width: parent.width;
                            MyText { width: 70; caption: qsTr("X"); text: CBSModel.tableX; onEnter: CBSModel.tableX= Number(text); }
                            MyText { width: 70; caption: qsTr("Y"); text: CBSModel.tableY; onEnter: CBSModel.tableY= Number(text); }
                        }
                        Flow { visible: table.checked; spacing: 10; width: parent.width;
                            MyText { caption: qsTr("steps/mm X"); text: CBSModel.tableXSteps; onTextChanged: CBSModel.tableXSteps= Number(text); }
                            MyText { caption: qsTr("Y"); text: CBSModel.tableYSteps; onTextChanged: CBSModel.tableYSteps= Number(text); }
                        }
                        Flow { visible: table.checked; spacing: 10; width: parent.width;
                            MyText { caption: qsTr("slack(mm) X"); text: CBSModel.tableXSlack; onTextChanged: CBSModel.tableXSlack= Number(text); }
                            MyText { caption: qsTr("Y");           text: CBSModel.tableYSlack; onTextChanged: CBSModel.tableYSlack= Number(text); }
                        }
                        Flow { visible: table.checked; spacing: 10; width: parent.width;
                            MyText { width: 100; id: xytableBtnGoX2; caption: qsTr("go X"); text: CBSModel.tableX; onEnter: CBSModel.goTable(Number(xytableBtnGoX2.text), Number(xytableBtnGoY2.text)); }
                            MyText { width: 70; id: xytableBtnGoY2; caption: qsTr("Y"); text: CBSModel.tableY; onEnter: CBSModel.goTable(Number(xytableBtnGoX2.text), Number(xytableBtnGoY2.text)); }
                            Button { width: 50; text: qsTr("Go"); onPressed: CBSModel.goTable(Number(xytableBtnGoX2.text), Number(xytableBtnGoY2.text)); }
                        }
                        Flow { visible: table.checked; spacing: 10; width: parent.width;
                            Text { text: "LED1"; color: "White" }
                            Slider { from: 0; to: 255; value: CBSModel.tableLed1; onValueChanged: CBSModel.tableLed1= value; stepSize: 3; }
                        }
                        Flow { visible: table.checked; spacing: 10; width: parent.width;
                            Text { text: "LED2"; color: "White" }
                            Slider { from: 0; to: 255; value: CBSModel.tableLed2; onValueChanged: CBSModel.tableLed2= value; stepSize: 3; }
                        }
                        ComboBox { visible: table.checked; textRole: "val"; model: CBSModel.coms; currentIndex: 0; onCurrentIndexChanged: CBSModel.setCom(currentIndex); }
                        Flow { visible: table.checked; spacing: 10; width: parent.width;
                            function updatePos() { CBSModel.goTable2(xybx12.pressed?100:(xybx22.pressed?-100:0), xyby12.pressed?100:(xyby22.pressed?-100:0)); }
                            Button { width: 60; id: xybx12; text: "+X"; onPressed: parent.updatePos(); onReleased: parent.updatePos(); } 
                            Button { width: 60; id: xybx22; text: "-X"; onPressed: parent.updatePos(); onReleased: parent.updatePos(); } 
                            Button { width: 60; id: xyby12; text: "+Y"; onPressed: parent.updatePos(); onReleased: parent.updatePos(); } 
                            Button { width: 60; id: xyby22; text: "-Y"; onPressed: parent.updatePos(); onReleased: parent.updatePos(); } 
                            MyText { caption: qsTr("mm/s");  text: CBSModel.tableSpd; onTextChanged: CBSModel.tableSpd= Number(text); }
                            Button { text: "mesureX"; onPressed: currentMesureWebcam.model.get(scopeView.model.zone).val= -CBSModel.tableX; } 
                            Button { text: "new measure"; onPressed: webcamedParabolizing.mesure.addReading(vcouder.getLastRadius(), -CBSModel.tableX); } 
                        }
                    }
                }
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
			Column { spacing: 5; y:1; x:1; width: parent.width
				Flow { spacing: 10; width: parent.width
					CheckBox { y: (parent.height-height)/2; text: qsTr("Show Disp"); checked: scopeSupport.showForces; onCheckedChanged: scopeSupport.showForces= checked }
					CheckBox { y: (parent.height-height)/2; text: qsTr("Show Mesh"); checked: scopeSupport.showMesh; onCheckedChanged: scopeSupport.showMesh= checked }
					CheckBox { y: (parent.height-height)/2; text: qsTr("Show Parts"); checked: scopeSupport.showParts; onCheckedChanged: scopeSupport.showParts= checked }
					CheckBox { y: (parent.height-height)/2; text: qsTr("Show Supports"); checked: scopeSupport.showSupports; onCheckedChanged: scopeSupport.showSupports= checked }
					CheckBox { y: (parent.height-height)/2; text: qsTr("Show secondary"); checked: scopeSupport.showSecondary; onCheckedChanged: scopeSupport.showSecondary= checked }
					}
				Flow { spacing: 10; width: parent.width
					ComboBox { y: (parent.height-height)/2; 
							   model: ListModel { ListElement { name: "3 points" } 
												  ListElement { name: "6 points" } 
												  ListElement { name: "9 points" } 
												  ListElement { name: "9 points fixed angles" } 
												  ListElement { name: "12 points" }
												  ListElement { name: "18 points" }
												  ListElement { name: "18 points fixed angles" }
												  ListElement { name: "27 points" } 
												  ListElement { name: "27 points fixed angles" } 
                                                  ListElement { name: "36 points" } 
                                                  ListElement { name: "36 points fixed angles" } 
                                                  ListElement { name: "54 points" }
                                                  ListElement { name: "54 points fixed angles" } } 
								textRole: "name";
								currentIndex: scopeView.model.cellType 
								onCurrentIndexChanged: scopeView.model.cellType= currentIndex;
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
								onCurrentIndexChanged: scopeSupport.zoom= currentIndex;
					 }
                    Button { text: (!scopeSupport.calc ? (scopeSupport.matrixProgresses===0 ? "Calc" : "Calc "+ scopeSupport.matrixProgresses): "stop "+(scopeSupport.matrixProgresses*100).toFixed(0)+"%");
							 onClicked: if (scopeSupport.calc) scopeSupport.doMesStop(); else scopeSupport.doMesSolve(); }
				}
				MyOText { caption: "P-V err"; text: (scopeSupport.errPV*1e6).toFixed(2)+"nm lam/"+(555e-6/scopeSupport.errPV).toFixed(0) }
				MyOText { caption: "RMS err"; text: (scopeSupport.errRms*1e6).toFixed(2)+"nm lam/"+(555e-6/scopeSupport.errRms).toFixed(0) }
			}
			MyOText { y: parent.height-height; caption: "parts"; text: scopeSupport.parts }
			Button  { visible: CBSModel.canPrint; y: 0; x: parent.width-width; text: qsTr("Print"); onPressed: scopeSupport.printPLOP() }

        }

        //********************************
        // COG page
        //********************************
        Column { spacing: 10; width: parent.width; height: parent.height;
			MyOText { caption: qsTr("Cog (mirror front to cog)"); text: scopeView.model.cog.toFixed(0) }
			Flow { spacing: 10; width: parent.width; 
				MyText  { caption: qsTr("bottom Len"); text: scopeView.model.cogBotLen; onTextChanged: scopeView.model.cogBotLen= text; }
				MyOText { caption: qsTr("mid Len"); text: scopeView.model.focal-scopeView.model.secondaryToFocal }
				MyText  { caption: qsTr("top Len"); text: scopeView.model.cogTopLen; onTextChanged: scopeView.model.cogTopLen= text; }
				MyText  { caption: qsTr("weight/mm"); text: scopeView.model.cogWeight; onTextChanged: scopeView.model.cogWeight= text; }
			}
			Flickable { id: cogLists; width: parent.width; height: parent.height-y-parent.spacing;
				property int maxw: 0
			    function horizontalLayout() { return maxw<=(width-10)/2 ? 2 : 1; }
			    contentHeight: 2*height/horizontalLayout();
				contentWidth: width

				ListView { id: bottomWeights
					width: parent.width/cogLists.horizontalLayout(); height: Math.min(parent.height*0.8, contentHeight);
					x: 0; y: 0;
					header: Row { spacing: 10;
								MyOText { caption: qsTr("Total bottom weight:"); text: scopeView.model.cogBottomWeights }
								Button  { text: qsTr("add"); onPressed: currentIndex= scopeView.model.addBottomWeight(); }
							}
					model: scopeView.model.bottomWeights; 
					delegate: Row { spacing: 10; Component.onCompleted: setFont(this); 
						onWidthChanged: cogLists.maxw= Math.max(cogLists.maxw, width);
						MyText { caption: qsTr("text"); text: comment; onTextChanged: comment= text; }
                        MyText { caption: qsTr("weight"); text: val; onTextChanged: val= text; fontcol: error ? "Red" : "white"; }
						Button { text: qsTr("delete"); onPressed: scopeView.model.bottomWeightsRemove(index); }
					}
				}
				ListView {  id: topWeights
					width: parent.width/cogLists.horizontalLayout(); height: Math.min(parent.height*0.8, contentHeight);
					x: cogLists.horizontalLayout()==1 ? 0 : width; y: cogLists.horizontalLayout()==1 ? height+10 : 0;
					header: Row { spacing: 10; 
								MyOText { caption: qsTr("Total top weight:"); text: scopeView.model.cogTopWeights }
								Button  { text: qsTr("add"); onPressed: currentIndex= scopeView.model.addTopWeight(); }
							}
					model: scopeView.model.topWeights; 
					delegate: Row { spacing: 10; Component.onCompleted: setFont(this); 
						onWidthChanged: cogLists.maxw= Math.max(cogLists.maxw, width);
						MyText { caption: qsTr("text"); text: comment; onTextChanged: comment= text; }
                        MyText { caption: qsTr("weight"); text: val; onTextChanged: val= text; fontcol: error ? "Red" : "white"; }
                        Button { text: qsTr("delete"); onPressed: scopeView.model.topWeightsRemove(index); }
					}
				}
			}
        }

        //********************************
        // comments page
        //********************************
		Rectangle {
	        width: parent.width; height: parent.height;
            border.width: 3; border.color: notesText.focus ? "#569ffd" : "lightgrey"; radius: 4;
            ScrollView { id: notesText // Scrollable text area...
				x: parent.border.width; y: parent.border.width; width: parent.width-parent.border.width*2; height: parent.height-parent.border.width*2;
				TextArea { 
					wrapMode: Text.Wrap; clip: true
					text: scopeView.model.notes;
					onTextChanged: scopeView.model.notes= text;
				}
			}
		}

        //********************************
        // X/Y table page
        //********************************
        Column { spacing: 10; width: parent.width; height: parent.height;
            Flow { spacing: 10; width: parent.width;
                MyText { caption: qsTr("X"); text: CBSModel.tableX; onEnter: CBSModel.tableX= Number(text); }
                MyText { caption: qsTr("Y"); text: CBSModel.tableY; onEnter: CBSModel.tableY= Number(text); }
            }
            Flow { spacing: 10; width: parent.width;
                MyText { caption: qsTr("steps/mm X"); text: CBSModel.tableXSteps; onTextChanged: CBSModel.tableXSteps= Number(text); }
                MyText { caption: qsTr("Y"); text: CBSModel.tableYSteps; onTextChanged: CBSModel.tableYSteps= Number(text); }
            }
            Flow { spacing: 10; width: parent.width;
                MyText { caption: qsTr("slack(mm) X"); text: CBSModel.tableXSlack; onTextChanged: CBSModel.tableXSlack= Number(text); }
                MyText { caption: qsTr("Y");           text: CBSModel.tableYSlack; onTextChanged: CBSModel.tableYSlack= Number(text); }
            }
            Flow { spacing: 10; width: parent.width;
                MyText { id: xytableBtnGoX; caption: qsTr("go X"); text: CBSModel.tableX; onEnter: CBSModel.goTable(Number(xytableBtnGoX.text), Number(xytableBtnGoY.text)); }
                MyText { id: xytableBtnGoY; caption: qsTr("go Y"); text: CBSModel.tableY; onEnter: CBSModel.goTable(Number(xytableBtnGoX.text), Number(xytableBtnGoY.text)); }
                Button { text: qsTr("Go"); onPressed: CBSModel.goTable(Number(xytableBtnGoX.text), Number(xytableBtnGoY.text)); }
            }
            Flow { spacing: 10; width: parent.width;
                Text { text: "LED1"; color: "White" }
                Slider { from: 0; to: 255; value: CBSModel.tableLed1; onValueChanged: CBSModel.tableLed1= value; stepSize: 3; }
            }
            Flow { spacing: 10; width: parent.width;
                Text { text: "LED2"; color: "White" }
                Slider { from: 0; to: 255; value: CBSModel.tableLed2; onValueChanged: CBSModel.tableLed2= value; stepSize: 3; }
            }
            ComboBox { textRole: "val"; model: CBSModel.coms; currentIndex: 0; onCurrentIndexChanged: CBSModel.setCom(currentIndex); }
            Flow { spacing: 10; width: parent.width;
                function updatePos() { CBSModel.goTable2(xybx1.pressed?100:(xybx2.pressed?-100:0), xyby1.pressed?100:(xyby2.pressed?-100:0)); }
                Button { id: xybx1; text: "+X"; onPressed: parent.updatePos(); onReleased: parent.updatePos(); } 
                Button { id: xybx2; text: "-X"; onPressed: parent.updatePos(); onReleased: parent.updatePos(); } 
                Button { id: xyby1; text: "+Y"; onPressed: parent.updatePos(); onReleased: parent.updatePos(); } 
                Button { id: xyby2; text: "-Y"; onPressed: parent.updatePos(); onReleased: parent.updatePos(); } 
                MyText { caption: qsTr("mm/s");  text: CBSModel.tableSpd; onTextChanged: CBSModel.tableSpd= Number(text); }
            }
        }

        //********************************
        // ring page
        //********************************
        Column {
            width: window.width; height: parent.height;
            Flow { spacing: 10; width: parent.width;
                property double na: 180/nnb;
                property double nnb: Number(nb.text);
                property double ndis: Number(dis.text);
                property double ndos: Number(dos.text);
                property double nli: ndis/2*2*Math.tan(Math.PI/2/nnb);
                property double nlo: ndos/2*2*Math.tan(Math.PI/2/nnb);
                MyText { id:nb;  caption: qsTr("nb");       text: "12"; }
                MyText { id:dis; caption: qsTr("di-small"); text: "50"; onTextChanged: scopeView.model.secondaryToFocal= Number(text); }
                MyText { id:dos; caption: qsTr("do-small"); text: "70"; onTextChanged: scopeView.model.secondaryToFocal= Number(text); }
                MyText { id:dib; caption: qsTr("di-big");   text: 2*Math.sqrt((parent.ndis/2)*(parent.ndis/2)+(parent.nli/2)*(parent.nli/2)); }
                MyText { id:dob; caption: qsTr("do-big");   text: 2*Math.sqrt((parent.ndos/2)*(parent.ndos/2)+(parent.nlo/2)*(parent.nlo/2)); }
                MyText { id:li;  caption: qsTr("li");       text: parent.nli*2; }
                MyText { id:lo;  caption: qsTr("lo");       text: parent.nlo*2; }
                MyText { id:ep;  caption: qsTr("ep");       text: (parent.ndos-parent.ndis)/2; }
                MyText { id:a;   caption: qsTr("a");        text: parent.na; }
            }
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
                background: Rectangle { color: "black" }
                TabButton {  width: implicitWidth; text: qsTr("Scope") }
                TabButton {  width: implicitWidth; text: qsTr("EP") }
                TabButton {  width: implicitWidth; text: qsTr("Secondary") }
                TabButton {  width: implicitWidth; text: qsTr("Hogging") }
                TabButton {  width: implicitWidth; text: qsTr("Parabolizing") }
                TabButton {  width: implicitWidth; text: qsTr("Couder") }
                TabButton {  width: implicitWidth; text: qsTr("Webcam") }
                TabButton {  width: implicitWidth; text: qsTr("Support") }
                TabButton {  width: implicitWidth; text: qsTr("COG") }
                TabButton {  width: implicitWidth; text: qsTr("Notes") }
                TabButton {  width: implicitWidth; text: qsTr("Coms") }
                TabButton {  width: implicitWidth; text: qsTr("rings") }
            }
    }
}
