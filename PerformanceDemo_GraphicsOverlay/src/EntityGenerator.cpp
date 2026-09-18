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

#include "EntityGenerator.h"

#include <array>
#include <chrono>
#include <cmath>
#include <QStringView>

static constexpr int kDimUnknown = -1;

// SIDC position-3 char -> battle-dimension index (0=air,1=ground,2=sea,3=sub).
// Return kDimUnknown for empty or malformed codes so callers can skip them.
static int dimFromSidc(QStringView sidc)
{
  if (sidc.size() < 3)
  {
    return kDimUnknown;
  }

  switch (sidc[2].unicode())
  {
    case u'A':
      return 0;
    case u'G':
      return 1;
    case u'S':
      return 2;
    case u'U':
      return 3;
    default:
      return kDimUnknown;
  }
}

// mil-2525c palette. Each code is exactly 15 chars; the renderer rejects
// anything else. Position 3 is the battle dimension: A air, G ground, S sea,
// U subsurface.
static const std::array<QString, 42> kSidcPalette{{
  // Air -- fighter / bomber / attack / cargo / recon, all 4 affiliations
  QStringLiteral("SFAPMFF--------"),
  QStringLiteral("SFAPMFB--------"),
  QStringLiteral("SFAPMFA--------"),
  QStringLiteral("SFAPMFC--------"),
  QStringLiteral("SFAPMFQ--------"),
  QStringLiteral("SHAPMFF--------"),
  QStringLiteral("SHAPMFB--------"),
  QStringLiteral("SHAPMFC--------"),
  QStringLiteral("SHAPMFA--------"),
  QStringLiteral("SHAPMFQ--------"),
  QStringLiteral("SNAPMFF--------"),
  QStringLiteral("SNAPMFA--------"),
  QStringLiteral("SNAPMFC--------"),
  QStringLiteral("SUAPMFA--------"),
  QStringLiteral("SUAPMFB--------"),
  QStringLiteral("SUAPMFF--------"),
  // Ground combat units -- infantry / recon / air defense / HQ
  QStringLiteral("SFGPUCI--------"),
  QStringLiteral("SFGPUCR--------"),
  QStringLiteral("SFGPUCAA-------"),
  QStringLiteral("SFGPUH---------"),
  QStringLiteral("SHGPUCI--------"),
  QStringLiteral("SHGPUCR--------"),
  QStringLiteral("SHGPUCAA-------"),
  QStringLiteral("SNGPUCI--------"),
  QStringLiteral("SNGPUCAA-------"),
  QStringLiteral("SUGPUCR--------"),
  QStringLiteral("SUGPUCAA-------"),
  // Ground equipment -- armor / tank / mobile sensors
  QStringLiteral("SFGPEVAL-------"),
  QStringLiteral("SFGPEVAT-------"),
  QStringLiteral("SFGPEWMS-------"),
  QStringLiteral("SHGPEVAL-------"),
  QStringLiteral("SHGPEWMS-------"),
  QStringLiteral("SNGPEVAT-------"),
  QStringLiteral("SNGPEWMA-------"),
  // Sea surface combatants
  QStringLiteral("SFSPCLDD-------"),
  QStringLiteral("SFSPCLCC-------"),
  QStringLiteral("SFSPCLFF-------"),
  QStringLiteral("SHSPCLCV-------"),
  QStringLiteral("SUSPCLLL-------"),
  // Subsurface
  QStringLiteral("SFUPWMD--------"),
  QStringLiteral("SHUPWMD--------"),
  QStringLiteral("SNUPWMS--------"),
}};
static const int kSidcPaletteSize = static_cast<int>(kSidcPalette.size());
static_assert(sizeof("SFGPUCI--------") - 1 == 15, "mil-2525c SIDC must be 15 chars");

