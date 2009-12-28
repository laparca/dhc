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
*******************************************************************************
*                                                                             *
* ajaxcore.js - AJAX helper routines                                          *
*                                                                             *
******************************************************************************/

function ajaxDownload(url, func)
{	
	var request = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject("MSXML2.XMLHTTP.3.0");
	if(!request)
	{
		alert("AJAX not supported");
	}

	//Request to get the text
	request.open("GET",url,false);	//TODO: asynchronous
	var ifModifiedSince=new Date(0);
	request.setRequestHeader("If-Modified-Since", ifModifiedSince);
	request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
	request.send("");
	
	//Process the results
	return request.responseText;
}

function InsertRow(tbl, nrow)
{
	//Marker for new rows
	nmarker = "<!-- {{newrows}} -->";

	//Append the new row (keeping comment in place so we can add multiple rows)
	ov = document.getElementById(tbl);
	ov.innerHTML = ov.innerHTML.replace(nmarker, nmarker + nrow);
}
