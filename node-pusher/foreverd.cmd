@IF EXIST "%~dp0\node.exe" (
  "%~dp0\node.exe"  "%~dp0\node_modules\forever\bin\foreverd" %*
) ELSE (
  node  "%~dp0\node_modules\forever\bin\foreverd" %*
)