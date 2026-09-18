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

#include "GenerationController.h"
#include "GeoSwarm.h"

#include "ArcGISRuntimeEnvironment.h"
#include "MapQuickView.h"
#include "SceneQuickView.h"
#include "PerformanceMonitor.h"

#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

//------------------------------------------------------------------------------

using namespace Esri::ArcGISRuntime;

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  // Qt Quick Controls style + palette overrides live in qrc:/qtquickcontrols2.conf
  // (loaded automatically at startup). Do not call QQuickStyle::setStyle here.

  // Use of ArcGIS location services, such as basemap styles, geocoding, and routing services,
  // requires an access token. For more information see
  // https://links.esri.com/arcgis-runtime-security-auth.

  // The following methods grant an access token:

  // 1. User authentication: Grants a temporary access token associated with a user's ArcGIS account.
  // To generate a token, a user logs in to the app with an ArcGIS account that is part of an
  // organization in ArcGIS Online or ArcGIS Enterprise.

  // 2. API key authentication: Get a long-lived access token that gives your application access to
  // ArcGIS location services. Go to the tutorial at https://links.esri.com/create-an-api-key.
  // Copy the API Key access token.

  const QString accessToken = qEnvironmentVariable("GEOSWARM_ARCGIS_API_KEY");

  if (!accessToken.isEmpty())
  {
    ArcGISRuntimeEnvironment::setApiKey(accessToken);
  }

  qmlRegisterType<MapQuickView>("GeoSwarm", 1, 0, "MapView");
  qmlRegisterType<SceneQuickView>("GeoSwarm", 1, 0, "SceneView");
  qmlRegisterType<GeoSwarm>("GeoSwarm", 1, 0, "GeoSwarm");
  qmlRegisterType<PerformanceMonitor>("GeoSwarm", 1, 0, "PerformanceMonitor");

  // Shared QML design tokens (qml/Theme.qml); panels use `import GeoSwarm.Theme`.
  qmlRegisterSingletonType(QUrl("qrc:/qml/Theme.qml"), "GeoSwarm.Theme", 1, 0, "Theme");

  // Owned by GeoSwarm, exposed via Q_PROPERTY — not constructable from QML.
  qmlRegisterUncreatableType<GenerationController>("GeoSwarm", 1, 0, "GenerationController", "Owned by GeoSwarm; access via geoSwarmModel.generator");

  QQmlApplicationEngine engine;
  engine.addImportPath(QDir(QCoreApplication::applicationDirPath()).filePath("qml"));
  engine.load(QUrl("qrc:/qml/main.qml"));

  return QGuiApplication::exec();
}

//------------------------------------------------------------------------------
