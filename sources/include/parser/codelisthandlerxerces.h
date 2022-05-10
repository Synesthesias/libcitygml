#pragma once

#include <xercesc/sax2/DefaultHandler.hpp>
#include <map>
#include <string>

typedef std::map<int, std::string> CodeList;
typedef std::map<std::string, CodeList> CodeLists;

class CodeListHandlerXerces {
public:
	const CodeList getCodeList(const std::string codeSpace) const;
};

class XercesHandler : public xercesc::DefaultHandler {
public:
	XercesHandler();
	CodeList getCodeList();
	void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc::Attributes& attrs);
	void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname);
	void characters(const XMLCh* const chars, const XMLSize_t length);
private:
	bool _definition_tag_flg;
	bool _description_tag_flg;
	bool _name_tag_flg;
	std::string _description;
	std::string _name;
	std::string _content;
	CodeList _code_list;
};