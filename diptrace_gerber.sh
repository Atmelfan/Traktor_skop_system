#!/usr/bin/env bash
echo "Board name: $1"

function error {
	echo -e "\e[00;31m$1\e[00m"
}

function move {
	cp $1 $2 2> /dev/null
}

echo "Deleting export..."
rm -rf "export"
mkdir "export"

echo "Copying files..."
move "Top.gbr" "export/$1.gtl"
if [ $? -eq 1 ]; then
	error " No top layer!"
fi
move "Bottom.gbr" "export/$1.gbl"
if [ $? -eq 1 ]; then
	error " No bottom layer!"
fi
move "TopMask.gbr" "export/$1.gts"
if [ $? -eq 1 ]; then
	error " No top mask!"
fi
move "BottomMask.gbr" "export/$1.gbs"
if [ $? -eq 1 ]; then
	error " No bottom mask!"
fi
move "TopSilk.gbr" "export/$1.gto"
if [ $? -eq 1 ]; then
	error " No top silkscreen!"
fi
move "BottomSilk.gbr" "export/$1.gbo"
if [ $? -eq 1 ]; then
	error " No bottom silkscreen!"
fi
move "BoardOutline.gbr" "export/$1.gko"
if [ $? -eq 1 ]; then
	error " No board outline!"
fi
move "Through.drl" "export/$1.xln"
if [ $? -eq 1 ]; then
	error " No drill file!"
fi
echo "Deleting unused files..."
rm *.gbr *.drl  2> /dev/null
echo "Creating ZIP file..."
zip -r "$1.zip" "export/"
echo "Done!"
