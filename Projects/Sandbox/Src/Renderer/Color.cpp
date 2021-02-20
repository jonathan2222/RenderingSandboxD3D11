#include "PreCompiled.h"
#include "Color.h"

using namespace RS;

Color Color::HSBToRGB(float hue, float saturation, float brightness)
{
	hue = glm::fract(hue) * 360.f;
	saturation = glm::clamp(saturation, 0.f, 1.f);
	brightness = glm::clamp(brightness, 0.f, 1.f);

	auto f = [&](float n)
	{
		float k = std::fmodf((n + hue / 60.f), 6.f);
		return brightness - brightness * saturation * glm::max(0.f, glm::min(glm::min(k, 4.f - k), 1.f));
	};

	return Color(f(5.f), f(3.f), f(1.f));
}

Color Color::Random(float saturation, float brightness, float hueStep)
{
	float hue = (float)std::rand() * hueStep;
	return HSBToRGB(hue, saturation, brightness);
}

Color Color::ChangeHue(const Color& color, float hue)
{
	return HSBToRGB(hue, color.GetSaturation(), color.GetBrightness());
}

Color Color::ChangeSaturation(const Color& color, float saturation)
{
	return HSBToRGB(color.GetHue(), saturation, color.GetBrightness());
}

Color Color::ChangeBrightness(const Color& color, float brightness)
{
	return HSBToRGB(color.GetHue(), color.GetSaturation(), brightness);
}

float Color::GetHue() const
{
	float cMax = glm::max(this->r, glm::max(this->g, this->b));
	float cMin = glm::min(this->r, glm::min(this->g, this->b));
	float invDelta = 1.f / (cMax - cMin);

	float hue = 0.f;
	if (cMax < FLT_EPSILON)
		hue = 0.f;
	else if (glm::abs(cMax - this->r) < FLT_EPSILON)
		hue = 60.f * std::fmodf((this->g - this->b) / invDelta, 6.f);
	else if (glm::abs(cMax - this->g) < FLT_EPSILON)
		hue = 60.f * ((this->b - this->r) / invDelta + 2.f);
	else if (glm::abs(cMax - this->b) < FLT_EPSILON)
		hue = 60.f * ((this->r - this->g) / invDelta + 4.f);

	return hue / 360.f;
}

float Color::GetSaturation() const
{
	float cMax = glm::max(this->r, glm::max(this->g, this->b));
	float cMin = glm::min(this->r, glm::min(this->g, this->b));
	float delta = cMax - cMin;

	// Calculate the saturation of the color.
	float saturation = 0.f;
	if (cMax > FLT_EPSILON)
		saturation = delta / cMax;

	return saturation;
}

float Color::GetBrightness() const
{
	float cMax = glm::max(this->r, glm::max(this->g, this->b));
	return cMax;
}

Color::Color() : r(0.f), g(0.f), b(0.f)
{
}

Color::Color(float r, float g, float b) : r(r), g(g), b(b)
{
}

Color::Color(const glm::vec3& color) : r(color.x), g(color.y), b(color.z)
{
}
