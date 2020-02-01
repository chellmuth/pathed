#include "rtc_manager.h"

#include <assert.h>

RTCManager::RTCManager(RTCScene rootScene)
    : m_rootScene(rootScene)
{
    m_rtcSceneToSurfaces[m_rootScene] = NestedSurfaceVector();
}

void RTCManager::registerSurfaces(
    RTCScene rtcScene,
    std::vector<std::shared_ptr<Surface> > &geometrySurfaces
) {
    const int keyCount = m_rtcSceneToSurfaces.count(rtcScene);
    if (keyCount == 0) {
        m_rtcSceneToSurfaces[rtcScene] = std::vector<std::vector<std::shared_ptr<Surface> > >();
    }

    NestedSurfaceVector &sceneSurfaces = m_rtcSceneToSurfaces[rtcScene];
    sceneSurfaces.push_back(geometrySurfaces);

    if (geometrySurfaces.size() > 0) {
        m_rtcRegistrationQueue.push_back({rtcScene, sceneSurfaces.size() - 1});
    }
}

void RTCManager::registerInstancedSurfaces(
    RTCScene rtcScene,
    RTCScene rtcInstanceScene,
    int rtcGeometryID,
    std::vector<std::shared_ptr<Surface> > &geometrySurfaces
) {
    registerSurfaces(rtcScene, geometrySurfaces);
    m_rtcSceneLookup[{rtcScene, rtcGeometryID}] = rtcInstanceScene;
}

std::shared_ptr<Surface> RTCManager::lookupInstancedSurface(
    int rtcGeometryID,
    int rtcPrimitiveID,
    unsigned int *rtcInstanceIDs
) const {
    int i = 0;
    RTCScene rtcCurrentScene = m_rootScene;
    while (rtcInstanceIDs[i] != RTC_INVALID_GEOMETRY_ID && i < 2) {
        // const NestedSurfaceVector &sceneSurfaces = m_rtcSceneToSurfaces.at(rtcCurrentScene);
        // const std::vector<std::shared_ptr<Surface> > &geometrySurfaces = sceneSurfaces.at(rtcInstanceID);

        // assert(geometrySurfaces.size() == 0);

        rtcCurrentScene = m_rtcSceneLookup.at({rtcCurrentScene, rtcInstanceIDs[i]});
        i++;
    }

    return lookupSurface(rtcCurrentScene, rtcGeometryID, rtcPrimitiveID);
}

std::shared_ptr<Surface> RTCManager::lookupSurface(int rtcGeometryID, int rtcPrimitiveID) const
{
    return lookupSurface(m_rootScene, rtcGeometryID, rtcPrimitiveID);
}

std::shared_ptr<Surface> RTCManager::lookupSurface(RTCScene rtcScene, int rtcGeometryID, int rtcPrimitiveID) const
{
    const NestedSurfaceVector &sceneSurfaces = m_rtcSceneToSurfaces.at(rtcScene);
    const std::vector<std::shared_ptr<Surface> > &geometrySurfaces = sceneSurfaces.at(rtcGeometryID);
    return geometrySurfaces.at(rtcPrimitiveID);
}

RTCGeometry RTCManager::lookupGeometry(
    int rtcGeometryID,
    unsigned int *rtcInstanceIDs
) const {
    int i = 0;
    RTCScene rtcCurrentScene = m_rootScene;
    while (rtcInstanceIDs[i] != RTC_INVALID_GEOMETRY_ID && i < 2) {
        rtcCurrentScene = m_rtcSceneLookup.at({rtcCurrentScene, rtcInstanceIDs[i]});
        i++;
    }

    return rtcGetGeometry(rtcCurrentScene, rtcGeometryID);
}

void RTCManager::registerFilters(void (&callback)(const RTCFilterFunctionNArguments *))
{
    for (auto &pair : m_rtcRegistrationQueue) {
        const RTCGeometry rtcGeometry = rtcGetGeometry(pair.first, pair.second);
        rtcSetGeometryIntersectFilterFunction(rtcGeometry, callback);
        rtcSetGeometryOccludedFilterFunction(rtcGeometry, callback);
    }
}
