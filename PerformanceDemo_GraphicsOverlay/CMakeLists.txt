cmake_minimum_required(VERSION 3.16)

project(GeoSwarm LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(IOS)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "16.0" CACHE STRING "Minimum iOS deployment version" FORCE)
elseif(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS deployment version" FORCE)
endif()

find_package(Qt6 COMPONENTS REQUIRED Core Concurrent Quick QuickControls2 Multimedia Positioning Sensors)
if (Qt6Core_VERSION VERSION_LESS 6.8.2)
  message(FATAL_ERROR "This version of the ArcGIS Maps SDK for Qt requires at least Qt 6.8.2")
endif()
find_package(ArcGISRuntime 300.0.0 COMPONENTS REQUIRED Cpp)

set(SOURCE_FILES
  src/main.cpp
  src/GeoSwarm.cpp
  src/GeoSwarm.h
  src/Observation.h
  src/EntityGenerator.h
  src/EntityGenerator.cpp
  src/EntityObservationAdapter.h
  src/EntityObservationAdapter.cpp
  src/GenerationWorker.h
  src/GenerationWorker.cpp
  src/GenerationController.h
  src/GenerationController.cpp
  src/GraphicsEntityController.h
  src/GraphicsEntityController.cpp
  src/PerformanceMonitor.h
  src/PerformanceMonitor.cpp
  qml/qml.qrc
  Resources/Resources.qrc
  $<$<BOOL:${WIN32}>:Win/Resources.rc>
  $<$<BOOL:${APPLE}>:Mac/AppIcon.icns>)

if(ANDROID)
  qt_add_executable(${PROJECT_NAME} ${SOURCE_FILES})
else()
  qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_FILES})
  # IOS uses static Runtimecore Framework
  if(IOS)
    qt_add_ios_ffmpeg_libraries(${PROJECT_NAME})
    get_target_property(Qt6Core_LOCATION Qt6::Core LOCATION)
    get_filename_component(Qt6_DIR ${Qt6Core_LOCATION} DIRECTORY)
    set(FFMPEG_LIB_DIR "${Qt6_DIR}/../ffmpeg")
    if(Qt6Core_VERSION VERSION_LESS "6.8.3")
      set(FRAMEWORK "framework")
    else()
      set(FRAMEWORK "xcframework")
    endif()

    set(FFMPEG_FRAMEWORKS
      "${FFMPEG_LIB_DIR}/libavcodec.${FRAMEWORK}"
      "${FFMPEG_LIB_DIR}/libavformat.${FRAMEWORK}"
      "${FFMPEG_LIB_DIR}/libavutil.${FRAMEWORK}"
      "${FFMPEG_LIB_DIR}/libswresample.${FRAMEWORK}"
      "${FFMPEG_LIB_DIR}/libswscale.${FRAMEWORK}")
    set_target_properties(${PROJECT_NAME} PROPERTIES
      INSTALL_RPATH "@executable_path/Frameworks;@loader_path/Frameworks"
      XCODE_EMBED_FRAMEWORKS "${ArcGISRuntime_runtimecore_LIB};${FFMPEG_FRAMEWORKS}"
      XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY TRUE
      LINK_FLAGS "-F${FFMPEG_LIB_DIR} -F${ArcGISRuntime_runtimecore_LIB_DIR} -Wl,-rpath,@executable_path -Wl,-rpath,@loader_path/Frameworks")
  endif()
  set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
  set_source_files_properties(Mac/AppIcon.icns
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

  if(DEFINED ArcGISRuntime_LIBRARIES)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${ArcGISRuntime_LIBRARIES}
      $<TARGET_FILE_DIR:${PROJECT_NAME}>)
  endif()
endif()

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt6::Core
  Qt6::Concurrent
  Qt6::Quick
  Qt6::QuickControls2
  Qt6::Multimedia
  Qt6::Positioning
  Qt6::Sensors
  ArcGISRuntime::Cpp)

if(ANDROID)
  set(PROJECT_DEPLOYABLE_LIBS
    ${ArcGISRuntime_LIBRARIES})

  list(JOIN PROJECT_DEPLOYABLE_LIBS , PROJECT_DEPLOYABLE_LIBS_STRING)

  add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${ArcGISRuntime_MapsSDKNative_JAR}"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/android-build-${PROJECT_NAME}/libs/MapsSDKNative.jar")

  set_property(TARGET ${PROJECT_NAME} PROPERTY QT_ANDROID_EXTRA_LIBS ${PROJECT_DEPLOYABLE_LIBS_STRING} ${ANDROID_EXTRA_LIBS})
  get_property(EXTRA_LIBS TARGET ${PROJECT_NAME} PROPERTY QT_ANDROID_EXTRA_LIBS)
  if(NOT EXTRA_LIBS MATCHES "libssl" OR NOT EXTRA_LIBS MATCHES "libcrypto")
    message(WARNING "openssl libraries are missing, check the project CMakeLists.txt and set up openssl environment")
  endif()
endif()