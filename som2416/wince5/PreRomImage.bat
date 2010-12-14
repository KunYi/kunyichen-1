@ECHO OFF
@ECHO	2416 starts preromimage.bat...
if "%IMGMULTIXIP%"=="1" (
	cscript //E:jscript  //NOLOGO %_FLATRELEASEDIR%\makexip.js -bib  %_FLATRELEASEDIR%\ce.bib
)


@ECHO ON