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

#include "EntityObservationAdapter.h"

#include "EntityGenerator.h"
#include "Observation.h"

Observation observationFromEntity(const Entity& entity, quint16 flags)
{
  Observation observation;
  observation.id = entity.id;
  observation.longitude = entity.lon;
  observation.latitude = entity.lat;
  observation.presentFlags = flags;

  if (flags & ObsFlags::SIDC)
  {
    observation.sidc = entity.sidc;
  }
  if (flags & ObsFlags::ALT)
  {
    observation.altitude = entity.altitude;
  }
  if (flags & ObsFlags::DIRECTION)
  {
    observation.direction = entity.direction;
  }
  if (flags & ObsFlags::SPEED)
  {
    observation.speed = static_cast<double>(entity.speedMps);
  }
  if (flags & ObsFlags::STATUS)
  {
    observation.status = entity.status;
  }
  if (flags & ObsFlags::ECHELON)
  {
    observation.echelon = entity.echelon;
  }
  if (flags & ObsFlags::HQTFFD)
  {
    observation.hqtffd = entity.hqtffd;
  }
  if (flags & ObsFlags::UNIQUEDESIG)
  {
    observation.uniquedesignation = entity.uniquedesignation;
  }
  if (flags & ObsFlags::ADDINFO)
  {
    observation.additionalinformation = entity.additionalinformation;
  }
  if (flags & ObsFlags::QUANTITY)
  {
    observation.quantity = entity.quantity;
  }
  if (flags & ObsFlags::COMBATEFF)
  {
    observation.combateffectiveness = entity.combateffectiveness;
  }
  if (flags & ObsFlags::STAFFCOMMENT)
  {
    observation.staffcomment = entity.staffcomment;
  }
  if (flags & ObsFlags::HIGHERFORM)
  {
    observation.higherformation = entity.higherformation;
  }
  if (flags & ObsFlags::COUNTRY)
  {
    observation.countrylabel = entity.countrylabel;
  }
  if (flags & ObsFlags::REINFORCED)
  {
    observation.reinforced = entity.reinforced;
  }
  if (flags & ObsFlags::DATETIME)
  {
    observation.datetimevalid = entity.datetimevalid;
  }

  return observation;
}
