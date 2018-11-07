pragma Singleton
import QtQuick 2.11
import QtQuick.Window 2.11

//used to keep our scaling and sizing information in one place to quickly change everything as needed
QtObject
{
    id: root

    readonly property real refScreenWidth: 500
    readonly property real refScreenHeight: 600

    readonly property real screenWidth: 500 //ScreenInfo.width
    readonly property real screenHeight: 600 //ScreenInfo.height

    function hscale(size) {
        return Math.round(size * (screenWidth / refScreenWidth))
    }

    function vscale(size) {
        return Math.round(size * (screenHeight / refScreenHeight))
    }

    function tscale(size) {
        return Math.round((hscale(size) + vscale(size)) / 2)
    }

    readonly property real screenLeftMargin: hscale(16)
    readonly property real screenRightMargin: screenLeftMargin
    readonly property real statusBarHeight: vscale(79)
    readonly property int mainPageHeight: vscale(screenHeight)
    readonly property int mainPageWidth: hscale(screenWidth)
    readonly property real appToolBarHeight: vscale(120)
    readonly property real dividerSize: 1

}
