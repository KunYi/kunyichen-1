// *******************************************************************************************
//
//  makexip.js
//
//  Usage:    cscript //nologo makexip.js <-bib>  <file to process>
//
//  Copyright Microsoft Corporation, All Rights Reserved.
//
// *******************************************************************************************

var shell      = WScript.CreateObject("WScript.Shell");

var fileSystem = WScript.CreateObject("Scripting.FileSystemObject");

var environmentVariables = shell.Environment("Process");

var argError = "Incorrect arguments\nExample:\ncscript //nologo makexip.js <-bib>  <file to process>";

var bibSection = ""; // defines where we are in the ce.bib file (FILES section, or MODULES section, etc)

var bibTempFile = "bibfiletmp.tmp";
var bibTempOutput;

var releaseDir = environmentVariables("_FLATRELEASEDIR");

try
{
	if (WScript.Arguments(0) == "-bib")
		bibTempOutput = fileSystem.CreateTextFile(bibTempFile);
	else
		throw "-bib must be the first argument";
	var file = WScript.Arguments(1);
	var input = fileSystem.OpenTextFile(file);

	ProcessFile(input);

	input.close();
	// finalize and cleanup
	bibTempOutput.close();
	fileSystem.CopyFile(bibTempFile, file, true);
	fileSystem.DeleteFile(bibTempFile);
}
catch(err)
{
    if (err != argError)
    {
        WScript.echo("Makexip.js: "+err);
        throw err;
    }
    else
    {
        WScript.echo(err);
    }
    WScript.Quit(-1);
}

// takes the input line and processes for BibMode.
// this is the guts of the Bib processing
function ProcessBibLine (line)
{
//MODULES
//  Name            Path                                           Memory Type
// --------------  ---------------------------------------------  -----------
//   nk.exe          E:\WINCE420\PUBLIC\MyPlat0\RelDir\CENTRALITY_ATLAS_DEV_BOARD_ARMV4Release\kern.exe                   TINYNK  SH

	var tokens = line.match(/\s*(\S+)\s+\S+\\(\S+)(\s+)(\s*)(\S*)(\s*)(\S*)/);

	if (tokens != null) {
		var filename0 = tokens[0];	// whole line
		var filename1 = tokens[1];
//		var filename2 = tokens[2];
//		var filename3 = tokens[3];
//		var filename4 = tokens[4];
		var filename5 = tokens[5];	// Property NK
//		var filename6 = tokens[6];
//		var filename7 = tokens[7];
//		var filename8 = tokens[8];

		if (filename1 == "nk.exe"       ||
		    filename1 == "kd.dll"       ||
		    filename1 == "osaxst0.dll"  ||
		    filename1 == "osaxst1.dll"  ||
		    filename1 == "hd.dll"       ||
		    filename1 == "coredll.dll"  ||
		
		    filename1 == "filesys.exe"  ||
		    filename1 == "fsdmgr.dll"   ||
		    filename1 == "relfsd.dll"   ||
		    filename1 == "pm.dll"       ||
		    filename1 == "devmgr.dll"   ||
		    
		    filename1 == "device.exe"   ||
			filename1 == "ceddk.dll"    ||
			filename1 == "busenum.dll"  ||
			filename1 == "utldrv.dll"   ||
		    filename1 == "flashdrv.dll" ||
		    filename1 == "newflashdrv.dll" ||
		    filename1 == "mspart.dll"   ||
		    filename1 == "binfs.dll"    ||
		    filename1 == "fatfsd.dll"   ||
		    filename1 == "fatutil.dll"  ||
		    filename1 == "diskcache.dll" ||
		    
		    // DAVID ADD FOR 2416
		    filename1 == "BIBDrv.dll" ||
		    
		    

            filename1 == "initobj.dat"  ||
            filename1 == "initdb.ini"   ||

			filename1 == "regenum.dll"  ||
		    filename1 == "ne2000isr.dll" ||
		    filename1 == "wince.nls"    ||
		    filename1 == "tahoma.ttf"   ||
		    filename1 == "default.fdf"  ||
		    filename1 == "default.hv"   ||
		    filename1 == "boot.hv"      ||
		    filename1 == "user.hv")
    	{
			line = line.replace(/(\s+)(\s*)NK(\S*)(\s*)(\S*)/, tokens[3] + " TINY" + tokens[4] + tokens[5] + tokens[6] + tokens[7]);
		}
	}
	else if (line.search(/^FILES$/) != -1) {
		WScript.Echo("FILES SECTION ");
		bibSection = "FILES";
	}
	else if (line.search(/^MODULES$/) != -1) {
		WScript.Echo("MODULES SECTION");
		bibSection = "MODULES";
	}

	bibTempOutput.WriteLine(line);
}

// main function that processes the input file.
// this function calls the appropriate function to handle
// each line of the input file based on the mode the script
// was invoked
function ProcessFile (input)
{
	while (!input.AtEndOfStream) {
		var line = input.ReadLine ();
		ProcessBibLine (line);
	}
}

function Warning (msg)
{
	WScript.Echo("MXIP Warning: " + msg);
}
