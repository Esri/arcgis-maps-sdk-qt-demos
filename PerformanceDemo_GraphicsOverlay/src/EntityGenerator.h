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

#ifndef GEOSWARM_ENTITYGENERATOR_H
#define GEOSWARM_ENTITYGENERATOR_H

#include <QString>

#include <array>
#include <cstdint>
#include <random>
#include <vector>

// Battle-dimension count (air, ground, sea, sub); matches GeoSwarm::Dim.
inline constexpr int kDimCount = 4;

// Per-entity state. Payload fields are kept in Observation form and built at
// roll time, so the publish loop only copies them.
struct Entity
{
  // Simulation state, advanced each gen tick.
  double lon = 0.0;
  double lat = 0.0;
  float headingRad = 0.0F;
  float speedMps = 0.0F;

  // Battle-dimension index (0-3).
  int dim = 1;

  // Payload, in Observation representation.
  QString id;
  QString sidc;
  double altitude = 0.0;

  // Degrees, refreshed each gen tick from headingRad.
  double direction = 0.0;

  QString status;
  QString echelon;
  QString hqtffd;
  QString uniquedesignation;
  QString additionalinformation;
  quint32 quantity = 0;
  QString combateffectiveness;
  QString staffcomment;
  QString higherformation;
  QString countrylabel;
  QString reinforced;

  // Epoch seconds.
  double datetimevalid = 0.0;
};

class EntityGenerator
{
public:
  struct Config
  {
    int entityCount = 10000;

    // First id index; ids are formatted "e0000001", "e0000002", ...
    int idOffset = 0;

    // Spawn bounds; latitude kept short of the poles.
    double bboxLonMin = -180.0;
    double bboxLonMax = 180.0;
    double bboxLatMin = -60.0;
    double bboxLatMax = 70.0;

    double altDefault = 0.0;
    float speedMps = 5.0F;
    float headingJitterRadPerGenTick = 0.05F;

    // Visual speed multiplier; 0 = frozen.
    double motionScale = 200.0;

    uint32_t seed = 1;

    // Re-roll every field except id each gen tick.
    bool randomizePerGenTick = false;

    // Battle-dimension mix (auto-normalized; 0 excludes a dimension).
    int dimWeightAir = 1;
    int dimWeightGround = 1;
    int dimWeightSea = 1;
    int dimWeightSub = 1;
  };

  explicit EntityGenerator(Config cfg);

  void advanceEntities(double dtSeconds);

  const std::vector<Entity>& entities() const;

private:
  void rollIdentity(Entity& entity);
  void rollModifiers(Entity& entity);

  Config m_cfg;
  std::vector<Entity> m_entities;
  std::mt19937 m_rng;

  // Palette SIDC indices grouped by dimension.
  std::array<std::vector<int>, kDimCount> m_dimBuckets;
  std::discrete_distribution<int> m_dimDist;
};

#endif // GEOSWARM_ENTITYGENERATOR_H
