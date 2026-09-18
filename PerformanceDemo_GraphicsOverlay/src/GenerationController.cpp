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

#include "EntityObservationAdapter.h"
#include "Observation.h"

#include <algorithm>
#include <utility>
#include <QString>
#include <QThread>

GenerationController::GenerationController(QObject* parent) :
  QObject(parent),
  m_flags(ObsFlags::SIDC | ObsFlags::ALT | ObsFlags::DIRECTION | ObsFlags::SPEED),
  m_workerThread(new QThread(this)),
  m_worker(new GenerationWorker())
{
  m_workerThread->setObjectName(QStringLiteral("gen"));
  m_worker->moveToThread(m_workerThread);
  connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
  connect(m_worker, &GenerationWorker::runningChanged, this, &GenerationController::onWorkerRunningChanged);
  connect(m_worker, &GenerationWorker::statsUpdated, this, &GenerationController::onWorkerStats);
  m_workerThread->start();
}

GenerationController::~GenerationController()
{
  shutdown();
}

GenerationWorker* GenerationController::worker() const
{
  return m_worker;
}

EntityGenerator::Config GenerationController::currentGenConfig() const
{
  EntityGenerator::Config cfg;
  cfg.entityCount = m_entityCount;
  cfg.idOffset = 0;
  cfg.randomizePerGenTick = m_randomize;
  cfg.headingJitterRadPerGenTick = static_cast<float>(m_headingJitter);
  cfg.motionScale = m_motionScale;
  cfg.dimWeightAir = m_dimWeightAir;
  cfg.dimWeightGround = m_dimWeightGround;
  cfg.dimWeightSea = m_dimWeightSea;
  cfg.dimWeightSub = m_dimWeightSub;
  cfg.seed = 1U;
  return cfg;
}

std::array<std::vector<Observation>, kDimCount> GenerationController::initialObservations() const
{
  EntityGenerator generator(currentGenConfig());
  const std::vector<Entity>& entities = generator.entities();

  std::array<std::vector<Observation>, kDimCount> result;
  for (const Entity& entity : entities)
  {
    result.at(static_cast<std::size_t>(entity.dim)).push_back(observationFromEntity(entity, m_flags));
  }
  return result;
}

int GenerationController::entityCount() const
{
  return m_entityCount;
}

bool GenerationController::randomize() const
{
  return m_randomize;
}

double GenerationController::obsPerSec() const
{
  return m_stats.obsPerSec;
}

double GenerationController::passesPerSecActual() const
{
  return m_stats.passesPerSecActual;
}

double GenerationController::generateUs() const
{
  return m_stats.generateUs;
}

double GenerationController::publishUs() const
{
  return m_stats.publishUs;
}

void GenerationController::setEntityCount(int count)
{
  count = std::max(count, 1);
  if (count != m_entityCount)
  {
    m_entityCount = count;
    emit entityCountChanged();
  }
}

void GenerationController::setTargetObsPerSec(int obsPerSec)
{
  obsPerSec = std::max(obsPerSec, 1);
  if (obsPerSec != m_targetObsPerSec)
  {
    m_targetObsPerSec = obsPerSec;
    emit targetObsPerSecChanged();
  }
}

void GenerationController::setFlags(quint16 flags)
{
  const quint16 masked = flags & ObsFlags::kAll;
  if (masked != m_flags)
  {
    m_flags = masked;
    emit flagsChanged();
  }
}

void GenerationController::setRandomize(bool randomize)
{
  if (randomize != m_randomize)
  {
    m_randomize = randomize;
    emit randomizeChanged();
  }
}

void GenerationController::setHeadingJitter(double jitter)
{
  jitter = std::max(jitter, 0.0);
  if (!qFuzzyCompare(1.0 + jitter, 1.0 + m_headingJitter))
  {
    m_headingJitter = jitter;
    emit headingJitterChanged();
  }
}

void GenerationController::setMotionScale(double scale)
{
  scale = std::max(scale, 0.0);
  if (!qFuzzyCompare(1.0 + scale, 1.0 + m_motionScale))
  {
    m_motionScale = scale;
    emit motionScaleChanged();
  }
}

void GenerationController::setDimWeightAir(int weight)
{
  weight = std::max(weight, 0);
  if (weight != m_dimWeightAir)
  {
    m_dimWeightAir = weight;
    emit dimWeightAirChanged();
  }
}

void GenerationController::setDimWeightGround(int weight)
{
  weight = std::max(weight, 0);
  if (weight != m_dimWeightGround)
  {
    m_dimWeightGround = weight;
    emit dimWeightGroundChanged();
  }
}

void GenerationController::setDimWeightSea(int weight)
{
  weight = std::max(weight, 0);
  if (weight != m_dimWeightSea)
  {
    m_dimWeightSea = weight;
    emit dimWeightSeaChanged();
  }
}

void GenerationController::setDimWeightSub(int weight)
{
  weight = std::max(weight, 0);
  if (weight != m_dimWeightSub)
  {
    m_dimWeightSub = weight;
    emit dimWeightSubChanged();
  }
}

void GenerationController::start()
{
  if (m_running || m_shutdown)
  {
    return;
  }

  emit aboutToStart();

  GenerationWorker::StartConfig cfg;
  cfg.gen = currentGenConfig();
  cfg.targetObsPerSec = m_targetObsPerSec;
  cfg.flags = m_flags;
  cfg.requestId = m_worker->prepareStart();

  QMetaObject::invokeMethod(m_worker, "start", Qt::QueuedConnection, Q_ARG(GenerationWorker::StartConfig, cfg));
}

void GenerationController::stop()
{
  if (m_worker)
  {
    m_worker->stop();
  }
}

void GenerationController::shutdown()
{
  if (m_shutdown)
  {
    return;
  }
  m_shutdown = true;

  stop();
  if (m_workerThread)
  {
    m_workerThread->quit();
    m_workerThread->wait();
  }
  m_worker = nullptr;
}

void GenerationController::onWorkerRunningChanged(bool running)
{
  if (m_running != running)
  {
    m_running = running;
    emit runningChanged();
  }
}

void GenerationController::onWorkerStats(GenStats stats)
{
  m_stats = std::move(stats);
  emit statsChanged();
}
