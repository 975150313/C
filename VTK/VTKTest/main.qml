import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.3

import QtQuick.VtkItem 1.0
Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")
    color: Qt.rgba(25/255,51/255,76/255,1)
//    color: Qt.rgba(0,0,0,0)
    GridLayout{
            id: gridLayout1
            columns: 2;
            rows:2;
            anchors.fill: parent;
            anchors.margins: 5;
            columnSpacing: 0;
            rowSpacing: 0;
            VtkItem {
                id: qtVTK
                Layout.fillWidth: true;
                Layout.fillHeight: true;
            }
            VtkItem {
                id: qtVTK1
                Layout.fillWidth: true;
                Layout.fillHeight: true;
            }
            Rectangle{
                id:rect01;
                color: Qt.rgba(0,1,0,1)
                Layout.fillWidth: true;
                Layout.fillHeight: true;
            }
            Rectangle{
                id:rect02;
                color: Qt.rgba(1,0,0,1)
                Layout.fillWidth: true;
                Layout.fillHeight: true;
            }

        }
}
