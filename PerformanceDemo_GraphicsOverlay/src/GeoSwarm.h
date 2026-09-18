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

#ifndef GEOSWARM_H
#define GEOSWARM_H

#include <QElapsedTimer>
#include <QObject>

#include <array>

namespace Esri::ArcGISRuntime
{
  class DictionaryRenderer;
  class DictionarySymbolStyle;
  class GraphicsOverlay;
  class Map;
  class MapQuickView;
  class Scene;
  class SceneQuickView;
  class SimpleRenderer;
} // namespace Esri::ArcGISRuntime

class GenerationController;
class GraphicsEntityController;

Q_MOC_INCLUDE("MapQuickView.h")
Q_MOC_INCLUDE("SceneQuickView.h")

class GeoSwarm : public QObject
{
  Q_OBJECT

  Q_PROPERTY(Esri::ArcGISRuntime::MapQuickView* mapView MEMBER m_mapView WRITE setMapView NOTIFY mapViewChanged)
  Q_PROPERTY(Esri::ArcGISRuntime::SceneQuickView* sceneView MEMBER m_sceneView WRITE setSceneView NOTIFY sceneViewChanged)
  Q_PROPERTY(bool sceneActive MEMBER m_sceneActive WRITE setSceneActive NOTIFY sceneActiveChanged)
  Q_PROPERTY(bool useDictionarySymbols MEMBER m_useDictionarySymbols WRITE setUseDictionarySymbols NOTIFY useDictionarySymbolsChanged)
  Q_PROPERTY(bool locationOnly MEMBER m_locationOnly WRITE setLocationOnly NOTIFY locationOnlyChanged)
  Q_PROPERTY(double obsPerSec MEMBER m_obsPerSec NOTIFY statusChanged)
  Q_PROPERTY(double entityIntervalMs MEMBER m_entityIntervalMs NOTIFY statusChanged)
  Q_PROPERTY(int entityCount READ entityCount NOTIFY statusChanged)

  Q_PROPERTY(GenerationController* generator MEMBER m_generator CONSTANT)

public:
  enum class Dim
  {
    Air = 0,
    Ground = 1,
    Sea = 2,
    Sub = 3,
    Count = 4
  };

  static constexpr int DimCount = static_cast<int>(Dim::Count);

  explicit GeoSwarm(QObject* parent = nullptr);
  ~GeoSwarm() override;

  int entityCount() const;

  void setSceneActive(bool sceneActive);
  void setUseDictionarySymbols(bool useDictionarySymbols);
  void setLocationOnly(bool locationOnly);

  Q_INVOKABLE void clearGraphics();
  Q_INVOKABLE void setLayerVisible(int dim, bool visible);
  Q_INVOKABLE bool layerVisible(int dim) const;

signals:
  void mapViewChanged();
  void sceneViewChanged();
  void sceneActiveChanged();
  void useDictionarySymbolsChanged();
  void locationOnlyChanged();
  void statusChanged();

private:
  void setMapView(Esri::ArcGISRuntime::MapQuickView* mapView);
  void setSceneView(Esri::ArcGISRuntime::SceneQuickView* sceneView);

  void setupRenderers();
  void applyRenderer();

  // Pushes the apply mask/attribute flags that follow symbol and randomize state.
  void syncApplyConfig();

  // Rebuilds the graphic pool from the generator's current config.
  void rebuildPool();

  // Moves all overlays onto the active view (map or scene).
  void moveOverlaysToView(bool toScene);
  void removeOverlaysFromViews();

  void initializeStatusTimers();

  Esri::ArcGISRuntime::Map* m_map = nullptr;
  Esri::ArcGISRuntime::Scene* m_scene = nullptr;
  Esri::ArcGISRuntime::MapQuickView* m_mapView = nullptr;
  Esri::ArcGISRuntime::SceneQuickView* m_sceneView = nullptr;

  GraphicsEntityController* m_graphics = nullptr;
  GenerationController* m_generator = nullptr;

  // One renderer instance per overlay;
  std::array<Esri::ArcGISRuntime::DictionaryRenderer*, DimCount> m_dictRenderers{};
  std::array<Esri::ArcGISRuntime::SimpleRenderer*, DimCount> m_simpleRenderers{};

  // Cached per-dimension visibility; survives a pool rebuild.
  std::array<bool, DimCount> m_layerVisible{{true, true, true, true}};

  Esri::ArcGISRuntime::DictionarySymbolStyle* m_dictStyle = nullptr;

  bool m_sceneActive = false;
  bool m_useDictionarySymbols = true;
  bool m_locationOnly = false;

  double m_obsPerSec = 0.0;
  double m_entityIntervalMs = 0.0;
  quint64 m_lastAppliedSample = 0;
  QElapsedTimer m_rateSampleTimer;
};

#endif // GEOSWARM_H
