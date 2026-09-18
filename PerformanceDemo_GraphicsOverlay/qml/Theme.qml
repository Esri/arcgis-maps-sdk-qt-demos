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

pragma Singleton
import QtQuick

// Shared design tokens (fonts, colors, metrics) for the dark UI.
QtObject {
    id: root

    // Fonts
    readonly property string fontUi: "Segoe UI, Helvetica, Arial, sans-serif"
    readonly property string fontMono: "Consolas, Courier New, monospace"

    // Surfaces
    readonly property color panelBg: "#1a1a1a"
    readonly property color border: "#3a3a3a"

    // Text
    readonly property color textPrimary: "#ffffff"
    readonly property color textValue: "#cccccc"
    readonly property color textLabel: "#aaaaaa"
    readonly property color textMuted: "#888888"
    readonly property color textFaint: "#666666"

    // Controls
    readonly property color buttonBg: "#2a2a2a"
    readonly property color buttonHover: "#333333"
    readonly property color buttonPressed: "#1f1f1f"
    readonly property color accent: "#2a5a30"
    readonly property color accentHover: "#2e6f3a"

    readonly property color primary: "#8F53CA"
    readonly property color primaryHover: "#7938B6"
    readonly property color primaryPressed: "#652E98"

    // Status (FPS thresholds)
    readonly property color good: "#5cb85c"
    readonly property color warn: "#f0ad4e"
    readonly property color bad: "#d9534f"

    // Metrics
    readonly property int radius: 6
}
