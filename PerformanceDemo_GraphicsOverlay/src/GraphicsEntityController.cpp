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

#include "GraphicsEntityController.h"

#include "AttributeListModel.h"
#include "Graphic.h"
#include "GraphicListModel.h"
#include "GraphicsOverlay.h"
#include "MapTypes.h"
#include "Point.h"
#include "SpatialReference.h"

#include <QFuture>
#include <QList>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QtConcurrentRun>

#include <utility>

using namespace Esri::ArcGISRuntime;

// Attribute keys the mil-2525c DictionarySymbolStyle reads; echelon maps to "echelonmobility".
static const QString kKeyId = QStringLiteral("id");
static const QString kKeySidc = QStringLiteral("sidc");
static const QString kKeyDirection = QStringLiteral("direction");
static const QString kKeySpeed = QStringLiteral("speed");
static const QString kKeyStatus = QStringLiteral("status");
static const QString kKeyEchelonMobility = QStringLiteral("echelonmobility");
static const QString kKeyHqtffd = QStringLiteral("hqtffd");
static const QString kKeyUniqueDesignation = QStringLiteral("uniquedesignation");
static const QString kKeyAdditionalInformation = QStringLiteral("additionalinformation");
static const QString kKeyQuantity = QStringLiteral("quantity");
static const QString kKeyCombatEffectiveness = QStringLiteral("combateffectiveness");
static const QString kKeyStaffComment = QStringLiteral("staffcomment");
static const QString kKeyHigherFormation = QStringLiteral("higherformation");
static const QString kKeyCountryLabel = QStringLiteral("countrylabel");
static const QString kKeyReinforced = QStringLiteral("reinforced");
static const QString kKeyDateTimeValid = QStringLiteral("datetimevalid");

static QString fixedTwoDecimalText(double value)
{
  return QString::number(value, 'f', 2);
}

template<typename Sink>
static void putAttributes(Sink&& put, const Observation& observation, bool locationOnly, bool includeId = true, quint16 liveMask = ObsFlags::kAll)
{
  const quint16 flags = observation.presentFlags & liveMask;
  if (includeId)
  {
    put(kKeyId, observation.id);
  }
  if (flags & ObsFlags::SIDC)
  {
    put(kKeySidc, observation.sidc);
  }
  if (locationOnly)
  {
    return;
  }
  if (flags & ObsFlags::DIRECTION)
  {
    put(kKeyDirection, fixedTwoDecimalText(observation.direction));
  }
  if (flags & ObsFlags::SPEED)
  {
    put(kKeySpeed, fixedTwoDecimalText(observation.speed));
  }
  if (flags & ObsFlags::STATUS)
  {
    put(kKeyStatus, observation.status);
  }
  if (flags & ObsFlags::ECHELON)
  {
    put(kKeyEchelonMobility, observation.echelon);
  }
  if (flags & ObsFlags::HQTFFD)
  {
    put(kKeyHqtffd, observation.hqtffd);
  }
  if (flags & ObsFlags::UNIQUEDESIG)
  {
    put(kKeyUniqueDesignation, observation.uniquedesignation);
  }
  if (flags & ObsFlags::ADDINFO)
  {
    put(kKeyAdditionalInformation, observation.additionalinformation);
  }
  if (flags & ObsFlags::QUANTITY)
  {
    put(kKeyQuantity, observation.quantity);
  }
  if (flags & ObsFlags::COMBATEFF)
  {
    put(kKeyCombatEffectiveness, observation.combateffectiveness);
  }
  if (flags & ObsFlags::STAFFCOMMENT)
  {
    put(kKeyStaffComment, observation.staffcomment);
  }
  if (flags & ObsFlags::HIGHERFORM)
  {
    put(kKeyHigherFormation, observation.higherformation);
  }
  if (flags & ObsFlags::COUNTRY)
  {
    put(kKeyCountryLabel, observation.countrylabel);
  }
  if (flags & ObsFlags::REINFORCED)
  {
    put(kKeyReinforced, observation.reinforced);
  }
  if (flags & ObsFlags::DATETIME)
  {
    put(kKeyDateTimeValid, fixedTwoDecimalText(observation.datetimevalid));
  }
}

GraphicsEntityController::GraphicsEntityController(QObject* parent) :
  QObject(parent)
{
  for (int dim = 0; dim < kDimCount; ++dim)
  {
    const auto dimIndex = static_cast<std::size_t>(dim);
    m_overlays.at(dimIndex) = new GraphicsOverlay(this);
    m_overlays.at(dimIndex)->setRenderingMode(GraphicsRenderingMode::Dynamic);
  }
}

GraphicsEntityController::~GraphicsEntityController() = default;

// Returns a non-owning pointer valid until this controller is destroyed.
GraphicsOverlay* GraphicsEntityController::overlay(int dim) const
{
  if (dim < 0 || dim >= kDimCount)
  {
    return nullptr;
  }
  return m_overlays.at(static_cast<std::size_t>(dim));
}

quint64 GraphicsEntityController::appliedCount() const
{
  return m_appliedCount.loadRelaxed();
}

