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

#ifndef GEOSWARM_OBSERVATION_H
#define GEOSWARM_OBSERVATION_H

#include <QMetaType>
#include <QString>
#include <QtGlobal>

// Bitmask of optional fields present in this observation; flush() checks these bits before writing attributes.
namespace ObsFlags
{
  inline constexpr quint16 SIDC = 1U << 0;
  inline constexpr quint16 ALT = 1U << 1;
  inline constexpr quint16 DIRECTION = 1U << 2;
  inline constexpr quint16 SPEED = 1U << 3;
  inline constexpr quint16 STATUS = 1U << 4;
  inline constexpr quint16 ECHELON = 1U << 5;
  inline constexpr quint16 HQTFFD = 1U << 6;
  inline constexpr quint16 UNIQUEDESIG = 1U << 7;
  inline constexpr quint16 ADDINFO = 1U << 8;
  inline constexpr quint16 QUANTITY = 1U << 9;
  inline constexpr quint16 COMBATEFF = 1U << 10;
  inline constexpr quint16 STAFFCOMMENT = 1U << 11;
  inline constexpr quint16 HIGHERFORM = 1U << 12;
  inline constexpr quint16 COUNTRY = 1U << 13;
  inline constexpr quint16 REINFORCED = 1U << 14;
  inline constexpr quint16 DATETIME = 1U << 15;

  inline constexpr quint16 kAll = SIDC | ALT | DIRECTION | SPEED | STATUS | ECHELON | HQTFFD | UNIQUEDESIG | ADDINFO | QUANTITY | COMBATEFF |
                                  STAFFCOMMENT | HIGHERFORM | COUNTRY | REINFORCED | DATETIME;
} // namespace ObsFlags

struct Observation
{
  QString id;
  double longitude = 0.0;
  double latitude = 0.0;

  quint16 presentFlags = 0;

  // Optional, flag-gated (see ObsFlags).
  QString sidc;

  // Also written to the Point Z coordinate.
  double altitude = 0.0;

  // Degrees, 0..360.
  double direction = 0.0;

  double speed = 0.0;
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
Q_DECLARE_METATYPE(Observation)

#endif
