/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* Copyright (C) 2010 Samuel Rodriguez Sevilla                                 *
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
#ifndef __PLUGIN_LOADER_H__
#define __PLUGIN_LOADER_H__

/*!
 *	@file Plugin.h
 *	
 *	@brief Plugin facilities.
 */
 
#define EXPORT extern "C"

#define BEGIN_PLUGIN(author) \
EXPORT  PluginFactory* GetPluginFactory() {\
	PluginFactory *rtn = new PluginFactory(author); \
#define END_PLUGIN() \
	return rtn; \
}

#define ADD_ALGORITHM(class, name, version) \
	rtn->AddFacility(new PluginFacilityAlgorithm<class>(name, version));
#define ADD_EXECUTOR(class, name, version) \
	rtn->AddFacility(new PluginFacilityExecutor<class>(name, version));

#define FACILITY_ALGORITHM 0
#define FACILITY_EXECUTOR  1


/*!
 * 
class PluginFacility {
	string       facility_name;
	int          facility_type;
	unsigned int facility_version;
public:
	unsigned int GetVersion() { return facility_version; }
	int          GetType() { return facility_type; }
	string       GetName() { return facility_name; }
	void*        GetInstance() = 0;
};

template<typename Algorithm>
class PluginFacilityAlgorithm : public PluginFacility {
public:
	PluginFacilityAlgorithm(string name, unsigned int version) :
	facility_name(name), facility_type(FACILITY_ALGORTHM), facility_version(verion)
	{}
	
	void* GetInstance() { return new Algorithm(); }
};

template<typename Executor>
class PluginFacilityAlgorithm : public PluginFacility {
public:
	PluginFacilityAlgorithm(string name,  unsigned int version) :
	facility_name(name), facility_type(FACILITY_EXECUTOR), facility_version(verion)
	{}
	
	void* GetInstance() { return new Executor(); }
};

class PluginFactory {
	vector<PluginFacility*> facilites;
	string _author;
public:
	PluginFactory(string author) : _author(author)
	{}
	string GetAuthorName()
	{
		return _author;
	}
	
	vector<PluginFacility*> GetFacilities()
	{
		return facilites;
	}
	
	void AddFacility(PluginFacility *facility)
	{
		facilites.push_back(facility);
	}
};

#endif

