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

// Free-floating, non-modal simulator dialog (toggled from GeoSwarmForm).
Window {
    id: root

    required property GenerationController controller

    readonly property var flagDefs: [
        { label: qsTr("SIDC"),                   bit: 0,  defaultOn: true  },
        { label: qsTr("Altitude"),               bit: 1,  defaultOn: true  },
        { label: qsTr("Direction"),              bit: 2,  defaultOn: true  },
        { label: qsTr("Speed"),                  bit: 3,  defaultOn: true  },
        { label: qsTr("Status"),                 bit: 4,  defaultOn: false },
        { label: qsTr("Echelon Mobility"),       bit: 5,  defaultOn: false },
        { label: qsTr("HQ/TF/FD"),               bit: 6,  defaultOn: false },
        { label: qsTr("Unique Designation"),     bit: 7,  defaultOn: false },
        { label: qsTr("Additional Information"), bit: 8,  defaultOn: false },
        { label: qsTr("Quantity"),               bit: 9,  defaultOn: false },
        { label: qsTr("Combat Effectiveness"),   bit: 10, defaultOn: false },
        { label: qsTr("Staff Comment"),          bit: 11, defaultOn: false },
        { label: qsTr("Higher Formation"),       bit: 12, defaultOn: false },
        { label: qsTr("Country Label"),          bit: 13, defaultOn: false },
        { label: qsTr("Reinforced"),             bit: 14, defaultOn: false },
        { label: qsTr("Valid Time"),             bit: 15, defaultOn: false }
    ]

    width: 420
    height: 640
    minimumWidth: 360
    minimumHeight: 480
    title: qsTr("Message Simulator")
    color: Theme.panelBg
    flags: Qt.Dialog
    modality: Qt.NonModal

    Pane {
        anchors.fill: parent
        padding: 0

        background: Rectangle {
            color: Theme.panelBg
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            TabBar {
                id: tabBar
                Layout.fillWidth: true
                TabButton {
                    id: symbolsTab
                    text: qsTr("Symbols")
                    contentItem: Text {
                        text: symbolsTab.text
                        color: Theme.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font {
                            family: Theme.fontUi
                            pixelSize: 13
                            bold: symbolsTab.checked
                        }
                    }
                    background: Rectangle {
                        color: symbolsTab.checked
                               ? (symbolsTab.pressed ? Theme.primaryPressed : Theme.primary)
                               : (symbolsTab.pressed ? Theme.buttonPressed
                                                     : symbolsTab.hovered ? Theme.buttonHover : Theme.buttonBg)
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                }
                TabButton {
                    id: messagesTab
                    text: qsTr("Messages")
                    contentItem: Text {
                        text: messagesTab.text
                        color: Theme.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font {
                            family: Theme.fontUi
                            pixelSize: 13
                            bold: messagesTab.checked
                        }
                    }
                    background: Rectangle {
                        color: messagesTab.checked
                               ? (messagesTab.pressed ? Theme.primaryPressed : Theme.primary)
                               : (messagesTab.pressed ? Theme.buttonPressed
                                                      : messagesTab.hovered ? Theme.buttonHover : Theme.buttonBg)
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                }
            }

            StackLayout {
                currentIndex: tabBar.currentIndex
                Layout.fillWidth: true
                Layout.fillHeight: true

                // === Tab 1: Symbols (composition + motion + attribute selection) ===
                ScrollView {
                    id: symbolsScroll
                    clip: true
                    contentWidth: availableWidth

                    ColumnLayout {
                        id: symbolsCol
                        property int dimWeightSum: root.controller
                                                   ? (root.controller.dimWeightAir + root.controller.dimWeightGround
                                                      + root.controller.dimWeightSea + root.controller.dimWeightSub)
                                                   : 0
                        width: symbolsScroll.availableWidth

                        function dimPct(w: int): int {
                            if (!root.controller || dimWeightSum <= 0) {
                                return 25;
                            }
                            return Math.round(100 * w / dimWeightSum);
                        }
                        spacing: 12

                        GroupBox {
                            Layout.fillWidth: true
                            Layout.margins: 12
                            label: Text {
                                text: qsTr("Composition (battle dimension mix)")
                                color: Theme.textPrimary
                                font {
                                    family: Theme.fontUi
                                    pixelSize: 13
                                    bold: true
                                }
                            }

                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 6

                                component DimRow : RowLayout {
                                    property string dimLabel
                                    property int weight: 0
                                    property int pct: 0
                                    signal weightAdjusted(int value)
                                    spacing: 8

                                    Text {
                                        text: dimLabel
                                        color: Theme.textPrimary
                                        Layout.preferredWidth: 90
                                        font {
                                            family: Theme.fontUi
                                            pixelSize: 12
                                        }
                                    }
                                    Slider {
                                        from: 0; to: 100; stepSize: 1
                                        value: weight
                                        enabled: !(root.controller && root.controller.running)
                                        Layout.fillWidth: true
                                        onMoved: parent.weightAdjusted(value);
                                    }
                                    Text {
                                        text: weight
                                        color: Theme.textPrimary
                                        Layout.preferredWidth: 24
                                        horizontalAlignment: Text.AlignRight
                                        font {
                                            family: Theme.fontMono
                                            pixelSize: 12
                                        }
                                    }
                                    Text {
                                        text: pct + "%"
                                        color: Theme.textMuted
                                        Layout.preferredWidth: 36
                                        horizontalAlignment: Text.AlignRight
                                        font {
                                            family: Theme.fontMono
                                            pixelSize: 12
                                        }
                                    }
                                }

                                DimRow {
                                    dimLabel: qsTr("Air")
                                    weight: root.controller ? root.controller.dimWeightAir : 1
                                    pct: symbolsCol.dimPct(weight)
                                    Layout.fillWidth: true
                                    onWeightAdjusted: (v) => {
                                                          if (root.controller) {
                                                              root.controller.dimWeightAir = v;
                                                          }
                                                      }
                                }
                                DimRow {
                                    dimLabel: qsTr("Ground")
                                    weight: root.controller ? root.controller.dimWeightGround : 1
                                    pct: symbolsCol.dimPct(weight)
                                    Layout.fillWidth: true
                                    onWeightAdjusted: (v) => {
                                                          if (root.controller) {
                                                              root.controller.dimWeightGround = v;
                                                          }
                                                      }
                                }
                                DimRow {
                                    dimLabel: qsTr("Sea")
                                    weight: root.controller ? root.controller.dimWeightSea : 1
                                    pct: symbolsCol.dimPct(weight)
                                    Layout.fillWidth: true
                                    onWeightAdjusted: (v) => {
                                                          if (root.controller) {
                                                              root.controller.dimWeightSea = v;
                                                          }
                                                      }
                                }
                                DimRow {
                                    dimLabel: qsTr("Subsurface")
                                    weight: root.controller ? root.controller.dimWeightSub : 1
                                    pct: symbolsCol.dimPct(weight)
                                    Layout.fillWidth: true
                                    onWeightAdjusted: (v) => {
                                                          if (root.controller) {
                                                              root.controller.dimWeightSub = v;
                                                          }
                                                      }
                                }

                                Text {
                                    text: symbolsCol.dimWeightSum > 0
                                          ? qsTr("Weights are auto-normalized at spawn.")
                                          : qsTr("All weights zero → uniform mix across the 4.")
                                    color: Theme.textMuted
                                    wrapMode: Text.Wrap
                                    Layout.topMargin: 4
                                    Layout.fillWidth: true
                                    font {
                                        family: Theme.fontUi
                                        pixelSize: 11
                                    }
                                }
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true
                            Layout.margins: 12
                            label: Text {
                                text: qsTr("Motion")
                                color: Theme.textPrimary
                                font {
                                    family: Theme.fontUi
                                    pixelSize: 13
                                    bold: true
                                }
                            }

                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 6

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    Text {
                                        text: qsTr("Heading drift")
                                        color: Theme.textPrimary
                                        Layout.preferredWidth: 110
                                        font {
                                            family: Theme.fontUi
                                            pixelSize: 12
                                        }
                                    }
                                    Slider {
                                        id: jitterSlider
                                        from: 0.0; to: 0.5; stepSize: 0.005
                                        value: root.controller ? root.controller.headingJitter : 0.05
                                        enabled: !(root.controller && root.controller.running)
                                        Layout.fillWidth: true
                                        onMoved: {
                                            if (root.controller) {
                                                root.controller.headingJitter = value;
                                            }
                                        }
                                    }
                                    Text {
                                        text: jitterSlider.value.toFixed(3) + " rad/fr"
                                        color: Theme.textPrimary
                                        Layout.preferredWidth: 110
                                        horizontalAlignment: Text.AlignRight
                                        font {
                                            family: Theme.fontMono
                                            pixelSize: 12
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    Text {
                                        text: qsTr("Speed multiplier")
                                        color: Theme.textPrimary
                                        Layout.preferredWidth: 110
                                        font {
                                            family: Theme.fontUi
                                            pixelSize: 12
                                        }
                                    }
                                    Slider {
                                        id: motionSlider
                                        Layout.fillWidth: true
                                        from: 1.0; to: 2000.0
                                        stepSize: 0
                                        value: root.controller ? root.controller.motionScale : 200.0
                                        enabled: !(root.controller && root.controller.running)
                                        onMoved: {
                                            const snapped = Math.max(Math.round(value / 10.0) * 10.0, 1.0);
                                            if (root.controller) {
                                                root.controller.motionScale = snapped;
                                            }
                                        }
                                    }
                                    Text {
                                        text: motionSlider.value.toFixed(0) + "x"
                                        color: Theme.textPrimary
                                        Layout.preferredWidth: 110
                                        horizontalAlignment: Text.AlignRight
                                        font {
                                            family: Theme.fontMono
                                            pixelSize: 12
                                        }
                                    }
                                }
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true
                            Layout.margins: 12
                            label: Text {
                                text: qsTr("Attributes")
                                color: Theme.textPrimary
                                font {
                                    family: Theme.fontUi
                                    pixelSize: 13
                                    bold: true
                                }
                            }

                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 8

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    rowSpacing: 2
                                    columnSpacing: 12

                                    Repeater {
                                        model: root.flagDefs
                                        delegate: CheckBox {
                                            id: attrCheck
                                            required property var modelData
                                            text: modelData.label
                                            Layout.fillWidth: true
                                            contentItem: Text {
                                                text: attrCheck.text
                                                color: Theme.textPrimary
                                                font {
                                                    family: Theme.fontUi
                                                    pixelSize: 12
                                                }
                                                leftPadding: attrCheck.indicator.width + attrCheck.spacing
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                            checked: root.controller ? !!(root.controller.flags & (1 << modelData.bit)) : modelData.defaultOn
                                            enabled: !(root.controller && root.controller.running)
                                            onToggled: {
                                                if (!root.controller) {
                                                    return;
                                                }

                                                const mask = (1 << modelData.bit);
                                                if (checked) {
                                                    root.controller.flags = root.controller.flags | mask;
                                                }
                                                else {
                                                    root.controller.flags = root.controller.flags & ~mask;
                                                }
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Layout.topMargin: 6
                                    spacing: 8

                                    Text {
                                        text: qsTr("Randomize per pass")
                                        color: Theme.textPrimary
                                        font {
                                            family: Theme.fontUi
                                            pixelSize: 12
                                        }
                                    }
                                    Item { Layout.fillWidth: true }
                                    Switch {
                                        checked: root.controller ? root.controller.randomize : false
                                        enabled: !(root.controller && root.controller.running)
                                        onToggled: {
                                            if (root.controller) {
                                                root.controller.randomize = checked;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // === Tab 2: Messages (rate config + live stats) ===
                ScrollView {
                    id: messagesScroll
                    contentWidth: availableWidth
                    clip: true

                    ColumnLayout {
                        width: messagesScroll.availableWidth
                        spacing: 12

                        GroupBox {
                            Layout.fillWidth: true
                            Layout.margins: 12
                            label: Text {
                                text: qsTr("Configuration")
                                color: Theme.textPrimary
                                font {
                                    family: Theme.fontUi
                                    pixelSize: 13
                                    bold: true
                                }
                            }

                            GridLayout {
                                anchors.fill: parent
                                columns: 2
                                rowSpacing: 6
                                columnSpacing: 12

                                Text {
                                    text: qsTr("Entity count")
                                    color: Theme.textPrimary
                                    font {
                                        family: Theme.fontUi
                                        pixelSize: 12
                                    }
                                }
                                SpinBox {
                                    id: entityCountSpin
                                    from: 1; to: 1000000; stepSize: 100; editable: true
                                    value: root.controller ? root.controller.entityCount : 10000
                                    enabled: !(root.controller && root.controller.running)
                                    Layout.fillWidth: true

                                    function commitValue() {
                                        const parsedValue = Number(contentItem.text.split(locale.groupSeparator).join(""));
                                        if (root.controller && Number.isNaN(parsedValue) === false
                                                && parsedValue >= from && parsedValue <= to) {
                                            root.controller.entityCount = parsedValue;
                                        }
                                    }

                                    onValueModified: {
                                        if (root.controller) {
                                            root.controller.entityCount = value;
                                        }
                                    }
                                }

                                Text {
                                    text: qsTr("Target obs/sec")
                                    color: Theme.textPrimary
                                    font {
                                        family: Theme.fontUi
                                        pixelSize: 12
                                    }
                                }
                                SpinBox {
                                    id: targetObsPerSecSpin
                                    from: 1; to: 5000000; stepSize: 1000; editable: true
                                    value: root.controller ? root.controller.targetObsPerSec : 100000
                                    enabled: !(root.controller && root.controller.running)
                                    Layout.fillWidth: true

                                    function commitValue() {
                                        const parsedValue = Number(contentItem.text.split(locale.groupSeparator).join(""));
                                        if (root.controller && Number.isNaN(parsedValue) === false
                                                && parsedValue >= from && parsedValue <= to) {
                                            root.controller.targetObsPerSec = parsedValue;
                                        }
                                    }

                                    onValueModified: {
                                        if (root.controller) {
                                            root.controller.targetObsPerSec = value;
                                        }
                                    }
                                }
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true
                            Layout.margins: 12
                            label: Text {
                                text: qsTr("Live stats (1 Hz)")
                                color: Theme.textPrimary
                                font {
                                    family: Theme.fontUi
                                    pixelSize: 13
                                    bold: true
                                }
                            }

                            GridLayout {
                                anchors.fill: parent
                                columns: 2
                                rowSpacing: 4
                                columnSpacing: 12

                                Text {
                                    text: qsTr("Obs/sec generated")
                                    color: Theme.textPrimary
                                    font {
                                        family: Theme.fontUi
                                        pixelSize: 12
                                    }
                                }
                                Text {
                                    text: root.controller ? root.controller.obsPerSec.toFixed(0) : "0"
                                    color: Theme.textPrimary
                                    font {
                                        family: Theme.fontMono
                                        pixelSize: 12
                                    }
                                }

                                Text {
                                    text: qsTr("Passes/sec")
                                    color: Theme.textPrimary
                                    font {
                                        family: Theme.fontUi
                                        pixelSize: 12
                                    }
                                }
                                Text {
                                    text: root.controller ? root.controller.passesPerSecActual.toFixed(2) : "0"
                                    color: Theme.textPrimary
                                    font {
                                        family: Theme.fontMono
                                        pixelSize: 12
                                    }
                                }

                                Text {
                                    text: qsTr("Generate µs/pass")
                                    color: Theme.textPrimary
                                    font {
                                        family: Theme.fontUi
                                        pixelSize: 12
                                    }
                                }
                                Text {
                                    text: root.controller ? root.controller.generateUs.toFixed(2) : "0"
                                    color: Theme.textPrimary
                                    font {
                                        family: Theme.fontMono
                                        pixelSize: 12
                                    }
                                }

                                Text {
                                    text: qsTr("Publish µs/pass")
                                    color: Theme.textPrimary
                                    font {
                                        family: Theme.fontUi
                                        pixelSize: 12
                                    }
                                }
                                Text {
                                    text: root.controller ? root.controller.publishUs.toFixed(2) : "0"
                                    color: Theme.textPrimary
                                    font {
                                        family: Theme.fontMono
                                        pixelSize: 12
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Pane {
                Layout.fillWidth: true
                padding: 12

                Button {
                    id: startStopButton
                    text: root.controller && root.controller.running ? qsTr("Stop") : qsTr("Start")
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    contentItem: Text {
                        text: startStopButton.text
                        color: Theme.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font {
                            family: Theme.fontUi
                            pixelSize: 13
                        }
                    }
                    onClicked: {
                        if (!root.controller) {
                            return;
                        }

                        if (root.controller.running) {
                            root.controller.stop();
                        }
                        else {
                            entityCountSpin.commitValue();
                            targetObsPerSecSpin.commitValue();
                            root.controller.start();
                        }
                    }
                }
            }
        }
    }
}
