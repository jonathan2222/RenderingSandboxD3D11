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
		"%{includeDir.tinyobj}",
		"%{includeDir.assimp}"
	}

	--libdirs = 
	--{
	--	"%{libDir.assimp}"	
	--}

	filter "configurations:Debug"
		links
		{
			"glfw",
			"imgui",
			"%{libDir.assimp}/assimp-vc142-mtd.lib",
			"%{libDir.assimp}/zlibstaticd.lib",
			"%{libDir.assimp}/dracod.lib"
		}

	filter "configurations:Release"
		links
		{
			"glfw",
			"imgui",
			"%{libDir.assimp}/assimp-vc142-mt.lib",
			"%{libDir.assimp}/zlibstatic.lib",
			"%{libDir.assimp}/draco.lib"
		}
	filter "configurations:Production"
		links
		{
			"glfw",
			"imgui",
			"%{libDir.assimp}/assimp-vc142-mt.lib",
			"%{libDir.assimp}/zlibstatic.lib",
			"%{libDir.assimp}/draco.lib"
		}
	filter {}

project "*"