#pragma once

#include "Rendering/Texture.h"

namespace Firefly
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D();
		virtual ~OpenGLTexture2D();

		virtual void Init(const std::string& path) override;
		virtual void Bind(uint32_t slot) override;

	private:
		uint32_t m_texture;
	};
}