// NATO phonetic for callsign synthesis.
static const std::array<QString, 26> kPhonetic{
  {QStringLiteral("ALPHA"),   QStringLiteral("BRAVO"),  QStringLiteral("CHARLIE"), QStringLiteral("DELTA"),    QStringLiteral("ECHO"),
   QStringLiteral("FOXTROT"), QStringLiteral("GOLF"),   QStringLiteral("HOTEL"),   QStringLiteral("INDIA"),    QStringLiteral("JULIETT"),
   QStringLiteral("KILO"),    QStringLiteral("LIMA"),   QStringLiteral("MIKE"),    QStringLiteral("NOVEMBER"), QStringLiteral("OSCAR"),
   QStringLiteral("PAPA"),    QStringLiteral("QUEBEC"), QStringLiteral("ROMEO"),   QStringLiteral("SIERRA"),   QStringLiteral("TANGO"),
   QStringLiteral("UNIFORM"), QStringLiteral("VICTOR"), QStringLiteral("WHISKEY"), QStringLiteral("XRAY"),     QStringLiteral("YANKEE"),
   QStringLiteral("ZULU")}};
static const int kPhoneticSize = static_cast<int>(kPhonetic.size());

// SIDC pos-12 echelon codes (MIL-STD-2525C): '-' unspecified, A..I team->division.
static const std::array<QString, 10> kEchelonPalette{{QStringLiteral("-"), QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"),
                                                      QStringLiteral("D"), QStringLiteral("E"), QStringLiteral("F"), QStringLiteral("G"),
                                                      QStringLiteral("H"), QStringLiteral("I")}};
static const int kEchelonPaletteSize = static_cast<int>(kEchelonPalette.size());

// SIDC pos-11 HQ/TF/FD codes.
static const std::array<QString, 8> kHqtffdPalette{{QStringLiteral("-"), QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"),
                                                    QStringLiteral("D"), QStringLiteral("E"), QStringLiteral("F"), QStringLiteral("G")}};
static const int kHqtffdPaletteSize = static_cast<int>(kHqtffdPalette.size());

static const std::array<QString, 3> kStatusPalette{
  {QStringLiteral("P"), QStringLiteral("A"), QStringLiteral("S")}}; // present, anticipated, suspected
static const int kStatusPaletteSize = static_cast<int>(kStatusPalette.size());

static const std::array<QString, 3> kCombatEffPalette{{QStringLiteral("G"), QStringLiteral("Y"), QStringLiteral("R")}};
static const int kCombatEffPaletteSize = static_cast<int>(kCombatEffPalette.size());

static const std::array<QString, 3> kReinforcedPalette{
  {QStringLiteral("-"), QStringLiteral("+"), QStringLiteral("R")}}; // unspecified, reinforced, reduced
static const int kReinforcedPaletteSize = static_cast<int>(kReinforcedPalette.size());

static const std::array<QString, 8> kCountryPalette{{QStringLiteral("US-"), QStringLiteral("UK-"), QStringLiteral("DE-"), QStringLiteral("FR-"),
                                                     QStringLiteral("CA-"), QStringLiteral("AU-"), QStringLiteral("JP-"), QStringLiteral("KR-")}};
static const int kCountryPaletteSize = static_cast<int>(kCountryPalette.size());

