#pragma once

#include "Rendering/GraphicsContext.h"

namespace Firefly
{
	class Texture
	{
	public:
		enum class Type
		{
			TEXTURE_2D,
			TEXTURE_CUBE_MAP
		};

		enum class Format
		{
			NONE,

			R_8,
			R_8_NON_LINEAR,
			R_16_FLOAT,
			R_32_FLOAT,

			RG_8,
			RG_8_NON_LINEAR,
			RG_16_FLOAT,
			RG_32_FLOAT,

			RGB_8,
			RGB_8_NON_LINEAR,
			RGB_16_FLOAT,
			RGB_32_FLOAT,

			RGBA_8,
			RGBA_8_NON_LINEAR,
			RGBA_16_FLOAT,
			RGBA_32_FLOAT,

			DEPTH_32_FLOAT,
			DEPTH_24_STENCIL_8
		};

		enum class SampleCount
		{
			SAMPLE_1,
			SAMPLE_2,
			SAMPLE_4,
			SAMPLE_8,
			SAMPLE_16,
			SAMPLE_32,
			SAMPLE_64
		};

		enum class WrapMode
		{
			REPEAT,
			MIRRORED_REPEAT,
			CLAMP_TO_EDGE,
			MIRROR_CLAMP_TO_EDGE,
			CLAMP_TO_BORDER
		};

		enum class FilterMode
		{
			NEAREST,
			LINEAR
		};

		struct SamplerDescription
		{
			bool isMipMappingEnabled = false;
			bool isAnisotropicFilteringEnabled = false;
			uint32_t maxAnisotropy = 8;
			WrapMode wrapMode = WrapMode::REPEAT;
			FilterMode magnificationFilterMode = FilterMode::LINEAR;
			FilterMode minificationFilterMode = FilterMode::LINEAR;
			FilterMode mipMapFilterMode = FilterMode::LINEAR;
		};

		struct Description
		{
			Type type = Type::TEXTURE_2D;
			uint32_t width = 0;
			uint32_t height = 0;
			Format format = Format::RGBA_8;
			SampleCount sampleCount = SampleCount::SAMPLE_1;
			bool useAsAttachment = false;
			bool useSampler = false;
			SamplerDescription sampler = {};
		};

		void Init(const std::string& path, bool useLinearColorSpace = true);
		void Init(const Description& description);
		virtual void Destroy() = 0;

		uint32_t GetWidth();
		uint32_t GetHeight();
		Type GetType() const;
		SampleCount GetSampleCount() const;
		uint32_t GetMipMapLevels() const;
		Format GetFormat() const;
		std::string GetFormatAsString() const;
		static std::string ConvertFormatToString(Format format);
		bool HasColorFormat() const;
		bool HasDepthFormat() const;
		bool HasDepthStencilFormat() const;

	protected:
		virtual void OnInit(void* pixelData) = 0;
		void CalcMipMapLevels();
		void CalcArrayLayers();
		static uint32_t ConvertToSampleCountNumber(SampleCount sampleCount);
		static uint32_t GetBytePerPixel(Format format);

		Description m_description;
		uint32_t m_mipMapLevels;
		uint32_t m_arrayLayers;
	};
}