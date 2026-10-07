// Copyright 2026 ESRI
//
// All rights reserved under the copyright laws of the United States
// and applicable international laws, treaties, and conventions.
//
// You may freely redistribute and use this sample code, with or
// without modification, provided you include the original copyright
// notice and use restrictions.
//
// See the Sample code usage restrictions document for further information.
//

import QtQuick
import QtQuick.Controls
import GeoSwarm
import GeoSwarm.Theme

Item {
    id: geoSwarmForm

    property bool showScene: false
    property bool showPerformance: false
    property bool locationOnly: false
    readonly property bool compactTopPanels: width < (statusPanel.implicitWidth + settingsPanel.implicitWidth + 36)

    function attachPerfMonitorWindow() {
        perfMonitor.attachWindow(geoSwarmForm.Window.window);
    }

    Component.onCompleted: attachPerfMonitorWindow()
    onWindowChanged: attachPerfMonitorWindow()

    MapView {
        id: mapView
        anchors.fill: parent
        focus: !geoSwarmForm.showScene
        visible: !geoSwarmForm.showScene
    }

    SceneView {
        id: sceneView
        anchors.fill: parent
        focus: geoSwarmForm.showScene
        visible: geoSwarmForm.showScene
    }

    GeoSwarm {
        id: geoSwarmModel
        mapView: mapView
        sceneView: sceneView
        sceneActive: geoSwarmForm.showScene
        locationOnly: geoSwarmForm.locationOnly
    }

    PerformanceMonitor {
        id: perfMonitor
    }

    StatusPanel {
        id: statusPanel
        model: geoSwarmModel
        anchors {
            left: parent.left
            top: parent.top
            margins: 12
        }
    }

    SettingsPanel {
        id: settingsPanel
        model: geoSwarmModel
        showScene: geoSwarmForm.showScene
        showPerformance: geoSwarmForm.showPerformance
        locationOnly: geoSwarmForm.locationOnly
        onClearGraphicsRequested: geoSwarmModel.clearGraphics();
        onLocationOnlyRequested: (locationOnly) => geoSwarmForm.locationOnly = locationOnly
        onShowPerformanceRequested: (showPerformance) => geoSwarmForm.showPerformance = showPerformance
        onShowSceneRequested: (showScene) => geoSwarmForm.showScene = showScene
        anchors {
            left: geoSwarmForm.compactTopPanels ? parent.left : undefined
            right: geoSwarmForm.compactTopPanels ? undefined : parent.right
            top: geoSwarmForm.compactTopPanels ? statusPanel.bottom : parent.top
            margins: 12
        }
    }

    PerformancePanel {
        id: performancePanel
        monitor: perfMonitor
        visible: geoSwarmForm.showPerformance
        anchors {
            left: parent.left
            bottom: parent.bottom
            leftMargin: 12
            bottomMargin: 24
        }
    }

    // In-process simulator — free-floating dialog, toggled by the button below.
    SimulatorPanel {
        id: simulatorDialog
        controller: geoSwarmModel.generator
        visible: false
    }

    Rectangle {
        id: simulatorToggleBtn
        width: 140
        height: 36
        radius: Theme.radius
        color: {
            if (simulatorToggleMa.pressed) {
                return Theme.buttonPressed;
            }
            if (simulatorToggleMa.containsMouse) {
                return Theme.buttonHover;
            }
            return Theme.panelBg;
        }
        border.color: Theme.border
        border.width: 1
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: 36
        }

        Behavior on color { ColorAnimation { duration: 120 } }

        Text {
            anchors.centerIn: parent
            text: simulatorDialog.visible ? qsTr("Hide Simulator") : qsTr("Show Simulator")
            color: Theme.textPrimary
            font {
                family: Theme.fontUi
                pixelSize: 13
            }
        }

        MouseArea {
            id: simulatorToggleMa
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (simulatorDialog.visible) {
                    simulatorDialog.hide();
                }
                else {
                    simulatorDialog.show();
                    simulatorDialog.raise();
                    simulatorDialog.requestActivate();
                }
            }
        }
    }
}