EntityGenerator::EntityGenerator(Config cfg) :
  m_cfg(std::move(cfg)),
  m_rng(m_cfg.seed)
{
  m_entities.reserve(static_cast<size_t>(m_cfg.entityCount));

  // Bucket palette entries by battle dimension so the weighted sampler in
  // rollIdentity() picks from real palette SIDCs the renderer handles.
  for (int paletteIdx = 0; paletteIdx < kSidcPaletteSize; ++paletteIdx)
  {
    const int dim = dimFromSidc(kSidcPalette.at(static_cast<std::size_t>(paletteIdx)));
    if (dim < 0)
    {
      continue;
    }
    m_dimBuckets.at(static_cast<std::size_t>(dim)).push_back(paletteIdx);
  }

  // Zero the weight of any dimension with no palette entries.
  std::array<int, kDimCount> dimWeights{{m_cfg.dimWeightAir, m_cfg.dimWeightGround, m_cfg.dimWeightSea, m_cfg.dimWeightSub}};
  for (int dim = 0; dim < kDimCount; ++dim)
  {
    const auto dimIndex = static_cast<std::size_t>(dim);
    if (m_dimBuckets.at(dimIndex).empty())
    {
      dimWeights.at(dimIndex) = 0;
    }
  }
  m_dimDist = std::discrete_distribution<int>(std::begin(dimWeights), std::end(dimWeights));

  std::uniform_real_distribution<double> lonDist(m_cfg.bboxLonMin, m_cfg.bboxLonMax);
  std::uniform_real_distribution<double> latDist(m_cfg.bboxLatMin, m_cfg.bboxLatMax);
  std::uniform_real_distribution<float> hdgDist(0.0F, 6.2831853F);

  const uint64_t nowEpoch =
    static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

  for (int i = 0; i < m_cfg.entityCount; ++i)
  {
    m_entities.emplace_back();
    auto& entity = m_entities.back();
    // Built once — stable for the lifetime of this entity.
    entity.id = QStringLiteral("e%1").arg(m_cfg.idOffset + i + 1, 7, 10, QLatin1Char('0'));

    entity.lon = lonDist(m_rng);
    entity.lat = latDist(m_rng);
    entity.headingRad = hdgDist(m_rng);
    entity.datetimevalid = static_cast<double>(nowEpoch);

    rollIdentity(entity);
    rollModifiers(entity);
  }
}

const std::vector<Entity>& EntityGenerator::entities() const
{
  return m_entities;
}

void EntityGenerator::rollIdentity(Entity& entity)
{
  std::uniform_real_distribution<double> altAirDist(1000.0, 10000.0);
  std::uniform_real_distribution<double> altSubDist(-200.0, -10.0);
  std::uniform_real_distribution<float> spdAirDist(150.0F, 280.0F);
  std::uniform_real_distribution<float> spdGroundDist(3.0F, 12.0F);
  std::uniform_real_distribution<float> spdSeaDist(8.0F, 18.0F);
  std::uniform_real_distribution<float> spdSubDist(4.0F, 12.0F);

  // Pick a dimension by the configured weight mix, then a SIDC within it.
  const int dim = m_dimDist(m_rng);
  const auto& bucket = m_dimBuckets.at(static_cast<std::size_t>(dim));
  std::uniform_int_distribution<size_t> inBucket(0, bucket.size() - 1);
  const QString& sidc = kSidcPalette.at(static_cast<std::size_t>(bucket.at(inBucket(m_rng))));
  entity.sidc = sidc;
  entity.dim = dimFromSidc(sidc);

  // Altitude + speed coupled to the dimension (subs underwater, jets high).
  switch (entity.dim)
  {
    case 0:
      entity.altitude = altAirDist(m_rng);
      entity.speedMps = spdAirDist(m_rng);
      break;
    case 1:
      entity.altitude = m_cfg.altDefault;
      entity.speedMps = spdGroundDist(m_rng);
      break;
    case 2:
      entity.altitude = 0.0;
      entity.speedMps = spdSeaDist(m_rng);
      break;
    case 3:
      entity.altitude = altSubDist(m_rng);
      entity.speedMps = spdSubDist(m_rng);
      break;
    default:
      entity.altitude = m_cfg.altDefault;
      entity.speedMps = m_cfg.speedMps;
      break;
  }

  // Callsign, parent unit, country.
  std::uniform_int_distribution<int> phonD(0, kPhoneticSize - 1);
  std::uniform_int_distribution<int> sufD(1, 999);
  std::uniform_int_distribution<int> divD(1, 9);
  std::uniform_int_distribution<int> bdeD(1, 9);
  std::uniform_int_distribution<int> ctryD(0, kCountryPaletteSize - 1);

  entity.uniquedesignation = QStringLiteral("%1-%2").arg(kPhonetic.at(static_cast<std::size_t>(phonD(m_rng)))).arg(sufD(m_rng));
  entity.higherformation = QStringLiteral("DIV%1-BDE%2").arg(divD(m_rng)).arg(bdeD(m_rng));
  entity.countrylabel = kCountryPalette.at(static_cast<std::size_t>(ctryD(m_rng)));
}

