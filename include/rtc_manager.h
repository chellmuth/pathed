#pragma once

#include "surface.h"

#include <embree3/rtcore.h>

#include <map>
#include <memory>
#include <vector>

class RTCManager {
public:
    RTCManager(RTCScene rootScene);

    void registerSurfaces(
        /* int rtcGeometryID, */
        std::vector<std::shared_ptr<Surface> > &geometrySurfaces
    );

    void registerInstanceSurfaces(
        RTCScene rtcInstanceScene,
        /* int rtcGeometryID, */
        std::vector<std::shared_ptr<Surface> > &geometrySurfaces
    );

    void registerInstancedSurfaces(
        RTCScene rtcInstanceScene,
        int rtcGeometryID,
        std::vector<std::shared_ptr<Surface> > &geometrySurfaces
    );

    std::shared_ptr<Surface> lookupInstancedSurface(
        int rtcGeometryID,
        int rtcPrimitiveID,
        int rtcInstanceID
    ) const;

    std::shared_ptr<Surface> lookupSurface(
        int rtcGeometryID,
        int rtcPrimitiveID
    ) const;

private:
    RTCScene m_rootScene;

    void registerSurfaces(
        RTCScene rtcScene,
        /* int rtcGeometryID, */
        std::vector<std::shared_ptr<Surface> > &geometrySurfaces
    );

    std::shared_ptr<Surface> lookupSurface(
        RTCScene rtcScene,
        int rtcGeometryID,
        int rtcPrimitiveID
    ) const;


    std::map<int, RTCScene> m_rtcSceneLookup;
    std::map<RTCScene, std::vector<std::vector<std::shared_ptr<Surface> > > > m_rtcSceneToSurfaces;
};
