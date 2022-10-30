#include "Renderer.h" 
#include "Walnut/Random.h"

namespace Util {
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render(const Camera& cam, const Scene& scene)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &cam;

	float aspectRatio = m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Util::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

Renderer::HitPayLoad Renderer::TraceRay(const Ray& ray)
{
	int closestSphereIndex = -1;
	float hitDist = std::numeric_limits<float>::max();

	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			continue;

		float t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		if (t1 > 0.0f && t1 < hitDist)
		{
			hitDist = t1;
			closestSphereIndex = i;
		}
	}

	if (closestSphereIndex < 0)
		return Miss(ray);

	return ClosestHit(ray, closestSphereIndex, hitDist);
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 colour(0.0f);

	float multiplier = 1.0f;

	int bounces = 2;
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayLoad payload = TraceRay(ray);

		if (payload.HitDist < 0.0f)
		{
			glm::vec3 skyColour(0.0f, 0.0f, 0.0f);
			colour += skyColour * multiplier;
			break;
		}

		glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
		float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f);

		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];

		glm::vec3 sphereColor = sphere.Albedo;
		sphereColor *= lightIntensity;
		colour += sphereColor * multiplier;

		multiplier *= 0.7f;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal);
	}

	return glm::vec4(colour, 1.0f);
}

Renderer::HitPayLoad Renderer::ClosestHit(const Ray& ray, int objectIndex, float hitDist)
{
	Renderer::HitPayLoad payload;
	payload.HitDist = hitDist;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - closestSphere.Position;
	payload.WorldPosition = origin + ray.Direction * hitDist;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	payload.WorldPosition += closestSphere.Position;

	return payload;
}

Renderer::HitPayLoad Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayLoad payload;
	payload.HitDist = -1;
		
	return payload;
}