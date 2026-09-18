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

#ifndef GEOSWARM_GENERATIONWORKER_H
#define GEOSWARM_GENERATIONWORKER_H

#include "EntityGenerator.h"
#include "Observation.h"

#include <QObject>

#include <array>
#include <atomic>
#include <vector>

// All dimensions' observation buckets for one gen tick, indexed by dim.
using ObservationBuckets = std::array<std::vector<Observation>, kDimCount>;

struct GenStats
{
  double obsPerSec = 0.0;
  double passesPerSecActual = 0.0;
  double generateUs = 0.0;
  double publishUs = 0.0;
};
Q_DECLARE_METATYPE(GenStats)

// Generates entities and emits Observations to the per-dimension data sources.
class GenerationWorker : public QObject
{
  Q_OBJECT
public:
  struct StartConfig
  {
    EntityGenerator::Config gen;
    int targetObsPerSec = 100000;
    quint64 requestId = 0;

    // ObsFlags bits selecting which optional fields to populate.
    quint16 flags = 0;
  };

  explicit GenerationWorker(QObject* parent = nullptr);
  ~GenerationWorker() override;

  quint64 prepareStart();
  void stop();

public slots:
  void start(const GenerationWorker::StartConfig& cfg);

signals:
  void statsUpdated(GenStats stats);
  void runningChanged(bool running);
  void observationsReady(const ObservationBuckets& buckets);

private:
  void runLoop(const GenerationWorker::StartConfig& cfg);

  std::atomic<bool> m_running{false};
  std::atomic<quint64> m_requestId{0};
};

Q_DECLARE_METATYPE(GenerationWorker::StartConfig)

#endif
