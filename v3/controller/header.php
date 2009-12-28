<?php
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
*******************************************************************************/

/*!
	@file header.php
	
	@brief Common stuff used by both controller back-end and UI
 */

mysql_connect($dbserver, $uname, $pass) or die('DB error');
mysql_select_db($dbname) or die('DB error');

$g_dberrors = true;		//TODO: turn off in release build
$action = $_GET['action'];

//TODO: implement login for hash submission

//Expire all obsolete cracks
$time = time();
dbquery("UPDATE `cracks` SET `active` = '0' WHERE `active` = '1' AND `expiration` < '$time'");

/*!
	@brief Executes a database query
	
	If an error occurs, the script is terminated. If $g_dberrors is set to true, a detailed diagnostic message suitable for
	debugging is displayed. If not, a generic "error" message is shown.
	
	@param $q The query to execute
	
	@return MySQL result set
 */
function dbquery($q)
{
	global $g_dberrors;
	$r = mysql_query($q);
	if($r)
		return $r;
	else if($g_dberrors)
		die($q . "<br/>: " . mysql_error());
	else
		die("Error");
}

/*!
	@brief Renders the page given a template array
	
	This function loads the page template from data/template.html, applies template transformations using GetTemplatedPage(),
	and prints the result.
	
	@param $tarr Parameter array
 */
function RenderPage($tarr)
{
	echo GetTemplatedPage('data/template.html', $tarr);
}

/*!
	@brief Applies template transformations to a file
	
	The file specified by $file is loaded and searched for HTML comments of the form <!-- [[x]] -->. All occurrences of them are
	replaced by the string in $tarr['x'].
	
	@param $file Name of the template file
	@param $tarr Parameter array
	
	@return The output string
 */
function GetTemplatedPage($file, $tarr)
{
	//Get the page
	$html = file_get_contents($file);
	
	//Templating
	foreach($tarr as $id => $val)
		$html = str_replace("<!-- [[$id]] -->", $val, $html);
		
	//and return
	return $html;
}
?>
