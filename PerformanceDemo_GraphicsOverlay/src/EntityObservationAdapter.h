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

#ifndef GEOSWARM_ENTITYOBSERVATIONADAPTER_H
#define GEOSWARM_ENTITYOBSERVATIONADAPTER_H

#include <QtGlobal>

struct Entity;
struct Observation;

Observation observationFromEntity(const Entity& entity, quint16 flags);

#endif // GEOSWARM_ENTITYOBSERVATIONADAPTER_H
