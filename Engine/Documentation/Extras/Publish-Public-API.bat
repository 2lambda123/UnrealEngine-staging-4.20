start /wait %~dp0\..\..\..\..\UnrealDocTool\Binaries\DotNET\unrealdoctool.exe API/* -s=%~dp0\..\Source -pathPrefix=%~dp0\..\Source -o=%~dp0\..\PublicRelease -lang=INT -publish=licensee -v=info -outputFormat=HTML -doxygenCache=%~dp0\..\XML -t=PublicAPI.html -nodb