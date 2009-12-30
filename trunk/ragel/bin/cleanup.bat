@echo off
cd ..\Ragel
del /Q /A:H Ragel.suo
del /Q Ragel.ncb
del /Q .\Debug\*.*
del /Q .\x64\Debug\*.*
del /Q .\Kelbt\*.user
del /Q .\Kelbt\Debug\*.*
del /Q .\Kelbt\Release\*.*
del /Q .\Kelbt\x64\Debug\*.*
del /Q .\Kelbt\x64\Release\*.*
del /Q .\Ragel\*.user
del /Q .\Ragel\Debug\*.*
del /Q .\Ragel\Release\*.*
del /Q .\Ragel\x64\Debug\*.*
del /Q .\Ragel\x64\Release\*.*
