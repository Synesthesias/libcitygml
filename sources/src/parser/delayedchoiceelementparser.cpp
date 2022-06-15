#include "parser/delayedchoiceelementparser.h"
#include "parser/geometryelementparser.h"
#include "parser/documentlocation.h"
#include "parser/citygmldocumentparser.h"
#include "parser/attributes.h"
#include "parser/cityobjectelementparser.h"

#include <citygml/citygmllogger.h>

#include <sstream>
#include <stdexcept>

namespace citygml {

    DelayedChoiceElementParser::DelayedChoiceElementParser(CityGMLDocumentParser& documentParser, std::shared_ptr<CityGMLLogger> logger, std::vector<ElementParser*> choices)
        : ElementParser(documentParser, logger)
        , m_unknownNodeFlg(false)
        , m_stockNode()
    {
        m_choices = choices;
    }

    bool DelayedChoiceElementParser::startElement(const NodeType::XMLNode& node, Attributes& attributes)
    {
        if (m_unknownNodeFlg) {
            m_unknownNodeFlg = false;
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
        } else {
            ElementParser* choosenParser = nullptr;
            for (ElementParser* parser : m_choices) {

                if (choosenParser == nullptr && parser->handlesElement(node)) {
                    choosenParser = parser;
                } else {
                    delete parser;
                }
            }

            if (choosenParser != nullptr) {
                m_documentParser.removeCurrentElementParser(this);
                m_documentParser.setCurrentElementParser(choosenParser);
                return choosenParser->startElement(node, attributes);
            } else {
                CITYGML_LOG_ERROR(m_logger, "DelayedChoiceElementParser could not find a parser to handle <" << node << "> at " << getDocumentLocation());
                throw std::runtime_error("No parser for XML element found.");
            }
        }
    }

    bool DelayedChoiceElementParser::endElement(const NodeType::XMLNode& node, const std::string& characters)
    {
        if (m_unknownNodeFlg) {// for unknown node which does not have any child
            m_unknownNodeFlg = false;
            m_documentParser.removeCurrentElementParser(this);
            delete m_choices[0];
            delete m_choices[1];
            return dynamic_cast <CityObjectElementParser*>(m_choices[2])->parseChildElementBothTag(node, characters);
        }else{
            throw std::runtime_error("DelayedChoiceElementParser::endElement must never be called.");
        }
    }

    bool DelayedChoiceElementParser::handlesElement(const NodeType::XMLNode& node) const
    {
        for (const ElementParser* parser : m_choices) {
            if (parser->handlesElement(node)) {
                return true;
            }
        }
        return false;
    }

    std::string DelayedChoiceElementParser::elementParserName() const
    {
        std::stringstream ss;
        ss << "DelayedChoiceElementParser (";
        for (size_t i = 0; i < m_choices.size(); i++) {

            if (i > 0) {
                ss << " | ";
            }

            ss << m_choices[i]->elementParserName();
        }

        ss << ")";
        return ss.str();
    }

    void DelayedChoiceElementParser::setUnknownNodeFlg(bool flg) {
        m_unknownNodeFlg = flg;
    }

    void DelayedChoiceElementParser::setStockNode(const NodeType::XMLNode& node) {
        m_stockNode = node;
    }

}
