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
import QtQuick.Layouts
import GeoSwarm
import GeoSwarm.Theme

Rectangle {
    id: root

    required property GeoSwarm model

    // Label + value pair, reused for each metric below.
    component StatBlock: ColumnLayout {
        id: block
        Layout.fillWidth: true
        property string label
        property string value
        property bool big: false
        spacing: 4

        Text {
            text: block.label
            color: Theme.textLabel
            font {
                family: Theme.fontUi
                pixelSize: 11
            }
        }

        Text {
            text: block.value
            color: Theme.textPrimary
            font {
                family: Theme.fontMono
                pixelSize: block.big ? 18 : 14
                bold: block.big
            }
        }
    }

    implicitWidth: 230
    implicitHeight: Math.min(
                        contentColumn.implicitHeight + 24,
                        parent ? Math.max(parent.height - 24, 0) : contentColumn.implicitHeight + 24)
    color: Theme.panelBg
    radius: Theme.radius
    border.color: Theme.border
    border.width: 1

    // Swallow events on the panel so they don't reach the view beneath.
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        hoverEnabled: true
        onWheel: (wheel) => wheel.accepted = true;
    }

    ScrollView {
        id: scroller
        anchors {
            fill: parent
            margins: 12
        }
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            id: contentColumn
            width: scroller.availableWidth
            spacing: 12

            Text {
                text: qsTr("STATUS")
                color: Theme.textPrimary
                Layout.fillWidth: true
                font {
                    family: Theme.fontUi
                    pixelSize: 11
                    letterSpacing: 1.5
                    bold: true
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

            StatBlock {
                label: qsTr("Graphic updates applied/sec")
                value: root.model ? root.model.obsPerSec.toLocaleString(Qt.locale(), "f", 0) : "0"
                big: true
            }

            // Per-entity refresh interval: 1000 * entityCount / obsPerSec.
            StatBlock {
                label: qsTr("Update interval per entity")
                value: root.model ? root.model.entityIntervalMs.toLocaleString(Qt.locale(), "f", 0) + " ms" : "0 ms"
            }

            StatBlock {
                label: qsTr("Tracked entities")
                value: root.model ? root.model.entityCount.toLocaleString(Qt.locale(), "f", 0) : "0"
            }
        }
    }
}
