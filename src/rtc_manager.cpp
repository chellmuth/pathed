#include "rtc_manager.h"

#include <assert.h>

RTCManager::RTCManager(RTCScene rootScene)
    : m_rootScene(rootScene)
{
    m_rtcSceneToSurfaces[m_rootScene] = NestedSurfaceVector();
}

void RTCManager::registerSurfaces(
    RTCScene rtcScene,
    // int rtcGeometryID,
    std::vector<std::shared_ptr<Surface> > &geometrySurfaces
) {
    const int keyCount = m_rtcSceneToSurfaces.count(rtcScene);
    if (keyCount == 0) {
        m_rtcSceneToSurfaces[rtcScene] = std::vector<std::vector<std::shared_ptr<Surface> > >();
    }

    NestedSurfaceVector &sceneSurfaces = m_rtcSceneToSurfaces[rtcScene];

    // assert(sceneSurfaces.size() - 1 == rtcGeometryID);

    sceneSurfaces.push_back(geometrySurfaces);
}

void RTCManager::registerSurfaces(
    // int rtcGeometryID,
    std::vector<std::shared_ptr<Surface> > &geometrySurfaces
) {
    registerSurfaces(m_rootScene, /*rtcGeometryID,*/ geometrySurfaces);
}

void RTCManager::registerInstanceSurfaces(
    RTCScene rtcInstanceScene,
    // int rtcGeometryID,
    std::vector<std::shared_ptr<Surface> > &geometrySurfaces
) {
    registerSurfaces(rtcInstanceScene, /*rtcGeometryID,*/ geometrySurfaces);
}

void RTCManager::registerInstancedSurfaces(
    RTCScene rtcInstanceScene,
    int rtcGeometryID,
    std::vector<std::shared_ptr<Surface> > &geometrySurfaces
) {
    registerSurfaces(/*rtcGeometryID, */ geometrySurfaces);
    m_rtcSceneLookup[rtcGeometryID] = rtcInstanceScene;
}

std::shared_ptr<Surface> RTCManager::lookupInstancedSurface(
    int rtcGeometryID,
    int rtcPrimitiveID,
    int rtcInstanceID
) const {
    const NestedSurfaceVector &sceneSurfaces = m_rtcSceneToSurfaces.at(m_rootScene);
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
    const NestedSurfaceVector &sceneSurfaces = m_rtcSceneToSurfaces.at(rtcScene);
    const std::vector<std::shared_ptr<Surface> > &geometrySurfaces = sceneSurfaces.at(rtcGeometryID);
    return geometrySurfaces.at(rtcPrimitiveID);
}

RTCGeometry RTCManager::lookupGeometry(
    int rtcGeometryID,
    int rtcInstanceID
) const {
    RTCScene rtcInstanceScene = m_rtcSceneLookup.at(rtcInstanceID);
    return rtcGetGeometry(rtcInstanceScene, rtcGeometryID);
}


void RTCManager::registerFilters(void (&callback)(const RTCFilterFunctionNArguments *))
{
    const NestedSurfaceVector &rootSurfaces = m_rtcSceneToSurfaces.at(m_rootScene);
    for (int rtcRootGeometryID = 0; rtcRootGeometryID < rootSurfaces.size(); rtcRootGeometryID++) {
        if (m_rtcSceneLookup.count(rtcRootGeometryID) == 0) {
            const RTCGeometry rtcGeometry = rtcGetGeometry(m_rootScene, rtcRootGeometryID);
            rtcSetGeometryIntersectFilterFunction(rtcGeometry, callback);
            rtcSetGeometryOccludedFilterFunction(rtcGeometry, callback);
            continue;
        }

        const RTCScene instanceScene = m_rtcSceneLookup.at(rtcRootGeometryID);
        const NestedSurfaceVector sceneSurfaces = m_rtcSceneToSurfaces.at(instanceScene);

        for (int rtcGeometryID = 0; rtcGeometryID < sceneSurfaces.size(); rtcGeometryID++) {
            const RTCGeometry rtcGeometry = rtcGetGeometry(instanceScene, rtcGeometryID);
            rtcSetGeometryIntersectFilterFunction(rtcGeometry, callback);
            rtcSetGeometryOccludedFilterFunction(rtcGeometry, callback);
        }
    }
}
