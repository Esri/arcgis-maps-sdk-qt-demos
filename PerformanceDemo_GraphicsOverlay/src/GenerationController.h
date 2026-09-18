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

#ifndef GEOSWARM_GENERATIONCONTROLLER_H
#define GEOSWARM_GENERATIONCONTROLLER_H

#include "GenerationWorker.h"

#include <QObject>

#include <array>
#include <vector>

class QThread;

// QML-facing facade for the in-process generator. Owns a single worker on its
// own thread and exposes its config + stats as Q_PROPERTYs.
class GenerationController : public QObject
{
  Q_OBJECT

  Q_PROPERTY(int entityCount MEMBER m_entityCount WRITE setEntityCount NOTIFY entityCountChanged)
  Q_PROPERTY(int targetObsPerSec MEMBER m_targetObsPerSec WRITE setTargetObsPerSec NOTIFY targetObsPerSecChanged)
  Q_PROPERTY(quint16 flags MEMBER m_flags WRITE setFlags NOTIFY flagsChanged)
  Q_PROPERTY(bool randomize MEMBER m_randomize WRITE setRandomize NOTIFY randomizeChanged)
  Q_PROPERTY(double headingJitter MEMBER m_headingJitter WRITE setHeadingJitter NOTIFY headingJitterChanged)
  Q_PROPERTY(double motionScale MEMBER m_motionScale WRITE setMotionScale NOTIFY motionScaleChanged)
  Q_PROPERTY(int dimWeightAir MEMBER m_dimWeightAir WRITE setDimWeightAir NOTIFY dimWeightAirChanged)
  Q_PROPERTY(int dimWeightGround MEMBER m_dimWeightGround WRITE setDimWeightGround NOTIFY dimWeightGroundChanged)
  Q_PROPERTY(int dimWeightSea MEMBER m_dimWeightSea WRITE setDimWeightSea NOTIFY dimWeightSeaChanged)
  Q_PROPERTY(int dimWeightSub MEMBER m_dimWeightSub WRITE setDimWeightSub NOTIFY dimWeightSubChanged)

  Q_PROPERTY(bool running MEMBER m_running NOTIFY runningChanged)

  Q_PROPERTY(double obsPerSec READ obsPerSec NOTIFY statsChanged)
  Q_PROPERTY(double passesPerSecActual READ passesPerSecActual NOTIFY statsChanged)
  Q_PROPERTY(double generateUs READ generateUs NOTIFY statsChanged)
  Q_PROPERTY(double publishUs READ publishUs NOTIFY statsChanged)

public:
  explicit GenerationController(QObject* parent = nullptr);
  ~GenerationController() override;

  GenerationWorker* worker() const;

  // Deterministic snapshot of the starting entities (full attributes), bucketed by battle dimension
  std::array<std::vector<Observation>, kDimCount> initialObservations() const;

  int entityCount() const;
  bool randomize() const;

  double obsPerSec() const;
  double passesPerSecActual() const;
  double generateUs() const;
  double publishUs() const;

  void setEntityCount(int count);
  void setTargetObsPerSec(int obsPerSec);
  void setFlags(quint16 flags);
  void setRandomize(bool randomize);
  void setHeadingJitter(double jitter);
  void setMotionScale(double scale);
  void setDimWeightAir(int weight);
  void setDimWeightGround(int weight);
  void setDimWeightSea(int weight);
  void setDimWeightSub(int weight);

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  void shutdown();

signals:
  void entityCountChanged();
  void targetObsPerSecChanged();
  void flagsChanged();
  void randomizeChanged();
  void headingJitterChanged();
  void motionScaleChanged();
  void dimWeightAirChanged();
  void dimWeightGroundChanged();
  void dimWeightSeaChanged();
  void dimWeightSubChanged();
  void runningChanged();
  void statsChanged();
  void aboutToStart();

private slots:
  void onWorkerRunningChanged(bool running);
  void onWorkerStats(GenStats stats);

private:
  EntityGenerator::Config currentGenConfig() const;

  int m_entityCount = 10000;
  int m_targetObsPerSec = 100000;
  quint16 m_flags = 0;
  bool m_randomize = false;
  double m_headingJitter = 0.05;
  double m_motionScale = 200.0;
  int m_dimWeightAir = 1;
  int m_dimWeightGround = 1;
  int m_dimWeightSea = 1;
  int m_dimWeightSub = 1;
  bool m_running = false;

  GenStats m_stats;
  QThread* m_workerThread = nullptr;
  GenerationWorker* m_worker = nullptr;
  bool m_shutdown = false;
};

#endif
