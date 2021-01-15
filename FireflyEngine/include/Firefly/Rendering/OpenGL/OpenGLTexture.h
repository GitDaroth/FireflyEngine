#pragma once

#include "Rendering/Texture.h"

#include <glad/glad.h>

namespace Firefly
{
	class OpenGLTexture : public Texture
	{
	public:
		OpenGLTexture(std::shared_ptr<GraphicsContext> context);

		virtual void Destroy() override;

		void Bind(GLuint slot);

	protected:
		virtual void OnInit(unsigned char* pixelData, ColorSpace colorSpace) override;

	private:
		uint32_t m_texture;
	};
}