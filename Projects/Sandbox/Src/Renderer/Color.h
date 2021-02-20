#pragma once

#include "Utils/Maths.h"

namespace RS
{
	class Color
	{
	public:
		inline static const glm::vec3 RED	= glm::vec3(1.f, 0.f, 0.f);
		inline static const glm::vec3 GREEN	= glm::vec3(0.f, 1.f, 0.f);
		inline static const glm::vec3 BLUE	= glm::vec3(0.f, 0.f, 1.f);
		inline static const glm::vec3 WHITE	= glm::vec3(1.f, 1.f, 1.f);
		inline static const glm::vec3 BLACK	= glm::vec3(0.f, 0.f, 0.f);

		/*
		Converts a color in HSB space into the sRGB space.

		Arguments:
			hue			[0, 1] (Can be larger, it will only use the fraction part to convert it to the hue angle)
			saturation	[0, 1]
			brightness	[0, 1]
		*/
		static Color HSBToRGB(float hue, float saturation, float brightness);

		/*
			Returns a random color with the given saturation and brightness.
			Arguments:
				hueStep: The hue will be a multiple of this. This can be used to get colors which are not too close to eachother.
				saturation	[0, 1]
				brightness	[0, 1]
		*/
		static Color Random(float saturation = 1.f, float brightness = 1.f, float hueStep = 0.0001f);

		static Color ChangeHue(const Color& color, float hue);
		static Color ChangeSaturation(const Color& color, float saturation);
		static Color ChangeBrightness(const Color& color, float brightness);

		float GetHue() const;
		float GetSaturation() const;
		float GetBrightness() const;

		Color();
		Color(float r, float g, float b);
		Color(const glm::vec3& color);

		float r, g, b;

		operator glm::vec3() const { return glm::vec3(r, g, b); }
		operator glm::vec4() const { return glm::vec4(r, g, b, 1.f); }
	};
}