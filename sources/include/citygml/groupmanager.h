#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <citygml/citygml_api.h>

namespace citygml {

    class CityObject;
    class CityGMLLogger;

    class LIBCITYGML_EXPORT GroupManager {
    public:
        GroupManager(std::shared_ptr<CityGMLLogger> logger);

        void addSharedGroupMember(std::shared_ptr<CityObject> cityobject);
        void requestSharedGroupMember(CityObject* cityobject, const std::string& cityobjectID);

        void finish();

        ~GroupManager();
    private:
        struct GroupRequest {
            GroupRequest(CityObject* target, std::string cityobjectID) : target(target), cityobjectID(cityobjectID) {
            }
            CityObject* target;
            std::string cityobjectID;
        };

        std::shared_ptr<CityGMLLogger> m_logger;
        std::vector<GroupRequest> m_groupRequests;
        std::unordered_map<std::string, std::shared_ptr<CityObject> > m_sharedCityobjects;
    };

}