#include "Scene/Scene.h"

namespace Firefly
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
		m_entityRegistry.clear();
	}
}