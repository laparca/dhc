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
#ifndef DEBUG_H
#define DEBUG_H

#define ERROR    0
#define WARINING 1
#define MESSAGE  2
#define LOG      3
#define DEBUG    4

#ifdef _DEBUG
#   define B_LOG(level, i, str)  do { if((level) <= _log_level) cout << "[" << i << "] " << (str) << endl; } while(0)
#   define DO_ENTER(class, str)     _local_method_log(class, str)

#   define DO_ERROR(str)            B_LOG(ERROR, "ERROR", str)
#   define DO_WARNING(str)          B_LOG(WARNING, "WARNING", str)
#   define DO_MESSAGE(str)          B_LOG(MESSAGE, "MESSAGE", str)
#   define DO_LOG(str)              B_LOG(LOG, "LOG", str)
#   define DO_DEBUG(str)            B_LOG(DEBUG, "DEBUG", str)

#   define INIT_LOG(lvl)            unsigned int _log_level = (lvl)

extern unsigned int _log_level;

class _local_method_log
{
public:
	_local_method_log(string cl, string method) : m_cl(cl), m_method(method)
	{
		B_LOG(DEBUG, "ENTER", m_cl + "::" + m_method);
	}
	
	~_local_method_log()
	{
		B_LOG(DEBUG, "EXIT", m_cl + "::" + m_method);
	}
private:
	string m_cl;
	string m_method;
};

#else
#   define B_LOG(level, i, str)
#   define DO_ENTER(class, str)
#   define DO_ERROR(str)
#   define DO_WARNING(str)
#   define DO_MESSAGE(str)
#   define DO_LOG(str)
#   define DO_DEBUG(str)
#   define INIT_LOG(lvl)

#endif /* _DEBUG */


#endif