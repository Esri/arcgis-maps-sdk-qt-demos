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

#include "GeoSwarm.h"

#include "GenerationController.h"
#include "GenerationWorker.h"
#include "GraphicsEntityController.h"
#include "Observation.h"

#include "ArcGISRuntimeEnvironment.h"
#include "ArcGISTiledElevationSource.h"
#include "ArcGISTiledLayer.h"
#include "Basemap.h"
#include "Camera.h"
#include "DictionaryRenderer.h"
#include "DictionarySymbolStyle.h"
#include "ElevationSourceListModel.h"
#include "Envelope.h"
#include "GraphicsOverlay.h"
#include "GraphicsOverlayListModel.h"
#include "Map.h"
#include "MapQuickView.h"
#include "MapTypes.h"
#include "Point.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneQuickView.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleRenderer.h"
#include "SpatialReference.h"
#include "Surface.h"
#include "SymbolTypes.h"
#include "Viewpoint.h"

#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFuture>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

using namespace Esri::ArcGISRuntime;

static_assert(GeoSwarm::DimCount == kDimCount, "GeoSwarm::DimCount must match kDimCount (battle dimensions)");

static const QString kStylxResource = QStringLiteral(":/Resources/Symbols/mil2525c.stylx");
static const QUrl kWorldElevationUrl =
  QUrl(QStringLiteral("https://elevation3d.arcgis.com/arcgis/rest/services/WorldElevation3D/Terrain3D/ImageServer"));
static const QUrl kPublicBasemapUrl = QUrl(QStringLiteral("https://services.arcgisonline.com/arcgis/rest/services/World_Topo_Map/MapServer"));
static constexpr double kSceneInitialAltitudeMeters = 25000000.0;
static constexpr double kSceneInitialHeadingDegrees = 0.0;
static constexpr double kSceneInitialPitchDegrees = 0.0;
static constexpr double kSceneInitialRollDegrees = 0.0;

static Map* createMap(QObject* parent)
{
  if (!ArcGISRuntimeEnvironment::apiKey().isEmpty())
  {
    return new Map(BasemapStyle::ArcGISStreets, parent);
  }

  auto* layer = new ArcGISTiledLayer(kPublicBasemapUrl, parent);
  return new Map(new Basemap(layer, parent), parent);
}

static Scene* createScene(QObject* parent)
{
  if (!ArcGISRuntimeEnvironment::apiKey().isEmpty())
  {
    return new Scene(BasemapStyle::ArcGISImagery, parent);
  }

  auto* layer = new ArcGISTiledLayer(kPublicBasemapUrl, parent);
  return new Scene(new Basemap(layer, parent), parent);
}

static QString extractStylxToDisk()
{
  const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  QDir().mkpath(dir);
  QString out = dir + "/mil2525c.stylx";
  if (QFile::exists(out))
  {
    QFile::remove(out);
  }
  if (!QFile::copy(kStylxResource, out))
  {
    qWarning() << "Failed to extract stylx to" << out;
    return {};
  }
  QFile::setPermissions(out, QFile::ReadOwner | QFile::WriteOwner);
  return out;
}

