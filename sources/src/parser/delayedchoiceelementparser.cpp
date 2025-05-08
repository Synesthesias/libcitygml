#include "parser/delayedchoiceelementparser.h"

#include "parser/documentlocation.h"
#include "parser/citygmldocumentparser.h"
#include "parser/skipelementparser.h"

#include <citygml/citygmllogger.h>

#include <sstream>
#include <stdexcept>

namespace citygml {

    DelayedChoiceElementParser::DelayedChoiceElementParser(CityGMLDocumentParser& documentParser, std::shared_ptr<CityGMLLogger> logger, std::vector<ElementParser*> choices)
        : ElementParser(documentParser, logger)
    {
        m_choices = choices;
    }

    bool DelayedChoiceElementParser::startElement(const NodeType::XMLNode& node, Attributes& attributes)
    {
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
            CITYGML_LOG_WARN(m_logger, "DelayedChoiceElementParser could not find a parser to handle <" << node << "> at " << getDocumentLocation());
            m_documentParser.removeCurrentElementParser(this);
            m_documentParser.setCurrentElementParser(new SkipElementParser(m_documentParser, m_logger));
            return false;
        }
    }

    bool DelayedChoiceElementParser::endElement(const NodeType::XMLNode&, const std::string&)
    {
        CITYGML_LOG_ERROR(m_logger, "DelayedChoiceElementParser::endElement must never be called. at " << getDocumentLocation());
        // endElement must never be called, but this could happen if its content is empty.
        // So we skip the element with error message.
        m_documentParser.removeCurrentElementParser(this);
        return true;
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

}