void EntityGenerator::rollModifiers(Entity& entity)
{
  std::uniform_int_distribution<int> statusD(0, kStatusPaletteSize - 1);
  std::uniform_int_distribution<int> echelonD(0, kEchelonPaletteSize - 1);
  std::uniform_int_distribution<int> hqtffdD(0, kHqtffdPaletteSize - 1);
  std::uniform_int_distribution<int> ceffD(0, kCombatEffPaletteSize - 1);
  std::uniform_int_distribution<int> reinforcedD(0, kReinforcedPaletteSize - 1);
  std::uniform_int_distribution<int> qtyD(1, 99);

  const QString& status = kStatusPalette.at(static_cast<std::size_t>(statusD(m_rng)));
  const QString& echelon = kEchelonPalette.at(static_cast<std::size_t>(echelonD(m_rng)));
  const QString& combatEff = kCombatEffPalette.at(static_cast<std::size_t>(ceffD(m_rng)));
  entity.status = status;
  entity.echelon = echelon;
  entity.hqtffd = kHqtffdPalette.at(static_cast<std::size_t>(hqtffdD(m_rng)));
  entity.combateffectiveness = combatEff;
  entity.reinforced = kReinforcedPalette.at(static_cast<std::size_t>(reinforcedD(m_rng)));
  entity.quantity = static_cast<quint32>(qtyD(m_rng));

  // Short, decorative free-text fields.
  entity.additionalinformation = QStringLiteral("info-%1%2%3").arg(status, echelon).arg(entity.quantity);
  entity.staffcomment = QStringLiteral("note q=%1 ce=%2").arg(entity.quantity).arg(combatEff);
}

void EntityGenerator::advanceEntities(double dtSeconds)
{
  std::normal_distribution<float> jitter(0.0F, m_cfg.headingJitterRadPerGenTick);

  // Flat-earth approximation — cheap and accurate enough at these scales.
  constexpr double kMetersPerDegLat = 111320.0;
  constexpr double kPi = 3.14159265358979323846;

  const double motionScale = m_cfg.motionScale;

  const uint64_t nowEpoch =
    static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

  for (auto& entity : m_entities)
  {
    entity.headingRad += jitter(m_rng);

    const double meters = static_cast<double>(entity.speedMps) * dtSeconds * motionScale;
    const double dxMeters = meters * std::sin(entity.headingRad);
    const double dyMeters = meters * std::cos(entity.headingRad);

    const double metersPerDegLon = kMetersPerDegLat * std::cos(entity.lat * kPi / 180.0);
    entity.lon += dxMeters / metersPerDegLon;
    entity.lat += dyMeters / kMetersPerDegLat;

    if (entity.lon < m_cfg.bboxLonMin)
    {
      entity.lon = m_cfg.bboxLonMax;
    }
    else if (entity.lon > m_cfg.bboxLonMax)
    {
      entity.lon = m_cfg.bboxLonMin;
    }
    if (entity.lat < m_cfg.bboxLatMin)
    {
      entity.lat = m_cfg.bboxLatMax;
    }
    else if (entity.lat > m_cfg.bboxLatMax)
    {
      entity.lat = m_cfg.bboxLatMin;
    }

    // Heading radians -> degrees in [0, 360) for the payload.
    float deg = std::fmod(static_cast<float>(entity.headingRad * 180.0 / kPi), 360.0F);
    if (deg < 0.0F)
    {
      deg += 360.0F;
    }

    // Round to whole degrees so tiny heading jitter does not churn the payload.
    double roundedDeg = std::round(static_cast<double>(deg));
    if (roundedDeg >= 360.0)
    {
      roundedDeg = 0.0;
    }
    entity.direction = roundedDeg;

    if (m_cfg.randomizePerGenTick)
    {
      rollIdentity(entity);
      rollModifiers(entity);
      entity.datetimevalid = static_cast<double>(nowEpoch);
    }
  }
}