GeoSwarm::GeoSwarm(QObject* parent /* = nullptr */) :
  QObject(parent),
  m_map(createMap(this)),
  m_scene(createScene(this)),
  m_graphics(new GraphicsEntityController(this)),
  m_generator(new GenerationController(this))
{
  const Envelope worldExtent(Point(-180.0, -60.0, SpatialReference::wgs84()), Point(180.0, 70.0, SpatialReference::wgs84()));
  m_map->setInitialViewpoint(Viewpoint(worldExtent));

  if (!ArcGISRuntimeEnvironment::apiKey().isEmpty())
  {
    auto* surface = new Surface(this);
    surface->elevationSources()->append(new ArcGISTiledElevationSource(kWorldElevationUrl, this));
    m_scene->setBaseSurface(surface);
  }

  setupRenderers();

  for (int dim = 0; dim < DimCount; ++dim)
  {
    const auto dimIndex = static_cast<std::size_t>(dim);
    auto* dot = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Circle, QColor(255, 0, 0), 5.0F, this);
    m_simpleRenderers.at(dimIndex) = new SimpleRenderer(dot, this);
    dot->setParent(m_simpleRenderers.at(dimIndex));
    if (m_dictStyle)
    {
      m_dictRenderers.at(dimIndex) = new DictionaryRenderer(m_dictStyle, this);
    }
  }
  applyRenderer();

  m_graphics->buildPool(m_generator->initialObservations(), m_locationOnly);
  syncApplyConfig();

  connect(m_generator, &GenerationController::randomizeChanged, this, &GeoSwarm::syncApplyConfig);
  connect(m_generator, &GenerationController::aboutToStart, this, &GeoSwarm::rebuildPool);

  connect(m_generator->worker(), &GenerationWorker::observationsReady, m_graphics, &GraphicsEntityController::applyAll, Qt::DirectConnection);

  // Creates the status timers once; QObject parenting owns them for this instance's lifetime.
  initializeStatusTimers();
}

GeoSwarm::~GeoSwarm()
{
  if (m_generator)
  {
    QObject::disconnect(m_generator->worker(), nullptr, m_graphics, nullptr);
    m_generator->shutdown();
  }
  removeOverlaysFromViews();
}

int GeoSwarm::entityCount() const
{
  return m_generator ? m_generator->entityCount() : 0;
}

void GeoSwarm::setupRenderers()
{
  const QString stylxPath = extractStylxToDisk();
  m_dictStyle = DictionarySymbolStyle::createFromFile(stylxPath, this);
  if (!m_dictStyle)
  {
    qWarning() << "DictionarySymbolStyle failed to load from" << stylxPath;
  }
}

void GeoSwarm::applyRenderer()
{
  for (int dim = 0; dim < DimCount; ++dim)
  {
    const auto dimIndex = static_cast<std::size_t>(dim);
    GraphicsOverlay* overlay = m_graphics->overlay(dim);
    if (!overlay)
    {
      continue;
    }
    Renderer* renderer = (m_useDictionarySymbols && m_dictRenderers.at(dimIndex)) ? static_cast<Renderer*>(m_dictRenderers.at(dimIndex)) :
                                                                                    static_cast<Renderer*>(m_simpleRenderers.at(dimIndex));
    overlay->setRenderer(renderer);
  }
}

void GeoSwarm::syncApplyConfig()
{
  m_graphics->setApplyAttributes(m_useDictionarySymbols && !m_locationOnly);
  m_graphics->setLiveMask(m_generator->randomize() ? ObsFlags::kAll : ObsFlags::DIRECTION);
}

void GeoSwarm::rebuildPool()
{
  m_graphics->buildPool(m_generator->initialObservations(), m_locationOnly);
}

void GeoSwarm::moveOverlaysToView(bool toScene)
{
  GraphicsOverlayListModel* targetOverlays =
    toScene ? (m_sceneView ? m_sceneView->graphicsOverlays() : nullptr) : (m_mapView ? m_mapView->graphicsOverlays() : nullptr);
  for (int dim = 0; dim < DimCount; ++dim)
  {
    GraphicsOverlay* overlay = m_graphics->overlay(dim);
    if (!overlay)
    {
      continue;
    }
    if (m_mapView)
    {
      m_mapView->graphicsOverlays()->removeOne(overlay);
    }
    if (m_sceneView)
    {
      m_sceneView->graphicsOverlays()->removeOne(overlay);
    }
    if (targetOverlays)
    {
      targetOverlays->append(overlay);
    }
    m_graphics->setDimensionVisible(dim, m_layerVisible.at(static_cast<std::size_t>(dim)), toScene);
  }
}

void GeoSwarm::removeOverlaysFromViews()
{
  for (int dim = 0; dim < DimCount; ++dim)
  {
    GraphicsOverlay* overlay = m_graphics->overlay(dim);
    if (m_mapView)
    {
      m_mapView->graphicsOverlays()->removeOne(overlay);
    }
    if (m_sceneView)
    {
      m_sceneView->graphicsOverlays()->removeOne(overlay);
    }
  }
}

