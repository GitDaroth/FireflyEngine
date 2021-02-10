#include "pch.h"
#include "Rendering/OpenGL/OpenGLRenderPass.h"

#include "Rendering/OpenGL/OpenGLFrameBuffer.h"
#include <glad/glad.h>

namespace Firefly
{
	void OpenGLRenderPass::OnInit()
	{

	}

	void OpenGLRenderPass::Destroy()
	{

	}

	void OpenGLRenderPass::OnBegin()
	{
		std::shared_ptr<OpenGLFrameBuffer> openGLFrameBuffer = std::dynamic_pointer_cast<OpenGLFrameBuffer>(m_currentFrameBuffer);
		openGLFrameBuffer->Bind();

		glViewport(0, 0, m_currentFrameBuffer->GetWidth(), m_currentFrameBuffer->GetHeight());

		if (m_description.isDepthTestingEnabled)
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(ConvertToOpenGLDepthFunction(m_description.depthCompareOperation));
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
		}

		if (m_description.isMultisamplingEnabled)
		{
			glEnable(GL_MULTISAMPLE);
			if (m_description.isSampleShadingEnabled)
			{
				glEnable(GL_SAMPLE_SHADING);
				glMinSampleShading(m_description.minSampleShading);
			}
			else
			{
				glDisable(GL_SAMPLE_SHADING);
			}
		}
		else
		{
			glDisable(GL_MULTISAMPLE);
			glDisable(GL_SAMPLE_SHADING);
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		GLbitfield clearBitMask = GL_COLOR_BUFFER_BIT;
		if (m_currentFrameBuffer->HasDepthStencilAttachment() && m_description.isDepthTestingEnabled)
		{
			glClearDepth(1.0f);
			clearBitMask |= GL_DEPTH_BUFFER_BIT;
		}
		glClear(clearBitMask);
	}

	void OpenGLRenderPass::OnEnd()
	{
	}

	GLenum OpenGLRenderPass::ConvertToOpenGLDepthFunction(CompareOperation compareOperation)
	{
		GLenum depthFunction = GL_NONE;
#
		switch (compareOperation)
		{
		case CompareOperation::LESS:
			depthFunction = GL_LESS;
			break;
		case CompareOperation::LESS_OR_EQUAL:
			depthFunction = GL_LEQUAL;
			break;
		case CompareOperation::EQUAL:
			depthFunction = GL_EQUAL;
			break;
		case CompareOperation::NOT_EQUAL:
			depthFunction = GL_NOTEQUAL;
			break;
		case CompareOperation::GREATER:
			depthFunction = GL_GREATER;
			break;
		case CompareOperation::GREATER_OR_EQUAL:
			depthFunction = GL_GEQUAL;
			break;
		case CompareOperation::ALWAYS:
			depthFunction = GL_ALWAYS;
			break;
		case CompareOperation::NEVER:
			depthFunction = GL_NEVER;
			break;
		}

		return depthFunction;
	}
}