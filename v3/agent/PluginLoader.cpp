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
#include "PluginLoader.h"
#include "debug.h"
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <string>

using namespace std;

extern vector<string> ConfigDirectories;

void PluginLoader::Load()
{
	DO_ENTER("PluginLoader", "Load");
	
	for (vector<string>::iterator dir_it = ConfigDirectories.begin() ; dir_it != ConfigDirectories.end(); ++dir_it)
	{
		DIR* dir;
		struct dirent* ent;
		
		dir = opendir(dir_it->c_str());

		/* No directory? Try next */
		if(dir == NULL) continue;

		while((ent = readdir(dir)) != NULL)
		{
			string file_name(ent->d_name);
			string file_ext(".aplug.so");
			if(file_name.substr(file_name.size() - file_ext.size(), file_ext.size()) == file_ext)
			{
				PluginLoader::Load(*dir_it + file_name);
			}
		}
		closedir(dir);
	}
}

void PluginLoader::Load(string file)
{
	DO_ENTER("PluginLoader", "Load");
	DO_MESSAGE(string("Try to load ") + file);
}