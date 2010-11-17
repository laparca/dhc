/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* Copyright (C) 2010 Samuel Rodríguez Sevilla
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
 *	@file Algorithm.h
 *	
 *	@brief Declares the algorith factory facility.
 */

#ifndef ALGORITHM_FACTORY_H
#define ALGORITHM_FACTORY_H

#include "Algorithm.h"

/*!
 *	@def INIT_ALGORITHMS
 *	@brief Initializes the algorithm system.
 *
 *	This method/macro has to be used in the global section of a source file
 *	for initilize the internal algorithm system.
 */
#define INIT_ALGORITHMS() AlgorithmFactory::algorithm_list AlgorithmFactory::vAlgorithms


/*!
 *	@class AlgorithmFactory
 *	@brief AlgorithmFactory is the utility to manage algorithms.
 *
 *  With AlgorithmFactory we can instance new algorithms.
 */
class AlgorithmFactory
{
/*!
 *	\defgroup Algorithm administration system
 *	\page Algorithm Organization of algorithm
 *
 *	Algorithms can be dynamic loaded or can be registered in compilation time
 *	and to achieve this purpose a algorithm management system is needed.
 */
/*@{*/
public:
	typedef vector<Algorithm *> algorithm_list;
	typedef vector<Algorithm *>::iterator algorithm_iterator;

private:
	/*! 
	 *	@brief The list of algorithms registered in the system.
	 *
	 *	Stores the list of algorithms registered in the system for grand
	 *	the hability of query them. This attribute has to be initialized in
	 *	one source file to be accesible. For that initialization you have to
	 *	declare it as follows:
	 *	Algorithm::algorithm_list Algorithm::vAlgorithms;
	 *	You can also use the macro INIT_ALGORITHMS()
	 */
	static algorithm_list vAlgorithms;

public:
	/*!
	 *	@brief Returns the list of algorithms
	 *
	 *	When a functionallity that implies algorithms is need but this
	 *	functionallity do not exists a this method help us. It returns the
	 *	list of algorithms and then it can be used to do the desire
	 *	functionallity.
	 *
	 *	@return The algorithm list 
	 */
	static algorithm_list& GetAlgorithmList()
	{
		return vAlgorithms;
	}
	
	/*!
	 *	@brief Register an algorithm in a internal list used for
	 *	control the algorithms.
	 *	@param pAlgorithm The algorithm to register
	 */
	static void RegisterAlgorithm(Algorithm *pAlgorithm)
	{
		vAlgorithms.push_back(pAlgorithm);
	}
	/*!
	 *	@brief Returns the algorithm whos name is the user specified
	 *	@param name The algorithm searched name
	 *	@return The algorithm or NULL if it does not exists
	 */
	static Algorithm *GetAlgorithm(const string& name)
	{
		algorithm_iterator end = vAlgorithms.end();
		for(algorithm_iterator it = vAlgorithms.begin(); it != end; it++)
		{
			if((*it)->GetName() == name)
				return *it;
		}
		return NULL;
	}
	/*!
	 *	@brief Return the name of the registered algorithms
	 *	@return The list of names
	 */
	static vector<string> GetAlgorithmNames();
	/*!
	 *	@brief Returns the list of algorithms that matches with the
	 *	function func.
	 *
	 *	There are lots of situations where the normal functions are not
	 *	suficient to obtain the desire algorithm. To solve this problem
	 *	GetAlgoritms function use a function or a functor to look inside
	 *	each algorithm and to select each algorithm needed. The functor
	 *	only has to implements the () operator with one parameter of
	 *	Algorithm * type. This functor o functions has to return true
	 *	if the algorithm is selected and false in other case.
	 *
	 *	@param func Function or functor needed to get the algorithms
	 *	@return The algorithm list.
	 */
	template<typename Func>
	static algorithm_list GetAlgorithms(Func func)
	{
		algorithm_list result;
		algorithm_iterator end = vAlgorithms.end();
		for(algorithm_iterator it = vAlgorithms.begin(); it != end; it ++)
			if(func(*it))
				result.push_back(*it);
		return result;
	}
	static void Test();
	/*@}*/
};

#endif
