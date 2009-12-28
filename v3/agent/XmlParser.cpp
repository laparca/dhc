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
	@file XmlParser.cpp
	
	@brief Implementation of XML parsing classes 
 */

#include "agent.h"
#include "XmlParser.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
using namespace std;

void reverse(char s[]);
char* my_itoa(int n, char s[]);

///////////////////////////////////////////////////////////////////////////////
//XmlParser

/*!
	@brief Loads an XML file
	
	@param fname Name of the file
 */
XmlParser::XmlParser(const char* fname)
: m_root(NULL)
, m_doctype(NULL)
, m_xmltag(NULL)
{
	//Open the file
	ifstream myfile(fname);
	if(!myfile)
		throw (string("Failed to open file ") + fname) ;
	myfile.seekg(0,ios::end);

	//Length is the BINARY length of the file. Text length may be less, so null fill the buffer in advance.
	unsigned int len = static_cast<unsigned int>(myfile.tellg());
	myfile.seekg(0,ios::beg);
	char* buf=new char[len+1];
	if(!buf)
		throw string("Memory allocation failed");
	memset(buf,0,len);

	//Read the data
	myfile.read(buf,len);

	//Do the actual parsing
	string xml(buf);
	Load(xml);

	//Clean up
	delete[] buf;
}

/*!
	@brief Loads XML from a string in memory
	
	@param xml XML data
 */
XmlParser::XmlParser(std::string& xml)
: m_root(NULL)
, m_doctype(NULL)
, m_xmltag(NULL)
{
	//Simple passthrough
	Load(xml);
}

/*!
	@brief Destroys the XML parser
 */
XmlParser::~XmlParser()
{
	//Clean up the stuff
	delete m_root;
	delete m_xmltag;
	delete m_doctype;
	m_root=NULL;
	m_xmltag=NULL;
	m_doctype=NULL;
}

/*!
	@brief Loads and parses XML
	
	@param xml XML data
 */
