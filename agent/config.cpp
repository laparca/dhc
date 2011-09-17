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

#include "debug.h"
#include <string>
#include <vector>
using namespace std;

/*!
 * @file config.cpp
 * @brief This file sets de values of global variables
 */

/* create unused names */
#define UNUSED UNUSED_(__COUNTER__)
#define UNUSED_(counter) UNUSED__(counter)
#define UNUSED__(counter) UNUSED_ ## counter

/* register a path inside the ConfigDirectories vector */
#define REGISTER_CONFIG_PATH(path) static _register_config_path UNUSED(path)


/*!
 * @brief List of directories where the cracker looks for plugins and other files
 */
vector<string> ConfigDirectories;

/*!
 * @brief Used to register paths inside ConfigDirectories
 */
struct _register_config_path {
 	_register_config_path(string str) {
		ConfigDirectories.push_back(str);
	}
};

REGISTER_CONFIG_PATH("./");
REGISTER_CONFIG_PATH("ptx/");
REGISTER_CONFIG_PATH("/usr/lib/cracker/ptx/");

/*!
 * @brief Sets the debug level if debug is active
 */
INIT_LOG(DEBUG);

