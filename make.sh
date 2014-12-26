#!/bin/sh

# if no arguments are given, we just want to build
if [ "${1}" = "" ]; then
	echo "Building dwm_status..."
	bam ${BAMFLAGS} -a Mode=Debug
elif [ "${1}" = "install" ]; then
	cp build/usr/local/bin/* /usr/local/bin/ &&
	echo "build/usr/local/bin/* -> /usr/local/bin/"
elif [ "${1}" = "uninstall" ]; then
	rm -f /usr/local/bin/dwm_status
	echo "Removing /usr/local/bin/dwm_status"
elif [ "${1}" = "clean" ]; then
	echo "removing build/"
	rm -fR build
else
	echo "Usage:"
	echo -e "  ${0}\t\t\tBuild"
	echo -e "  ${0} install\t\tInstall to /usr/local/bin"
	echo -e "  ${0} uninstall\t\tUninstall /usr/local/bin/dwm_status"
	echo -e "  ${0} clean\t\tClean build outputs"
fi