void XmlParser::Load(std::string& xml)
{
	unsigned int i=0;
	unsigned int line=1;	//use 1-based indexing for lines

	//Load tags
	while(i < xml.length())
	{
		//Create a tag
		XmlNode* pTag=new XmlNode(xml,i,line);
		if(!pTag)
			throw string("Memory allocation failed");

		//EOF?
		if(!pTag->IsValid())
		{
			delete pTag;
			break;
		}

		//Top level elements must be tags
		if(pTag->GetNodeType() != XmlNode::NODETYPE_TAG)
		{
			throw string("Top-level XML nodes must be tags");
		}

		//Process the tag
		if(pTag->GetType() == "?xml")
			m_xmltag=pTag;
		else if(pTag->GetType() == "!DOCTYPE")
			m_doctype=pTag;
		else
		{
			//Save the root tag and quit
			m_root=pTag;
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//XmlNode

/*!
	@brief Loads an XML node
	
	@param xml File being parsed
	@param i Parsing index (updated at return)
	@param line Current line (updated at return, displayed for errors)
 */
XmlNode::XmlNode(std::string& xml,unsigned int& i,unsigned int& line)
: m_nodetype(NODETYPE_TAG)
, m_bSingleTag(false)
, m_bValid(false)
{
	unsigned int len = xml.length();

	//Skip space before tag start
	EatSpaces(xml,i,line);

	//EOF? Return
	if(i>=xml.length())
		return;

	//Next character must be an opening angle bracket unless we're a text node.
	if(xml[i] == '<')
	{
		//Skip <
		i++;

		//Read tag type
		while(i<len && !isspace(xml[i]) && xml[i] !='>')
		{
			m_type += xml[i];
			i++;
		}

		//Are we an ending tag? Can't have children
		if(m_type[0]=='/')
			m_bSingleTag=true;

		//Are we a comment?
		if(m_type=="!--")
		{
			m_nodetype=NODETYPE_COMMENT;

			//Read body
			while(i<len - 3)
			{
				//Bump line count
				if(xml[i]=='\n')
					line++;

				//Done?
				if(xml[i]=='-' && xml[i+1]=='-' && xml[i+2]=='>')
					break;

				//Add to body
				m_body += xml[i];
				i++;
			}
			i+=3;

#ifdef DEBUG_VERBOSE_PARSING
			cout << "Comment: " << m_body.c_str() << endl;
#endif
		}

		//EOF? Shouldn't happen but make sure...
		if(i>=len)
			throw string("Unexpected end of file found!");

		//DOCTYPE is special
		if(m_type=="!DOCTYPE")
		{
#ifdef DEBUG_VERBOSE_PARSING
			cout << "Ignoring DOCTYPE declaration" << endl;
#endif

			//Read it
			while(i<len && xml[i] != '>')
			{
				if(xml[i]=='\n')		//Newline? Bump line number
					line++;
				i++;
			}

			//EOF? Shouldn't happen but make sure...
			if(i>=len)
				throw string("Unexpected end of file found!");

			//Skip the '>'
			i++;

			//We're done
			m_bSingleTag=true;
			m_bValid = true;
			return;
		}

		//Calculate ending tag type
		if(m_nodetype==NODETYPE_TAG)
		{
			if(m_type[m_type.length()-1]=='/')
			{
				m_type.replace(0,'/','\0');
				m_bSingleTag=true;
				i--;
			}

#ifdef DEBUG_VERBOSE_PARSING
		cout << "Tag: " << m_type.c_str() << endl;
#endif
	
			//Skip space between tag name and attributes
			EatSpaces(xml,i,line);

			//Read attributes
			if(m_type[0]!='/')
			{
				while(xml[i] != '/' && xml[i] != '>')
				{
					if(xml[i] == '?' && m_type=="?xml")
						break;

					//Read name
					string name;
					while(i<len && !isspace(xml[i]) && xml[i]!='=')
					{
						name += xml[i];
						i++;
					}

					//Skip space...
					EatSpaces(xml,i,line);
					
					//EOF? Shouldn't happen but make sure...
					if(i>=len)
						throw string("Unexpected end of file found!");

					//Next character must be an "=". Make sure of that...
					if(xml[i] != '=')
					{
						char buf[32];
						string err = "Syntax error: expected '=', found '";
						err += xml[i];
						err += "' on line ";
						err += my_itoa(line,buf);
						throw err;
					}
					i++;

					//Skip space...
					EatSpaces(xml,i,line);
					
					//EOF? Shouldn't happen but make sure...
					if(i>=len)
						throw string("Unexpected end of file found!");

					//Use strict XML syntax - attribs must be enclosed by quotes.
					char quotestart = xml[i];
					if(quotestart != '"' && quotestart != '\'')
					{
						char buf[32];
						string err = "Syntax error: expected single or double quotation mark, found '";
						err += quotestart;
						err += "' on line ";
						err += my_itoa(line,buf);
						throw err;
					}
					i++;

					//Read attribute value
					string value;
					while(i<len && xml[i]!=quotestart && xml[i] != '\n')
					{
						value += xml[i];
						i++;
					}

					//Skip the quote
					if(quotestart != '"' && quotestart != '\'')
					{
						char buf[32];
						string err = "Syntax error: expected single or double quotation mark, found '";
						err += quotestart;
						err += "' on line ";
						err += my_itoa(line,buf);
						throw err;
					}
					i++;

					//EOF? Shouldn't happen but make sure...
					if(i>=len)
						throw string("Unexpected end of file found!");

					//Skip space...
					EatSpaces(xml,i,line);

					//Add to attribute list
					m_attribs.push_back(XmlTagAttribute(name,value));

					#ifdef DEBUG_VERBOSE_PARSING
						cout << "   Attribute: " << name.c_str() << " = " << value.c_str() << endl;
					#endif
				}
			}
	
			//Three possible options:
			//Ending angle bracket - end of start tag
			//? - end of ?xml tag
			//Slash - end of singular tag
			if( (xml[i] == '?' && m_type=="?xml") || (xml[i] == '/') )
			{
				//Skip closing mark
				i++;

				//We have no children
				m_bSingleTag=true;
			}

			//EOF? Shouldn't happen but make sure...
			if(i>=len)
				throw string("Unexpected end of file found!");

			//Must be a >
			if(xml[i] != '>')
			{
				char buf[32];
				string err = "Syntax error: expected '>', found '";
				err += xml[i];
				err += "' on line ";
				err += my_itoa(line,buf);
				throw err;
			}
			i++;

			//Read children
			if(!m_bSingleTag)
			{
				bool bEnded=false;

				string endtype = string("/") + m_type;

				while(true)
				{
					//Skip space before tag start
					EatSpaces(xml,i,line);

					//EOF? Shouldn't happen but make sure...
					if(i>=len)
						break;

					//Create a node
					XmlNode* pTag=new XmlNode(xml,i,line);
					if(!pTag)
						throw string("Memory allocation failed");

					//EOF?
					if(!pTag->IsValid())
					{
						delete pTag;
						break;
					}

					//Is it our end tag?
					if(pTag->GetNodeType() == NODETYPE_TAG && pTag->GetType() == endtype)
					{
						//Yes - we're done
						delete pTag;
						bEnded = true;
						break;
					}
					
					//Add to our child list
					m_nodes.push_back(pTag);
				}

				//Make sure we were terminated properly
				if(!bEnded)
				{
					char buf[32];
					string err = "Syntax error: unexpected end of file while looking for\nclosing ";
					err += m_type;
					err += " tag (opening tag is on line ";
					err += my_itoa(line,buf);
					err += ")";
					throw err;
				}
			}
		}
	}

	//Text node - read us
	else
	{
		m_nodetype=NODETYPE_TEXT;

		//Read body
		while(i<len)
		{
			//Bump line count
			if(xml[i]=='\n')
				line++;

			//Done?
			if(xml[i]=='<')
				break;

			//Add to body
			m_body += xml[i];
			i++;
		}

#ifdef DEBUG_VERBOSE_PARSING
		cout << "Text node: " << m_body.c_str() << endl;
#endif
	}

	//We're now a well-formed tag
	m_bValid = true;
}

/*!
	@brief Destroys an XML node
 */
XmlNode::~XmlNode()
{
	//Clean up
	for(unsigned int i=0;i<m_nodes.size();i++)
		delete m_nodes[i];
}

/*!
	@brief Skip spaces, updating line numbers as needed
	
	@param xml String being parsed
	@param i Parsing index (updated after return)
	@param line Line number (updated after return)
 */
void XmlNode::EatSpaces(std::string& xml,unsigned int& i,unsigned int& line)
{
	unsigned int len = xml.length();
	for(;i<len;i++)
	{
		if(xml[i]=='\n')		//Newline? Bump line number
			line++;

		if(!isspace(xml[i]))	//Not space? Quit
			break;
	}
}


/*!
	@brief K&R implementation of itoa for portability to g++
	
	Slightly modified - "return s" added at end; return type changed from void to char*
	
	@param n Number being converted
	@param s Output buffer (size not checked, must be large enough to hold decimal value of INT_MAX)

	@return Pointer to s[0]
*/
char* my_itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
	return s;
} 

/*!
	@brief Reverses a string in-place
	
	@param s String to be reversed
 */
void reverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
