#pragma once

#include "Walnut/Image.h"
#include <memory>
#include <glm/glm.hpp>

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Camera& cam, const Scene& scene);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; };
private:
	struct HitPayLoad 
	{
		float HitDist;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		uint32_t ObjectIndex;
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y);

	HitPayLoad TraceRay(const Ray& ray);
	HitPayLoad ClosestHit(const Ray& ray, int objectIndex, float hitDist);
	HitPayLoad Miss(const Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	uint32_t* m_ImageData = nullptr;
};
