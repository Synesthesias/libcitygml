#include <citygml/groupmanager.h>

#include <citygml/cityobject.h>
#include <citygml/citygmllogger.h>

namespace citygml {

    GroupManager::GroupManager(std::shared_ptr<CityGMLLogger> logger) {
        m_logger = logger;
    }

    void GroupManager::addSharedGroupMember(std::shared_ptr<CityObject> cityobject) {
        if (m_sharedCityobjects.count(cityobject->getId()) > 0) {
            CITYGML_LOG_WARN(m_logger, "Duplicate definition of shared cityobject with id '" << cityobject->getId() << "'... overwriting existing object.");
        }

        m_sharedCityobjects[cityobject->getId()] = cityobject;
    }

    void GroupManager::requestSharedGroupMember(CityObject* cityobject, const std::string& cityobjectID) {
        m_groupRequests.push_back(GroupRequest(cityobject, cityobjectID));
    }

    void GroupManager::finish() {
        CITYGML_LOG_INFO(m_logger, "Start processing shared cityobject requests (" << m_groupRequests.size() << ").");
        for (const GroupRequest& request : m_groupRequests) {

            auto it = m_sharedCityobjects.find(request.cityobjectID);
            if (it == m_sharedCityobjects.end()) {
                CITYGML_LOG_WARN(m_logger, "cityobject parent with id '" << request.target->getId() << "' requests cityobject member with id '" << request.cityobjectID << "' but no such"
                                 << "shared cityobject object exists.");
                continue;
            }

            request.target->addChildCityObject(it->second.get());
        }

        m_sharedCityobjects.clear();
        m_groupRequests.clear();

        CITYGML_LOG_INFO(m_logger, "Finished processing shared cityobject requests.");
    }

    GroupManager::~GroupManager() 
    {
    }


}