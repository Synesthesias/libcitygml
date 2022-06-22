#include "parser/unknownelementparser.h"
#include "parser/geometryelementparser.h"
#include "parser/documentlocation.h"
#include "parser/citygmldocumentparser.h"
#include "parser/attributes.h"
#include "parser/cityobjectelementparser.h"

#include <citygml/citygmllogger.h>

#include <sstream>
#include <stdexcept>

namespace citygml {

    UnknownElementParser::UnknownElementParser(CityGMLDocumentParser& documentParser, std::shared_ptr<CityGMLLogger> logger, std::vector<ElementParser*> choices)
        : ElementParser(documentParser, logger)
        , m_stockNode()
    {
        m_choices = choices;
    }

    bool UnknownElementParser::startElement(const NodeType::XMLNode& node, Attributes& attributes)
    {
        m_documentParser.removeCurrentElementParser(this);
        if (attributes.getAttribute("gml:id") == "") {
            dynamic_cast <CityObjectElementParser*>(m_choices[2])->setAdeDataComingFlg(true);
            m_choices[2]->startElement(m_stockNode, attributes);// for parent node
            return m_choices[2]->startElement(node, attributes);
        } else {
            if(m_choices[0]->handlesElement(node)){// check for GeometryElementParser
                m_documentParser.setCurrentElementParser(m_choices[0]); // set GeometryElementParser
                delete m_choices[1];
                dynamic_cast <GeometryElementParser*>(m_choices[0])->SetUnknownGeometryComingFlg(true);
                m_choices[0]->startElement(m_stockNode, attributes);
                return m_choices[0]->startElement(node, attributes);
            } else {
                m_documentParser.setCurrentElementParser(m_choices[1]);// set CityObjectElementParser
                delete m_choices[0];
                dynamic_cast <CityObjectElementParser*>(m_choices[1])->SetUnknownCityObjectComingFlg(true);
                m_choices[1]->startElement(m_stockNode, attributes);
                return m_choices[1]->startElement(node, attributes);
            }
        }
    }

    bool UnknownElementParser::endElement(const NodeType::XMLNode& node, const std::string& characters)
    {
        m_documentParser.removeCurrentElementParser(this);
        delete m_choices[0];
        delete m_choices[1];
        return dynamic_cast <CityObjectElementParser*>(m_choices[2])->parseChildElementBothTag(node, characters);   
    }

    bool UnknownElementParser::handlesElement(const NodeType::XMLNode& node) const
    {
        for (const ElementParser* parser : m_choices) {
            if (parser->handlesElement(node)) {
                return true;
            }
        }
        return false;
    }

    std::string UnknownElementParser::elementParserName() const
    {
        std::stringstream ss;
        ss << "UnknownElementParser (";
        for (size_t i = 0; i < m_choices.size(); i++) {

            if (i > 0) {
                ss << " | ";
            }

            ss << m_choices[i]->elementParserName();
        }

        ss << ")";
        return ss.str();
    }

    void UnknownElementParser::setStockNode(const NodeType::XMLNode& node) {
        m_stockNode = node;
    }

}
