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

    // Parent owns the source of truth; the panel reflects state and emits user intent.
    property bool showScene: false
    property bool showPerformance: false
    property bool useDictionarySymbols: true
    property bool locationOnly: false

    // Layer visibility flags (toggling calls model.setLayerVisible directly).
    property bool airVisible: true
    property bool groundVisible: true
    property bool seaVisible: true
    property bool subVisible: true

    signal clearGraphicsRequested()
    signal showSceneRequested(bool showScene)
    signal showPerformanceRequested(bool showPerformance)
    signal useDictionarySymbolsRequested(bool useDictionarySymbols)
    signal locationOnlyRequested(bool locationOnly)

    // label + Switch, reused for the on/off toggles below.
    component ToggleRow: RowLayout {
        id: trow
        Layout.fillWidth: true
        property string label
        property bool checked: false
        property bool enabled: true
        signal toggled(bool on)
        spacing: 10

        Text {
            text: trow.label
            color: Theme.textPrimary
            font {
                family: Theme.fontUi
                pixelSize: 13
            }
        }

        Item { Layout.fillWidth: true }

        Switch {
            checked: trow.checked
            enabled: trow.enabled
            Layout.alignment: Qt.AlignVCenter
            onToggled: trow.toggled(checked);
        }
    }

    // label + SpinBox that commits live on every keystroke.
    component SpinRow: RowLayout {
        id: srow
        Layout.fillWidth: true
        property string label
        property int from: 0
        property int to: 1000
        property int value: 0
        property bool enabled: true
        signal edited(int newValue)
        spacing: 6

        Text {
            text: srow.label
            color: Theme.textPrimary
            font {
                family: Theme.fontUi
                pixelSize: 12
            }
        }

        Item { Layout.fillWidth: true }

        SpinBox {
            id: spin
            from: srow.from; to: srow.to; stepSize: 1; editable: true
            value: srow.value
            enabled: srow.enabled
            implicitWidth: 96
            onValueModified: srow.edited(value);

            Connections {
                target: spin.contentItem

                function onTextEdited() {
                    const parsedValue = parseInt(spin.contentItem.text);
                    if (Number.isNaN(parsedValue) === false && parsedValue >= srow.from && parsedValue <= srow.to) {
                        srow.edited(parsedValue);
                    }
                }
            }
        }
    }

    implicitWidth: 260
    implicitHeight: Math.min(
                        contentColumn.implicitHeight + 24,
                        parent ? Math.max(parent.height - 24, 0) : contentColumn.implicitHeight + 24)
    color: Theme.panelBg
    radius: Theme.radius
    border.color: Theme.border
    border.width: 1

    // Swallow events on dead space so they don't reach the view beneath.
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
                text: qsTr("SETTINGS")
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

            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true

                Text {
                    text: qsTr("View mode")
                    color: Theme.textLabel
                    font {
                        family: Theme.fontUi
                        pixelSize: 11
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 30
                    radius: 4
                    color: Theme.buttonBg
                    border.color: Theme.border
                    border.width: 1
                    clip: true

                    // Enforces exactly one selected segment.
                    ButtonGroup { id: viewModeGroup }

                    RowLayout {
                        anchors.fill: parent
                        spacing: 0

                        component SegButton: Button {
                            id: segButton
                            ButtonGroup.group: viewModeGroup
                            checkable: true
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.preferredWidth: 1

                            contentItem: Text {
                                text: segButton.text
                                color: Theme.textPrimary
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font {
                                    family: Theme.fontUi
                                    pixelSize: 13
                                    bold: segButton.checked
                                }
                            }

                            background: Rectangle {
                                color: segButton.checked
                                       ? (segButton.pressed ? Theme.primaryPressed : Theme.primary)
                                       : (segButton.pressed ? Theme.buttonPressed
                                                            : segButton.hovered ? Theme.buttonHover
                                                                                : Theme.buttonBg)
                                Behavior on color { ColorAnimation { duration: 120 } }
                            }
                        }

                        SegButton {
                            text: qsTr("Map")
                            checked: !root.showScene
                            onClicked: root.showSceneRequested(false);
                        }

                        Rectangle {
                            Layout.preferredWidth: 1
                            Layout.fillHeight: true
                            color: Theme.border
                        }

                        SegButton {
                            text: qsTr("Scene")
                            checked: root.showScene
                            onClicked: root.showSceneRequested(true);
                        }
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

            ToggleRow {
                label: qsTr("Show performance HUD")
                checked: root.showPerformance
                onToggled: (on) => root.showPerformanceRequested(on);
            }

            ToggleRow {
                label: qsTr("Use MIL-2525C symbols")
                checked: root.useDictionarySymbols
                onToggled: (on) => root.useDictionarySymbolsRequested(on);
            }

            ToggleRow {
                label: qsTr("Show symbol modifiers")
                checked: !root.locationOnly
                onToggled: (on) => root.locationOnlyRequested(!on);
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true

                Text {
                    text: qsTr("Layer visibility")
                    color: Theme.textLabel
                    font {
                        family: Theme.fontUi
                        pixelSize: 11
                    }
                }

                Repeater {
                    model: [
                        { label: qsTr("Air"),        dim: 0, propName: "airVisible" },
                        { label: qsTr("Ground"),     dim: 1, propName: "groundVisible" },
                        { label: qsTr("Sea"),        dim: 2, propName: "seaVisible" },
                        { label: qsTr("Subsurface"), dim: 3, propName: "subVisible" }
                    ]

                    delegate: ToggleRow {
                        required property var modelData
                        label: modelData.label
                        checked: root[modelData.propName]
                        onToggled: (on) => {
                                       root[modelData.propName] = on;
                                       if (root.model) {
                                           root.model.setLayerVisible(modelData.dim, on);
                                       }
                                   }
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

            Rectangle {
                id: clearButton
                implicitHeight: 32
                radius: 4
                color: {
                    if (clearMa.pressed) {
                        return Theme.buttonPressed;
                    }
                    if (clearMa.containsMouse) {
                        return Theme.buttonHover;
                    }
                    return Theme.buttonBg;
                }
                border.color: Theme.border
                border.width: 1
                Layout.fillWidth: true

                Behavior on color { ColorAnimation { duration: 120 } }

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Clear graphics")
                    color: Theme.textPrimary
                    font {
                        family: Theme.fontUi
                        pixelSize: 13
                    }
                }

                MouseArea {
                    id: clearMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.clearGraphicsRequested();
                }
            }
        }
    }
}
