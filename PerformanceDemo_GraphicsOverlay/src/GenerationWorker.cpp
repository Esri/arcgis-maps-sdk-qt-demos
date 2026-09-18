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

#include "GenerationWorker.h"

#include "EntityObservationAdapter.h"

#include <array>
#include <chrono>
#include <thread>

using namespace std::chrono;

GenerationWorker::GenerationWorker(QObject* parent) :
  QObject(parent)
{
  qRegisterMetaType<Observation>("Observation");
  qRegisterMetaType<GenStats>();
  qRegisterMetaType<GenerationWorker::StartConfig>();
}

GenerationWorker::~GenerationWorker() = default;

quint64 GenerationWorker::prepareStart()
{
  return m_requestId.fetch_add(1, std::memory_order_acq_rel) + 1;
}

void GenerationWorker::stop()
{
  m_requestId.fetch_add(1, std::memory_order_acq_rel);
  m_running.store(false, std::memory_order_release);
}

void GenerationWorker::start(const GenerationWorker::StartConfig& cfg)
{
  if (cfg.requestId != m_requestId.load(std::memory_order_acquire))
  {
    return;
  }
  if (m_running.exchange(true))
  {
    return;
  }
  if (cfg.requestId != m_requestId.load(std::memory_order_acquire))
  {
    m_running.store(false, std::memory_order_release);
    return;
  }
  emit runningChanged(true);
  runLoop(cfg);
  emit runningChanged(false);
}

void GenerationWorker::runLoop(const GenerationWorker::StartConfig& cfg)
{
  EntityGenerator gen(cfg.gen);

  const int entityCount = cfg.gen.entityCount;
  // One pass updates all entities once, so passesPerSec * entityCount = obsPerSec.
  const double passesPerSecTarget = static_cast<double>(cfg.targetObsPerSec) / static_cast<double>(entityCount);
  // Wall-clock spacing between two consecutive all-entities passes.
  const auto passInterval = duration_cast<steady_clock::duration>(duration<double>(1.0 / passesPerSecTarget));
  // Simulation time advanced by one pass over all entities.
  const double dtSec = 1.0 / passesPerSecTarget;

  constexpr auto kCoarseSleepChunk = milliseconds(50);

  auto waitUntilOrStop = [this, kCoarseSleepChunk](steady_clock::time_point target)
  {
    while (m_running.load())
    {
      const auto now = steady_clock::now();
      if (now >= target)
      {
        return;
      }
      const auto remaining = target - now;
      if (remaining > kCoarseSleepChunk)
      {
        std::this_thread::sleep_for(kCoarseSleepChunk);
      }
      else
      {
        std::this_thread::sleep_until(target);
        return;
      }
    }
  };

  const quint16 flags = cfg.flags;

  // One bucket per battle dimension, clear()'d each gen tick to retain capacity
  // (zero allocations in steady state). Reserve for the all-in-one-dim case.
  ObservationBuckets buckets;
  for (auto& bucket : buckets)
  {
    bucket.reserve(entityCount);
  }

  quint64 cumObs = 0;
  qint64 cumGenNs = 0;
  qint64 cumPubNs = 0;
  int genTicksThisSec = 0;

  auto statsClock = steady_clock::now();
  auto nextDeadline = steady_clock::now();

  while (m_running.load())
  {
    nextDeadline += passInterval;

    {
      // Measure state updates separately from building and emitting observations.
      const auto t0 = steady_clock::now();
      gen.advanceEntities(dtSec);
      const auto t1 = steady_clock::now();
      cumGenNs += duration_cast<nanoseconds>(t1 - t0).count();

      const auto& entities = gen.entities();
      const int total = static_cast<int>(entities.size());

      for (auto& bucket : buckets)
      {
        bucket.clear();
      }

      for (const auto& entity : entities)
      {
        buckets.at(static_cast<std::size_t>(entity.dim)).push_back(observationFromEntity(entity, flags));
      }

      emit observationsReady(buckets);

      const auto t2 = steady_clock::now();
      // Publish time covers building Observation payloads, bucketing, and emitting batches.
      cumPubNs += duration_cast<nanoseconds>(t2 - t1).count();
      // One pass produces one observation per entity.
      cumObs += static_cast<quint64>(total);
      ++genTicksThisSec;
    }

    if (steady_clock::now() - statsClock >= seconds(1))
    {
      const double elapsedSec = duration_cast<duration<double>>(steady_clock::now() - statsClock).count();
      // Once per second, report achieved obs/sec, achieved passes/sec, and
      // average generate/publish microseconds for one pass over all entities.
      GenStats stats;
      stats.obsPerSec = static_cast<double>(cumObs) / elapsedSec;
      stats.passesPerSecActual = static_cast<double>(genTicksThisSec) / elapsedSec;
      stats.generateUs = genTicksThisSec > 0 ? static_cast<double>(cumGenNs) / genTicksThisSec / 1000.0 : 0.0;
      stats.publishUs = genTicksThisSec > 0 ? static_cast<double>(cumPubNs) / genTicksThisSec / 1000.0 : 0.0;
      emit statsUpdated(stats);

      cumObs = 0;
      cumGenNs = 0;
      cumPubNs = 0;
      genTicksThisSec = 0;
      statsClock = steady_clock::now();
    }

    if (nextDeadline > steady_clock::now())
    {
      waitUntilOrStop(nextDeadline);
    }
    else
    {
      nextDeadline = steady_clock::now();
    }
  }
}
