#include "rtc_manager.h"

#include <assert.h>

RTCManager::RTCManager(RTCScene rootScene)
    : m_rootScene(rootScene)
{}

void RTCManager::registerSurfaces(
    RTCScene rtcScene,
    int rtcGeometryID,
    std::vector<std::shared_ptr<Surface> > &geometrySurfaces
) {
    const int keyCount = m_rtcSceneToSurfaces.count(rtcScene);
    if (keyCount == 0) {
        m_rtcSceneToSurfaces[rtcScene] = std::vector<std::vector<std::shared_ptr<Surface> > >();
    }

    std::vector<std::vector<std::shared_ptr<Surface> > > &sceneSurfaces = m_rtcSceneToSurfaces[m_rootScene];

    assert(sceneSurfaces.size() - 1 == rtcGeometryID);

    sceneSurfaces.push_back(geometrySurfaces);
}

void RTCManager::registerSurfaces(
    int rtcGeometryID,
    std::vector<std::shared_ptr<Surface> > &geometrySurfaces
) {
    registerSurfaces(m_rootScene, rtcGeometryID, geometrySurfaces);
}

void RTCManager::registerInstanceSurfaces(
    RTCScene rtcInstanceScene,
    int rtcGeometryID,
    std::vector<std::shared_ptr<Surface> > &geometrySurfaces
) {
    registerSurfaces(rtcInstanceScene, rtcGeometryID, geometrySurfaces);
}

void RTCManager::registerInstancedSurfaces(
    RTCScene rtcInstanceScene,
    int rtcGeometryID,
    std::vector<std::shared_ptr<Surface> > &geometrySurfaces
) {
    registerSurfaces(rtcGeometryID, geometrySurfaces);
    m_rtcSceneLookup[rtcGeometryID] = rtcInstanceScene;
}

std::shared_ptr<Surface> RTCManager::lookupInstancedSurface(
    int rtcGeometryID,
    int rtcPrimitiveID,
    int rtcInstanceID
) const {
    const std::vector<std::vector<std::shared_ptr<Surface> > > &sceneSurfaces = m_rtcSceneToSurfaces.at(m_rootScene);
    const std::vector<std::shared_ptr<Surface> > &geometrySurfaces = sceneSurfaces.at(rtcInstanceID);

    assert(geometrySurfaces.size() == 0);

    RTCScene rtcInstanceScene = m_rtcSceneLookup.at(rtcInstanceID);
    return lookupSurface(rtcInstanceScene, rtcGeometryID, rtcPrimitiveID);
}

std::shared_ptr<Surface> RTCManager::lookupSurface(int rtcGeometryID, int rtcPrimitiveID) const
{
    return lookupSurface(m_rootScene, rtcGeometryID, rtcPrimitiveID);
}

std::shared_ptr<Surface> RTCManager::lookupSurface(RTCScene rtcScene, int rtcGeometryID, int rtcPrimitiveID) const
{
    const std::vector<std::vector<std::shared_ptr<Surface> > > &sceneSurfaces = m_rtcSceneToSurfaces.at(rtcScene);
    const std::vector<std::shared_ptr<Surface> > &geometrySurfaces = sceneSurfaces.at(rtcGeometryID);
    return geometrySurfaces.at(rtcPrimitiveID);
}
