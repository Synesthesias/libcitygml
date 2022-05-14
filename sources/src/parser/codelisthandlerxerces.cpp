#include <parser/codelisthandlerxerces.h>
#include <iostream>

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/PlatformUtils.hpp>

using namespace xercesc;

const CodeList CodeListHandlerXerces::getCodeList(const std::string codeSpace) const {
    CodeList code_list;

    try {
        XMLPlatformUtils::Initialize();
    }
    catch (...) {
        throw std::runtime_error("Fail to initialize Xerces-C++.");
    }

    SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();

    try {
        XercesHandler handler;
        parser->setContentHandler(&handler);
        parser->parse(codeSpace.c_str());
        code_list = handler.getCodeList();
    }
    catch (...) {
        throw std::runtime_error("Fail to analize xml file.");
    }
    delete parser;
    XMLPlatformUtils::Terminate();

    return code_list;
}

XercesHandler::XercesHandler() :
    _definition_tag_flg(false),
    _description_tag_flg(false),
    _name_tag_flg(false),
    _description(""),
    _name(""),
    _content("")
{ 
}

CodeList XercesHandler::getCodeList() {
    return _code_list;
}

void XercesHandler::startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc::Attributes& attrs) {
	char* name = XMLString::transcode(localname);

    if (std::string(name) == "Definition") _definition_tag_flg = true;
    if (std::string(name) == "description" && _definition_tag_flg) _description_tag_flg = true;
    if (std::string(name) == "name" && _definition_tag_flg) _name_tag_flg = true;

	XMLString::release(&name);
}

void XercesHandler::endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) {
	char* name = XMLString::transcode(localname);

    if (std::string(name) == "description") _description_tag_flg = false;
    if (std::string(name) == "name") _name_tag_flg = false;

    if (std::string(name) == "Definition") {
        _code_list[stoi(_name)] = _description;
        
        std::cout << "CodeList[" << _name << "] = " << _description << std::endl;// for debug

        _definition_tag_flg = false;
        _description = "";
        _name = "";
    }
    
	XMLString::release(&name);
}

void XercesHandler::characters(const XMLCh* const chars, const XMLSize_t length) {
	XMLCh* buffer = new XMLCh[XMLString::stringLen(chars) + 1];
	XMLString::copyString(buffer, chars);
	XMLString::trim(buffer);
	char* content = XMLString::transcode(buffer);
	delete[] buffer;

    if (_description_tag_flg) _description = std::string(content);
    if (_name_tag_flg) _name = std::string(content);

	XMLString::release(&content);
}