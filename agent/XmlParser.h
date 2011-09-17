/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* All rights reserved.                                                        *
*                                                                             *
* Redistribution and use in source and binary forms, with or without modifi-  *
* cation, are permitted provided that the following conditions are met:       *
*                                                                             *
*    * Redistributions of source code must retain the above copyright notice  *
*      this list of conditions and the following disclaimer.                  *
*                                                                             *
*    * Redistributions in binary form must reproduce the above copyright      *
*      notice, this list of conditions and the following disclaimer in the    *
*      documentation and/or other materials provided with the distribution.   *
*                                                                             *
*    * Neither the name of RPISEC nor the names of its contributors may be    *
*      used to endorse or promote products derived from this software without *
*      specific prior written permission.                                     *
*                                                                             *
* THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     *
* NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      *
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                *
*                                                                             *
******************************************************************************/

/*!
	@file XmlParser.h
	
	@brief Implementation of XML parsing classes 
 */

#ifndef xmlparser_h
#define xmlparser_h

//Disable stupid VC++ warnings - we want to be portable, not use MS's nonstandard functions.
//I can check for buffer overflows myself on G++ - why not here too?
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <string>
#include <vector>

/*!
	@brief An attribute of an XML tag.
	
	Ported from old code; will be phased out in favor of an std::map in the future.
 */
class XmlTagAttribute
{
public:
	/*!
		@brief Default constructor
	 */
	XmlTagAttribute()
	{}
	
	/*!
		@brief Initializes an attribute
		
		@param n Name
		@param v Value
	*/
	XmlTagAttribute(std::string n,std::string v)
		: m_name(n)
		, m_value(v)
	{}

	/*!
		@brief Name of the attribute
	*/
	std::string m_name;
	
	/*!
		@brief Value of the attribute
	*/
	std::string m_value;
};

/*!
	@brief An XML node
 */
class XmlNode
{
public:
	XmlNode(
		std::string& xml,
		unsigned int& i,
		unsigned int& line);
	virtual ~XmlNode();

	/*!
		@brief Type of the node
	 */
	enum nodetype
	{
		NODETYPE_TAG,
		NODETYPE_TEXT,
		NODETYPE_COMMENT
	};

	/*!
		@brief Indicates if this object was loaded successfully
		
		@return False if EOF or other error encountered during load, true otherwise
	 */
	bool IsValid() const
	{ return m_bValid; }

	/*!
		@brief Gets the tag type
		
		@return Type of our tag ("<type>")
	 */
	const std::string& GetType() const
	{ return m_type; }

	/*!
		@brief Gets the type of the node
		
		@return Type of the node (NODETYPE_*)
	 */
	nodetype GetNodeType() const
	{ return m_nodetype; } 

	/*!
		@brief Gets our text
		
		@return Our body if we are a text or comment node
	 */
	const std::string& GetBody() const
	{ return m_body; }

	/*!
		@brief Gets the number of child nodes
		
		@return Number of children
	 */
	unsigned int GetChildCount() const
	{ return m_nodes.size(); }

	/*!
		@brief Gets a child node
		
		@param n Node index
		
		@return Const pointer to our nth node
	 */
	const XmlNode* GetChildNode(unsigned int n) const
	{ return m_nodes[n];  }

	/*!
		@brief Gets our attribute count
		
		@return Number of attributes
	 */
	unsigned int GetAttributeCount() const
	{ return m_attribs.size(); }

	/*!
		@brief Gets an attribute
		
		@param n Attribute index
		
		@return Our nth attribute
	 */
	const XmlTagAttribute& GetAttribute(unsigned int n) const
	{ return m_attribs[n]; }

	/*!
		@brief Gets an attribute name
		
		@param n Attribute index
		
		@return Name of our nth attribute
	 */
	const std::string& GetAttributeName(unsigned int n) const
	{ return m_attribs[n].m_name; }

	/*!
		@brief Gets an attribute value
		
		@param n Attribute index
		
		@return Value of our nth attribute
	 */
	const std::string& GetAttributeValue(unsigned int n) const
	{ return m_attribs[n].m_value; }

protected:
	/*!
		@brief List of our attributes
	 */
	std::vector<XmlTagAttribute> m_attribs;

	/*!
		@brief Type of the node
	 */
	nodetype m_nodetype;
	
	/*!
		@brief Body of the node (valid for text nodes and comments only)
	 */
	std::string m_body;

	/*!
		@brief Tag type (valid for tags only)
	 */
	std::string m_type;

	/*!
		@brief Indicates if we are a self-closing tag (e.g. br/)
	 */
	bool m_bSingleTag;
	
	/*!
		@brief Indicates if the node is valid and properly loaded
	 */
	bool m_bValid;

	/*!
		@brief Our nodes
	 */
	std::vector<XmlNode*> m_nodes;

	void EatSpaces(					//Skip spaces, updating line numbers as needed
		std::string& xml,
		unsigned int& i,
		unsigned int& line);
};

/*!
	@brief An XML parser
 */
class XmlParser
{
public:
	XmlParser(const char* fname);			//no default constructor - XmlParser objects must be initialized at creation
	XmlParser(std::string& xml);

private:
	/*!
		@brief Disallow copying
	 */
	XmlParser(const XmlParser& rhs)
	{ throw std::string("XmlParser objects cannot be copied"); }
	
	/*!
		@brief Disallow copying
	 */
	XmlParser& operator=(const XmlParser& rhs)
	{ throw std::string("XmlParser objects cannot be copied"); }

public:
	virtual ~XmlParser();

	/*!
		@brief Gets our root node
	 */
	XmlNode* GetRoot()
	{ return m_root; }

	/*!
		@brief Gets our doctype tag, if any
	 */
	XmlNode* GetDoctype()
	{ return m_doctype; }

	/*!
		@brief Gets our ?xml tag, if any
	 */
	XmlNode* GetXmlTag()
	{ return m_xmltag; }

protected:
	/*!
		@brief Pointer to root node
	 */
	XmlNode* m_root;
	
	/*!
		@brief Pointer to doctype
	 */
	XmlNode* m_doctype;
	
	/*!
		@brief Pointer to ?xml
	 */
	XmlNode* m_xmltag;

	void Load(std::string& xml);	//The core loading code
};

#endif
