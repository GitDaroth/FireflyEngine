#pragma once

#include <glm/glm.hpp>

namespace Firefly
{
	enum class ShaderDataType
	{
		Float, Float2, Float3, Float4, Mat3, Mat4
	};

	class VertexBuffer
	{
	public:
		class Element
		{
		public:
			Element(ShaderDataType type, const std::string& name = "", bool isNormalized = false) :
				m_type(type), m_name(name), m_isNormalized(isNormalized), m_offset(0) 
			{
				switch (type)
				{
				case ShaderDataType::Float:
					m_count = 1;
					m_size = sizeof(glm::vec1);
					break;
				case ShaderDataType::Float2:
					m_count = 2;
					m_size = sizeof(glm::vec2);
					break;
				case ShaderDataType::Float3:
					m_count = 3;
					m_size = sizeof(glm::vec3);
					break;
				case ShaderDataType::Float4:
					m_count = 4;
					m_size = sizeof(glm::vec4);
					break;
				case ShaderDataType::Mat3:
					m_count = 3 * 3;
					m_size = sizeof(glm::mat3x3);
					break;
				case ShaderDataType::Mat4:
					m_count = 4 * 4;
					m_size = sizeof(glm::mat4x4);
					break;
				}
			}

			inline ShaderDataType GetType() const { return m_type; }
			inline const std::string& GetName() const { return m_name; }
			inline bool IsNormalized() const { return m_isNormalized; }
			inline uint32_t GetSize() const { return m_size; }
			inline uint32_t GetCount() const { return m_count; }
			inline uint32_t GetOffset() const { return m_offset; }
			inline void SetOffset(uint32_t offset) { m_offset = offset; }

		private:
			ShaderDataType m_type;
			std::string m_name;
			bool m_isNormalized;
			uint32_t m_size;
			uint32_t m_count;
			uint32_t m_offset;
		};

		class Layout
		{
		public:
			Layout(const std::initializer_list<Element>& elements) :
				m_elements(elements), m_stride(0)
			{
				for (auto& element : m_elements)
				{
					element.SetOffset(m_stride);
					m_stride += element.GetSize();
				}
			}

			inline const std::vector<Element>& GetElements() const { return m_elements; }
			inline uint32_t GetStride() const { return m_stride; }

		private:
			uint32_t m_stride;
			std::vector<Element> m_elements;
		};

		VertexBuffer() : m_layout({}) {}
		virtual ~VertexBuffer() {}

		virtual void Init(float* vertices, uint32_t size) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		inline const Layout& GetLayout() const { return m_layout; }
		inline void SetLayout(const Layout& layout) { m_layout = layout; }

	private:
		Layout m_layout;
	};
}