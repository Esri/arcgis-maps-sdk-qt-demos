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
    id: simulatorPanel

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
                        property int dimWeightSum: simulatorPanel.controller
                                                   ? (simulatorPanel.controller.dimWeightAir + simulatorPanel.controller.dimWeightGround
                                                      + simulatorPanel.controller.dimWeightSea + simulatorPanel.controller.dimWeightSub)
                                                   : 0
                        width: symbolsScroll.availableWidth

                        function dimPct(w: int): int {
                            if (!simulatorPanel.controller || dimWeightSum <= 0) {
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
                                        enabled: !(simulatorPanel.controller && simulatorPanel.controller.running)
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
                                    weight: simulatorPanel.controller ? simulatorPanel.controller.dimWeightAir : 1
                                    pct: symbolsCol.dimPct(weight)
                                    Layout.fillWidth: true
                                    onWeightAdjusted: (v) => {
                                                          if (simulatorPanel.controller) {
                                                              simulatorPanel.controller.dimWeightAir = v;
                                                          }
                                                      }
                                }
                                DimRow {
                                    dimLabel: qsTr("Ground")
                                    weight: simulatorPanel.controller ? simulatorPanel.controller.dimWeightGround : 1
                                    pct: symbolsCol.dimPct(weight)
                                    Layout.fillWidth: true
                                    onWeightAdjusted: (v) => {
                                                          if (simulatorPanel.controller) {
                                                              simulatorPanel.controller.dimWeightGround = v;
                                                          }
                                                      }
                                }
                                DimRow {
                                    dimLabel: qsTr("Sea")
                                    weight: simulatorPanel.controller ? simulatorPanel.controller.dimWeightSea : 1
                                    pct: symbolsCol.dimPct(weight)
                                    Layout.fillWidth: true
                                    onWeightAdjusted: (v) => {
                                                          if (simulatorPanel.controller) {
                                                              simulatorPanel.controller.dimWeightSea = v;
                                                          }
                                                      }
                                }
                                DimRow {
                                    dimLabel: qsTr("Subsurface")
                                    weight: simulatorPanel.controller ? simulatorPanel.controller.dimWeightSub : 1
                                    pct: symbolsCol.dimPct(weight)
                                    Layout.fillWidth: true
                                    onWeightAdjusted: (v) => {
                                                          if (simulatorPanel.controller) {
                                                              simulatorPanel.controller.dimWeightSub = v;
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
                                        from: 0.0; to: 0.02; stepSize: 0.0005
                                        value: simulatorPanel.controller ? simulatorPanel.controller.headingJitter : 0.005
                                        enabled: !(simulatorPanel.controller && simulatorPanel.controller.running)
                                        Layout.fillWidth: true
                                        onMoved: {
                                            if (simulatorPanel.controller) {
                                                simulatorPanel.controller.headingJitter = value;
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
                                        value: simulatorPanel.controller ? simulatorPanel.controller.motionScale : 200.0
                                        enabled: !(simulatorPanel.controller && simulatorPanel.controller.running)
                                        onMoved: {
                                            const snapped = Math.max(Math.round(value / 10.0) * 10.0, 1.0);
                                            if (simulatorPanel.controller) {
                                                simulatorPanel.controller.motionScale = snapped;
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
                                        model: simulatorPanel.flagDefs
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
                                            checked: simulatorPanel.controller ? !!(simulatorPanel.controller.flags & (1 << modelData.bit)) : modelData.defaultOn
                                            enabled: !(simulatorPanel.controller && simulatorPanel.controller.running)
                                            onToggled: {
                                                if (!simulatorPanel.controller) {
                                                    return;
                                                }

                                                const mask = (1 << modelData.bit);
                                                if (checked) {
                                                    simulatorPanel.controller.flags = simulatorPanel.controller.flags | mask;
                                                }
                                                else {
                                                    simulatorPanel.controller.flags = simulatorPanel.controller.flags & ~mask;
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
                                        checked: simulatorPanel.controller ? simulatorPanel.controller.randomize : false
                                        enabled: !(simulatorPanel.controller && simulatorPanel.controller.running)
                                        onToggled: {
                                            if (simulatorPanel.controller) {
                                                simulatorPanel.controller.randomize = checked;
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
                                    value: simulatorPanel.controller ? simulatorPanel.controller.entityCount : 10000
                                    enabled: !(simulatorPanel.controller && simulatorPanel.controller.running)
                                    Layout.fillWidth: true

                                    function commitValue() {
                                        const parsedValue = Number(contentItem.text.split(locale.groupSeparator).join(""));
                                        if (simulatorPanel.controller && Number.isNaN(parsedValue) === false
                                                && parsedValue >= from && parsedValue <= to) {
                                            simulatorPanel.controller.entityCount = parsedValue;
                                        }
                                    }

                                    onValueModified: {
                                        if (simulatorPanel.controller) {
                                            simulatorPanel.controller.entityCount = value;
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
                                    value: simulatorPanel.controller ? simulatorPanel.controller.targetObsPerSec : 100000
                                    enabled: !(simulatorPanel.controller && simulatorPanel.controller.running)
                                    Layout.fillWidth: true

                                    function commitValue() {
                                        const parsedValue = Number(contentItem.text.split(locale.groupSeparator).join(""));
                                        if (simulatorPanel.controller && Number.isNaN(parsedValue) === false
                                                && parsedValue >= from && parsedValue <= to) {
                                            simulatorPanel.controller.targetObsPerSec = parsedValue;
                                        }
                                    }

                                    onValueModified: {
                                        if (simulatorPanel.controller) {
                                            simulatorPanel.controller.targetObsPerSec = value;
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
                                    text: simulatorPanel.controller ? simulatorPanel.controller.obsPerSec.toFixed(0) : "0"
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
                                    text: simulatorPanel.controller ? simulatorPanel.controller.passesPerSecActual.toFixed(2) : "0"
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
                                    text: simulatorPanel.controller ? simulatorPanel.controller.generateUs.toFixed(2) : "0"
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
                                    text: simulatorPanel.controller ? simulatorPanel.controller.publishUs.toFixed(2) : "0"
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
                    text: simulatorPanel.controller && simulatorPanel.controller.running ? qsTr("Stop") : qsTr("Start")
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
                        if (!simulatorPanel.controller) {
                            return;
                        }

                        if (simulatorPanel.controller.running) {
                            simulatorPanel.controller.stop();
                        }
                        else {
                            entityCountSpin.commitValue();
                            targetObsPerSecSpin.commitValue();
                            simulatorPanel.controller.start();
                        }
                    }
                }
            }
        }
    }
}