void GraphicsEntityController::releaseDim(std::size_t dimIndex)
{
  m_overlays.at(dimIndex)->graphics()->clear();
  const QVector<Graphic*>& pool = m_pools.at(dimIndex);
  for (Graphic* graphic : pool)
  {
    graphic->deleteLater();
  }
  m_pools.at(dimIndex).clear();
  m_idToIndex.at(dimIndex).clear();
}

void GraphicsEntityController::clearPool()
{
  QMutexLocker locker(&m_poolMutex);
  for (int dim = 0; dim < kDimCount; ++dim)
  {
    releaseDim(static_cast<std::size_t>(dim));
  }
}

void GraphicsEntityController::buildPool(const std::array<std::vector<Observation>, kDimCount>& initial, bool locationOnly)
{
  QMutexLocker locker(&m_poolMutex);
  m_locationOnly = locationOnly;

  for (int dim = 0; dim < kDimCount; ++dim)
  {
    const auto dimIndex = static_cast<std::size_t>(dim);
    releaseDim(dimIndex);

    const std::vector<Observation>& observations = initial.at(dimIndex);
    const int entityCount = static_cast<int>(observations.size());

    QVector<Graphic*>& pool = m_pools.at(dimIndex);
    QHash<QString, int>& idToIndex = m_idToIndex.at(dimIndex);
    pool.reserve(entityCount);
    idToIndex.reserve(entityCount);

    QList<Graphic*> created;
    created.reserve(entityCount);
    for (int index = 0; index < entityCount; ++index)
    {
      const Observation& observation = observations.at(index);

      QVariantMap attributes;
      putAttributes([&attributes](const QString& name, const QVariant& value)
      {
        attributes.insert(name, value);
      }, observation, locationOnly);

      auto* graphic = new Graphic(attributes, m_overlays.at(dimIndex));
      created.append(graphic);
      pool.append(graphic);
      idToIndex.insert(observation.id, index);
    }
    m_overlays.at(dimIndex)->graphics()->append(created);
  }
}

void GraphicsEntityController::setApplyAttributes(bool applyAttributes)
{
  // applyDim reads this on pool threads under applyAll's lock; take it here too.
  QMutexLocker locker(&m_poolMutex);
  m_applyAttributes = applyAttributes;
}

void GraphicsEntityController::setLiveMask(quint16 liveMask)
{
  QMutexLocker locker(&m_poolMutex);
  m_liveMask = liveMask;
}

void GraphicsEntityController::setDimensionVisible(int dim, bool visible, bool sceneActive)
{
  if (dim < 0 || dim >= kDimCount)
  {
    return;
  }

  QMutexLocker locker(&m_poolMutex);
  const auto dimIndex = static_cast<std::size_t>(dim);
  GraphicsOverlay* overlay = m_overlays.at(dimIndex);

  if (sceneActive)
  {
    overlay->setVisible(true);
    for (Graphic* graphic : m_pools.at(dimIndex))
    {
      graphic->setVisible(visible);
    }
  }
  else
  {
    for (Graphic* graphic : m_pools.at(dimIndex))
    {
      graphic->setVisible(true);
    }
    overlay->setVisible(visible);
  }
}

void GraphicsEntityController::applyDim(int dim, const std::vector<Observation>& batch, const SpatialReference& spatialReference)
{
  const auto dimIndex = static_cast<std::size_t>(dim);
  const QVector<Graphic*>& pool = m_pools.at(dimIndex);
  const QHash<QString, int>& idToIndex = m_idToIndex.at(dimIndex);
  quint64 appliedCount = 0;
  for (const Observation& observation : batch)
  {
    const auto found = idToIndex.constFind(observation.id);
    if (found == idToIndex.constEnd())
    {
      continue;
    }
    Graphic* graphic = pool.at(found.value());

    const Point point(observation.longitude, observation.latitude, observation.altitude, spatialReference);
    graphic->setGeometry(point);

    if (m_applyAttributes)
    {
      AttributeListModel* attributes = graphic->attributes();
      putAttributes([attributes](const QString& name, const QVariant& value)
      {
        attributes->replaceAttribute(name, value);
      }, observation, m_locationOnly, false, m_liveMask);
    }
    ++appliedCount;
  }
  m_appliedCount.fetchAndAddRelaxed(appliedCount);
}

// Applies all dimension batches in parallel and blocks until every batch is complete.
void GraphicsEntityController::applyAll(const std::array<std::vector<Observation>, kDimCount>& buckets)
{
  QMutexLocker locker(&m_poolMutex);
  const SpatialReference wgs84 = SpatialReference::wgs84();

  QVector<QFuture<void>> futures;
  futures.reserve(kDimCount);
  for (int dim = 0; dim < kDimCount; ++dim)
  {
    const std::vector<Observation>& batch = buckets.at(static_cast<std::size_t>(dim));
    if (!batch.empty())
    {
      futures.append(QtConcurrent::run([this, dim, &buckets, wgs84]()
      {
        applyDim(dim, buckets.at(static_cast<std::size_t>(dim)), wgs84);
      }));
    }
  }
  for (QFuture<void>& future : futures)
  {
    future.waitForFinished();
  }
}
