function GetFiles(path)
	return path .. "**.hpp", path .. "**.h", path .. "**.cpp", path .. "**.c"
end

project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++latest"
	systemversion "latest"

    -- Pre-Compiled Headers
    pchheader "PreCompiled.h"
    pchsource "Src/PreCompiled.cpp"

    forceincludes
	{
		"PreCompiled.h"
	}

	-- Targets
	targetdir ("%{wks.location}/Build/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/Build/obj/" .. outputdir .. "/%{prj.name}")

    -- Files to include
	files { GetFiles("Src/") }

	--Includes
    includedirs { "Src" }

    sysincludedirs
	{
		"%{includeDir.glm}",
		"%{includeDir.imgui}",
		"%{includeDir.spdlog}",
		"%{includeDir.stb}",
		"%{includeDir.json}",
		"%{includeDir.glfw}",
		"%{includeDir.tinyobj}"
	}

	links
	{
		"glfw",
		"imgui"
	}

project "*"