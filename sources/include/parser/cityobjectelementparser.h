#pragma once

#include <parser/gmlfeaturecollectionparser.h>

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

#include <citygml/cityobject.h>

namespace citygml {

    class CityObjectElementParser : public GMLFeatureCollectionElementParser {
    public:
        CityObjectElementParser(CityGMLDocumentParser& documentParser, CityGMLFactory& factory, std::shared_ptr<CityGMLLogger> logger, const ParserParams& parserParams, std::function<void(std::shared_ptr<CityObject>)> callback);
        
        // ElementParser interface
        virtual std::string elementParserName() const override;
        virtual bool handlesElement(const NodeType::XMLNode &node) const override;

    protected:

        // CityGMLElementParser interface
        virtual bool parseElementStartTag(const NodeType::XMLNode& node, Attributes& attributes) override;
        virtual bool parseElementEndTag(const NodeType::XMLNode& node, const std::string& characters) override;
        virtual bool parseChildElementStartTag(const NodeType::XMLNode& node, Attributes& attributes) override;
        virtual bool parseChildElementEndTag(const NodeType::XMLNode& node, const std::string& characters) override;

        // GMLFeatureCollectionElementParser interface
        virtual FeatureObject* getFeatureObject() override;

    private:
        static void initializeTypeIDTypeMap();
        static void initializeAttributesSet();
        static AttributeType getAttributeType(const NodeType::XMLNode& node);

        std::shared_ptr<CityObject> m_model;
        std::function<void(std::shared_ptr<CityObject>)> m_callback;

        std::string m_lastAttributeName;
        AttributeType m_lastAttributeType;
        AttributesMap* m_genericAttributeSet;

        // The nodes that are valid CityObjects
        static std::mutex initializedTypeIDMutex;
        static std::unordered_map<int, CityObject::CityObjectsType> typeIDTypeMap;
        static bool typeIDTypeMapInitialized;

        static std::mutex initializedAttributeSetMutex;
        static std::unordered_set<int> attributesSet;
        static std::unordered_map<int, AttributeType> attributeTypeMap;
        static bool attributesSetInitialized;

        void parseGeometryForLODLevel(int lod);
        void parseGeometryForLod0RoofEdgeNode();
        void parseGeometryForLod0FootPrintNode();
        void parseImplicitGeometryForLODLevel(int lod);
        void parseGeometryPropertyElementForLODLevel(int lod, const std::string& id);

        /**
         * @brief
         * When m_parserParams.ignoreGeometries is false, returns new GeometryElementParser.
         * When m_parserParams.ignoreGeometries is true , returns new SkipElementParser.
         */
        ElementParser* MakeGeometryElementParser(int lodLevel, CityObject::CityObjectsType parentType);

        std::string m_lastCodeSpace;
        std::string m_lastCode;

        const ParserParams& m_parserParams;
    };

}
