# GraphicsOverlay Performance Benchmark - GeoSwarm

GeoSwarm is a Qt 6 / QML demo app that showcases **ArcGIS GraphicsOverlay performance** for high-volume moving-entity scenarios. It generates synthetic observations in-process, buckets them by battle dimension, and updates preallocated graphics in place so you can study rendering and ingest behavior under load in both map and scene views.

## What it does

- Simulates large numbers of moving entities entirely in-process.
- Maintains four graphics overlays: air, ground, sea, and subsurface.
- Preallocates one `Graphic` per entity and updates geometry and attributes in place during steady-state runs.
- Uses a decoupled pipeline: a background generation worker produces observations while the overlay controller applies updates per dimension.
- Switches the same overlay set between `MapView` and `SceneView`.
- Supports MIL-2525C dictionary rendering or lightweight simple markers.
- Shows live ingest, entity-update, FPS, and frame-phase metrics in the UI.

## Project structure

- `src/`
	- `GeoSwarm`: top-level app coordinator that owns the map, scene, overlays, renderers, and generator.
	- `GraphicsEntityController`: owns the four `GraphicsOverlay` instances and the per-entity graphic pools.
	- `GenerationController` / `GenerationWorker` / `EntityGenerator`: background simulation and observation publishing pipeline.
	- `PerformanceMonitor`: captures Qt Quick frame timing, FPS, and render-phase metrics.
- `qml/`
	- `GeoSwarmForm.qml`: main application shell and view composition.
	- `SettingsPanel.qml`: map/scene toggle, renderer mode, location-only mode, layer visibility, and clear actions.
	- `StatusPanel.qml`: observation throughput, per-entity update interval, and tracked entity count.
	- `PerformancePanel.qml`: FPS and frame-phase timing HUD.
	- `SimulatorPanel.qml`: workload controls for entity count, target rate, battle-dimension mix, motion, and payload fields.
- `Resources/`
	- Includes the MIL-2525C style file used by the dictionary renderer.

## What you can try

- Change entity count and target observations per second.
- Switch between map and scene views while reusing the same overlay-backed entity set.
- Compare MIL-2525C dictionary symbols against a simple marker renderer.
- Enable `Location only` mode to isolate geometry-update cost from attribute and symbol-rule evaluation cost.
- Adjust battle-dimension weights, heading drift, speed multiplier, and per-pass randomization.
- Toggle individual overlay visibility for air, ground, sea, and subsurface entities.
- Watch how throughput, update interval, FPS, and frame timings respond as the workload changes.

## Build requirements

- CMake 3.16+
- Qt 6.8.2 or newer
- ArcGIS Maps SDK for Qt 300.0+

## Notes

- This is a demo app for exploring graphics-overlay performance behavior, not a production-ready application.
- The implementation is designed to demonstrate practical high-throughput overlay patterns: preallocation, in-place updates, off-thread generation, per-dimension batching, and keeping payloads small when possible.

## ArcGIS authentication

Set `GEOSWARM_ARCGIS_API_KEY` to use the ArcGIS basemap styles and World Elevation service. Without a key, the app uses the public World Topographic Map service and displays the scene without elevation. To get an ArcGIS Online API key with access to location services, follow the [Create an API key tutorial](https://developers.arcgis.com/documentation/security-and-authentication/api-key-authentication/tutorials/create-an-api-key/online/).