void GeoSwarm::clearGraphics()
{
  m_graphics->clearPool();
}

void GeoSwarm::setLayerVisible(int dim, bool visible)
{
  if (dim < 0 || dim >= DimCount)
  {
    return;
  }
  m_layerVisible.at(static_cast<std::size_t>(dim)) = visible;
  m_graphics->setDimensionVisible(dim, visible, m_sceneActive);
}

bool GeoSwarm::layerVisible(int dim) const
{
  if (dim < 0 || dim >= DimCount)
  {
    return false;
  }
  return m_layerVisible.at(static_cast<std::size_t>(dim));
}

void GeoSwarm::setUseDictionarySymbols(bool useDictionarySymbols)
{
  if (useDictionarySymbols == m_useDictionarySymbols)
  {
    return;
  }
  m_useDictionarySymbols = useDictionarySymbols;
  applyRenderer();
  syncApplyConfig();
  emit useDictionarySymbolsChanged();
}

void GeoSwarm::setLocationOnly(bool locationOnly)
{
  if (locationOnly == m_locationOnly)
  {
    return;
  }
  m_locationOnly = locationOnly;
  m_graphics->buildPool(m_generator->initialObservations(), m_locationOnly);
  syncApplyConfig();
  emit locationOnlyChanged();
}

void GeoSwarm::initializeStatusTimers()
{
  auto* statusTimer = new QTimer(this);
  connect(statusTimer, &QTimer::timeout, this, &GeoSwarm::statusChanged);
  statusTimer->start(100);

  auto* rateTimer = new QTimer(this);
  m_lastAppliedSample = m_graphics->appliedCount();
  m_rateSampleTimer.start();
  connect(rateTimer, &QTimer::timeout, this, [this]()
  {
    const qint64 elapsedMs = m_rateSampleTimer.restart();
    const quint64 applied = m_graphics->appliedCount();
    const quint64 appliedDelta = applied - m_lastAppliedSample;
    m_lastAppliedSample = applied;
    m_obsPerSec = elapsedMs > 0 ? static_cast<double>(appliedDelta) * 1000.0 / static_cast<double>(elapsedMs) : 0.0;
    const int liveEntities = entityCount();
    m_entityIntervalMs = (liveEntities > 0 && m_obsPerSec > 0.0) ? 1000.0 * static_cast<double>(liveEntities) / m_obsPerSec : 0.0;
  });
  rateTimer->start(1000);
}

void GeoSwarm::setMapView(MapQuickView* mapView)
{
  if (!mapView || mapView == m_mapView)
  {
    return;
  }

  m_mapView = mapView;
  connect(m_mapView, &QObject::destroyed, this, [this]()
  {
    m_mapView = nullptr;
  });
  m_mapView->setMap(m_map);
  moveOverlaysToView(m_sceneActive);

  emit mapViewChanged();
}

void GeoSwarm::setSceneActive(bool sceneActive)
{
  if (sceneActive == m_sceneActive)
  {
    return;
  }
  m_sceneActive = sceneActive;
  moveOverlaysToView(m_sceneActive);
  emit sceneActiveChanged();
}

void GeoSwarm::setSceneView(SceneQuickView* sceneView)
{
  if (!sceneView || sceneView == m_sceneView)
  {
    return;
  }

  m_sceneView = sceneView;
  connect(m_sceneView, &QObject::destroyed, this, [this]()
  {
    m_sceneView = nullptr;
  });
  m_sceneView->setArcGISScene(m_scene);

  const Point cameraLocation(0.0, 20.0, kSceneInitialAltitudeMeters, SpatialReference::wgs84());
  const Camera camera(cameraLocation, kSceneInitialHeadingDegrees, kSceneInitialPitchDegrees, kSceneInitialRollDegrees);
  m_sceneView->setViewpointCameraAsync(camera);

  moveOverlaysToView(m_sceneActive);

  emit sceneViewChanged();
}
