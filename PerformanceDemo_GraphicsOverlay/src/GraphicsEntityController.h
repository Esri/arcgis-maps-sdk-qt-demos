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

#ifndef GEOSWARM_GRAPHICSENTITYCONTROLLER_H
#define GEOSWARM_GRAPHICSENTITYCONTROLLER_H

#include "EntityGenerator.h"
#include "Observation.h"

#include <QAtomicInteger>
#include <QHash>
#include <QMutex>
#include <QObject>
#include <QVector>

#include <array>
#include <vector>

namespace Esri::ArcGISRuntime
{
  class Graphic;
  class GraphicsOverlay;
  class SpatialReference;
} // namespace Esri::ArcGISRuntime

// Owns one GraphicsOverlay per battle dimension
class GraphicsEntityController : public QObject
{
  Q_OBJECT
public:
  explicit GraphicsEntityController(QObject* parent = nullptr);
  ~GraphicsEntityController() override;

  Esri::ArcGISRuntime::GraphicsOverlay* overlay(int dim) const;
  quint64 appliedCount() const;

  void buildPool(const std::array<std::vector<Observation>, kDimCount>& initial, bool locationOnly);
  void clearPool();

  void setApplyAttributes(bool applyAttributes);
  void setLiveMask(quint16 liveMask);
  void setDimensionVisible(int dim, bool visible, bool sceneActive);

public slots:
  void applyAll(const std::array<std::vector<Observation>, kDimCount>& buckets);

private:
  void applyDim(int dim, const std::vector<Observation>& batch, const Esri::ArcGISRuntime::SpatialReference& spatialReference);

  // Detaches and deletes one dimension's graphics. Caller must hold m_poolMutex.
  void releaseDim(std::size_t dimIndex);

  std::array<Esri::ArcGISRuntime::GraphicsOverlay*, kDimCount> m_overlays{};
  std::array<QVector<Esri::ArcGISRuntime::Graphic*>, kDimCount> m_pools{};
  std::array<QHash<QString, int>, kDimCount> m_idToIndex{};
  bool m_applyAttributes = false;
  bool m_locationOnly = false;
  quint16 m_liveMask = ObsFlags::DIRECTION;

  QMutex m_poolMutex;
  QAtomicInteger<quint64> m_appliedCount = 0;
};

#endif // GEOSWARM_GRAPHICSENTITYCONTROLLER_H
