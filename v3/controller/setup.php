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
	@file setup.php
	
	@brief generates a config.php and populates an initial database.
*/

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en" dir="ltr">
<head>
<title>RPISEC Hash Cracker : Initial Setup</title>
</head>
<body>
<?php

//If we have already been configured, quit (security check)
if(file_exists("config.php") && trim(file_get_contents("config.php")) != '')
{
	die("Setup already completed, please make all future changes in config.php!");
}

//Sanity check
if(PHP_INT_SIZE < 8)
{
	die("The current version of the controller requires a 64-bit version of PHP.");
}

$action=$_GET['action'];

if ($action == "y")
{
	$dbserver = $_POST['dbserver'];
	$uname = $_POST['uname'];
	$pass = $_POST['pass'];
	$db = $_POST['db'];

	if ($uname!="" && $pass!="" && $db!="")
	{
		//TODO: Update this for new database schema
		
		$sql = mysql_connect ($dbserver, $uname, $pass);
		mysql_select_db($db, $sql);
		$query1 = "CREATE TABLE IF NOT EXISTS `cracks` (
		`id` int(11) NOT NULL auto_increment,
		`algorithm` varchar(32) collate latin1_general_ci NOT NULL,
  		`charset` varchar(256) collate latin1_general_ci NOT NULL,
		`hash` text collate latin1_general_ci NOT NULL,
		`maxlen` int(11) NOT NULL,
		`nextwu` varchar(64) collate latin1_general_ci NOT NULL,
		`started` int(11) NOT NULL,
		`expiration` int(11) NOT NULL,
		`priority` int(11) NOT NULL,
		`disposition` varchar(6) collate latin1_general_ci NOT NULL,
		`collision` varchar(255) collate latin1_general_ci NOT NULL,
		`updated` int(11) NOT NULL,
		PRIMARY KEY  (`id`)
		) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci AUTO_INCREMENT=1";

		$query2 = "CREATE TABLE IF NOT EXISTS `stats` (
		`updated` int(11) NOT NULL,
		`device` varchar(255) collate latin1_general_ci NOT NULL,
		`time` int(11) NOT NULL,
		`speed` float NOT NULL
		) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;";

		$query3 = "CREATE TABLE IF NOT EXISTS `workunits` (
		`id` int(11) NOT NULL auto_increment,
		`crack` int(11) NOT NULL,
		`hostname` varchar(255) collate latin1_general_ci NOT NULL,
		`devtype` varchar(16) collate latin1_general_ci NOT NULL,
		`devid` int(11) NOT NULL,
		`start` varchar(255) collate latin1_general_ci NOT NULL,
		`end` varchar(255) collate latin1_general_ci NOT NULL,
		`started` int(11) NOT NULL,
		`expiration` int(11) NOT NULL,
		PRIMARY KEY  (`id`)
		) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci AUTO_INCREMENT=1";
		
		$query4 = "
		CREATE TABLE IF NOT EXISTS `history` (
		  `time` int(11) NOT NULL,
		  `speed` float NOT NULL,
		  UNIQUE KEY `time` (`time`,`speed`)
		) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
		";


		if(mysql_query($query1) && mysql_query($query2) && mysql_query($query3) && mysql_query($query4))
		{
			$dbserver = addslashes($dbserver);
			$uname = addslashes($uname);
			$pass = addslashes($pass);
			$db = addslashes($db);
			
			file_put_contents(
				"config.php", 
				"<?php\n\$dbserver = '".$dbserver."';\n\$uname = '".$uname."';\n\$pass = '".$pass."';\n\$dbname = '".$db."';\n?>"
				);
			
			?>
			<p>
			Setup complete! <a href='index.php'>Continue</a>
			</p>
			<?
		} 
		else
		{ 
			echo "Error: "; echo mysql_error();
		}

		mysql_close($sql);
	}
	else
	{
		echo "Error - one or more fields is empty";
	}
}


else
{
	$fname = "config.php";
	$handle = @fopen($fname, 'a');		// check if we can write to config.php without altering anything that might be there already
	if ($handle)
	{	
		fclose($handle);
		?>
				
		<h1>Initial Setup</h1>
		<form method="POST" action="setup.php?action=y" enctype="multipart/form-data">
		<table>
		<tr><td>Server:</td><td><input type="text" name="dbserver" size="25" value=""></td></tr>
		<tr><td>Username:</td><td><input type="text" name="uname" size="25" value=""></td></tr>
		<tr><td>Password:</td><td><input type="password" name="pass" size="25" value=""></td></tr>
		<tr><td>Database:</td><td><input type="text" name="db" size="25" value=""></td></tr>
		</table>
		<input type="submit" value="Save Changes"/>
		</form>
		
		<?php
	}
	else
	{
		die("Could not open config.php for writing - please create an empty file with the appropriate permissions and try again.");
	}
}
?>
</body>
</html>
