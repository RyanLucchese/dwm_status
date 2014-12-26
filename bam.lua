-- input paths
sourcePath = "src/"

-- output paths
outputPath = "build/"
objectPath = outputPath .. "obj/"
outputBinPath = outputPath .. "usr/local/bin/"


-- common settings
function NewDwmStatusSettings()
	local dwm_status_settings = NewSettings()

	-- use clang (TODO: bootstrap this properly with compiler detection)
	dwm_status_settings.cc.exe_c = "clang"
	dwm_status_settings.cc.exe_cxx = "clang++"
	dwm_status_settings.link.exe = "clang"
	dwm_status_settings.dll.exe = "clang"

	-- full warning level, treat warnings as errors, position independent code
	dwm_status_settings.cc.flags:Add("-Wall", "-Wextra", "-Werror", "-fPIC")

	-- set optimization level
	if (dwm_status_settings.debug > 0) then
	        dwm_status_settings.cc.flags:Add("-O0")
	else
	        dwm_status_settings.cc.flags:Add("-O3")
	end

	-- set version string
	dwm_status_settings.cc.defines:Add("PACKAGE_VERSION=\\\"`git describe --always --tags --dirty=-modified --abbrev=10`-`git rev-parse --abbrev-ref HEAD`\\\"")

	-- output objects to obj directory
	dwm_status_settings.cc.Output = function(dwm_status_settings, input)
	        return objectPath .. PathFilename(PathBase(input))
	end

	-- add X11 include paths
	dwm_status_settings.cc.includes:Add("/usr/local/include")

	-- add X11 library paths
	dwm_status_settings.link.libpath:Add("/usr/local/lib")

	-- link to X11 library
	dwm_status_settings.link.libs:Add("X11")

	return dwm_status_settings
end

-- set up output directories
ExecuteSilent("mkdir -p " .. outputBinPath)

-- set up compiler settings
settings = NewDwmStatusSettings()

-- collect sources and compile
objects = Compile(settings, sourcePath .. "dwm_status.c")

-- make shared objects
dwm_settings = Link(settings, outputBinPath .. "dwm_status", objects)

