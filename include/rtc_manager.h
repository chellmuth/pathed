#pragma once

#include "surface.h"
#include "types.h"

#include <embree3/rtcore.h>

#include <map>
#include <memory>
#include <vector>

class RTCManager {
public:
    RTCManager(RTCScene rootScene);

    void registerSurfaces(
        RTCScene rtcScene,
        std::vector<std::shared_ptr<Surface> > &geometrySurfaces
    );

    void registerInstancedSurfaces(
        RTCScene rtcScene,
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

    RTCGeometry lookupGeometry(
        int rtcGeometryID,
        int rtcInstanceID
    ) const;

    const NestedSurfaceVector &getSurfaces() const
    {
        return m_rtcSceneToSurfaces.at(m_rootScene);
    }

    void registerFilters(void (&callback)(const RTCFilterFunctionNArguments *));

private:
    RTCScene m_rootScene;

    std::shared_ptr<Surface> lookupSurface(
        RTCScene rtcScene,
        int rtcGeometryID,
        int rtcPrimitiveID
    ) const;

    std::map<int, RTCScene> m_rtcSceneLookup;
    std::map<RTCScene, NestedSurfaceVector> m_rtcSceneToSurfaces;
};
