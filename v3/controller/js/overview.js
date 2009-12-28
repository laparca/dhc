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
* overview.js - JavaScript utils for queue page                               *
*                                                                             *
******************************************************************************/

//Get the current timestamp
var d = new Date();
time = parseInt(d.getTime() / 1000);

//Set the update timer
setTimeout("checkUpdates(" + time + ")", 1000);

function checkUpdates(since)
{
	//Grab stuff from server and parse out lines
	stuff = ajaxDownload("?action=ajax&page=overview&from=" + since, "abc");
	lines = stuff.split("\n");
	
	//Grab time
	time = lines[0];
	
	//Process changes
	for(i=1; i<lines.length; i++)
	{
		//Get the line
		str = lines[i];
		
		//Parse it
		stuff = str.split(",");
		
		if(stuff[0] == "add")
		{
			//Check if we already have it
			hash = stuff[4];
			if(!document.getElementById(hash))
			{			
				//Format the new row
				nrow = "<tr id='" + hash + "'><td><a href='?page=queue&op=cancel&id=" + stuff[1] + "'>[Cancel]</a></td>";
				
				//Most stuff is just text
				for(j=2; j<stuff.length; j++)
					nrow += "<td>" + stuff[j] + "</td>";
					
				//Done	
				nrow += "</tr>\n";			

				//Insert it
				InsertRow("ovtable", nrow);
			}
		}
		else if(stuff[0] == "update")
		{
			//Make sure we have it
			hash = stuff[4];
			row = document.getElementById(hash);
			if(row)
			{			
				//Format the new row
				nrow = "<td><a href='?page=queue&op=cancel&id=" + stuff[1] + "'>[Cancel]</a></td>";
				for(j=2; j<stuff.length; j++)
					nrow += "<td>" + stuff[j] + "</td>";		

				//Update it
				row.innerHTML = nrow;
			}
		}
		else if(stuff[0] == "delete")
		{
			//Get the hash
			hash = stuff[1];
			row = document.getElementById(hash);
			if(row)
			{
				//Get parent and remove us
				parent = row.parentNode;
				parent.removeChild(row);
			}
		}		
	}
	
	//Get ready for next updates
	setTimeout("checkUpdates(" + time + ")", 2000);
}
