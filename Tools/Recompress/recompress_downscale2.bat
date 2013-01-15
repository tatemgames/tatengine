lz4 -d data0.lz4 data0.zip
unzip -o data0.zip -d data0
del data0.zip
ren data0.lz4 data0.lz4_b
cd data0
for /f %%i in ('dir *.tga /b') do convert %%i -set colorspace RGB -channel rgba -separate -swap 0,2 -combine -filter Lanczos -resize 1024 %%i.png
del *.tga
for /f "delims==" %%F in ('dir /b *.png') do ren "%%~nxF" "%%~nF"
ren *.tga *.png
..\zip -0 ..\data0.zip *.*
cd ..
del /f /q data0/*.*
rmdir /s /q data0
lz4 -c1 data0.zip data0.lz4
del data0.zip