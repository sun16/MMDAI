/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef VPVL2_GL_INTERNAL_ENGINECOMMON_H_
#define VPVL2_GL_INTERNAL_ENGINECOMMON_H_
#include <vpvl2/vpvl2.h>
#include <vpvl2/IRenderContext.h>

#if defined(VPVL2_LINK_QT)
#include <QtOpenGL/QtOpenGL>
#elif defined(WIN32)
#include <windows.h>
#endif /* VPVL_LINK_QT */

#if defined(VPVL2_ENABLE_GLES2)
#include <GLES2/gl2.h>
#elif defined(VPVL2_BUILD_IOS)
#include <OpenGLES/ES2/gl.h>
#else
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/CGLCurrent.h>
#elif defined(VPVL2_LINK_GLEW)
#include <GL/glew.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif /* __APPLE__ */
#endif /* VPVL_BUILD_IOS */

namespace vpvl2
{
namespace gl2
{

const GLsizei kShadowMappingTextureWidth = 1024;
const GLsizei kShadowMappingTextureHeight = 1024;
const GLuint kAddressNotFound = GLuint(-1);

class BaseShaderProgram
        #ifdef VPVL2_LINK_QT
        : protected QGLFunctions
        #endif
{
public:
    BaseShaderProgram(IRenderContext *renderContextRef)
        : m_program(0),
          m_renderContextRef(renderContextRef),
          m_modelViewProjectionUniformLocation(0),
          m_positionAttributeLocation(0),
          m_message(0)
    {
#ifdef VPVL2_LINK_QT
        initializeGLFunctions();
#endif
        m_program = glCreateProgram();
    }
    virtual ~BaseShaderProgram() {
        if (m_program) {
            glDeleteProgram(m_program);
            m_program = 0;
        }
        delete[] m_message;
        m_message = 0;
        m_modelViewProjectionUniformLocation = 0;
        m_positionAttributeLocation = 0;
    }

    bool addShaderSource(const IString *s, GLenum type, void *context) {
        if (!s) {
            log0(context, IRenderContext::kLogWarning, "Empty shader source found!");
            return false;
        }
        GLuint shader = glCreateShader(type);
        const char *source = reinterpret_cast<const char *>(s->toByteArray());
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                delete[] m_message;
                m_message = new char[len];
                glGetShaderInfoLog(shader, len, NULL, m_message);
                log0(context, IRenderContext::kLogWarning, "%s", m_message);
            }
            glDeleteShader(shader);
            return false;
        }
        glAttachShader(m_program, shader);
        glDeleteShader(shader);
        return true;
    }
    bool linkProgram(void *context) {
        GLint linked;
        bindAttributeLocations();
        glLinkProgram(m_program);
        glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint len = 0;
            glGetShaderiv(m_program, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                delete[] m_message;
                m_message = new char[len];
                glGetProgramInfoLog(m_program, len, NULL, m_message);
                log0(context, IRenderContext::kLogWarning, "Link failed: %s", m_message);
            }
            glDeleteProgram(m_program);
            return false;
        }
        log0(context, IRenderContext::kLogInfo, "Created a shader program (ID=%d)", m_program);
        getUniformLocations();
        return true;
    }
    virtual void bind() {
        glUseProgram(m_program);
    }
    virtual void unbind() {
        glUseProgram(0);
    }
    void setModelViewProjectionMatrix(const float value[16]) {
        glUniformMatrix4fv(m_modelViewProjectionUniformLocation, 1, GL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        glBindAttribLocation(m_program, IModel::IBuffer::kVertexStride, "inPosition");
    }
    virtual void getUniformLocations() {
        m_modelViewProjectionUniformLocation = glGetUniformLocation(m_program, "modelViewProjectionMatrix");
    }
    void log0(void *context, IRenderContext::LogLevel level, const char *format...) {
        va_list ap;
        va_start(ap, format);
        m_renderContextRef->log(context, level, format, ap);
        va_end(ap);
    }

    GLuint m_program;

private:
    IRenderContext *m_renderContextRef;
    GLuint m_modelViewProjectionUniformLocation;
    GLuint m_positionAttributeLocation;
    char *m_message;
};

class ObjectProgram : public BaseShaderProgram
{
public:
    static const char *const kNormalAttributeName;
    static const char *const kTexCoordAttributeName;

    ObjectProgram(IRenderContext *renderContextRef)
        : BaseShaderProgram(renderContextRef),
          m_normalAttributeLocation(0),
          m_texCoordAttributeLocation(0),
          m_normalMatrixUniformLocation(0),
          m_lightColorUniformLocation(0),
          m_lightDirectionUniformLocation(0),
          m_lightViewProjectionMatrixUniformLocation(0),
          m_hasMainTextureUniformLocation(0),
          m_hasDepthTextureUniformLocation(0),
          m_mainTextureUniformLocation(0),
          m_depthTextureUniformLocation(0),
          m_depthTextureSizeUniformLocation(0),
          m_enableSoftShadowUniformLocation(0),
          m_opacityUniformLocation(0)
    {
    }
    virtual ~ObjectProgram() {
        m_normalAttributeLocation = 0;
        m_texCoordAttributeLocation = 0;
        m_normalMatrixUniformLocation = 0;
        m_lightColorUniformLocation = 0;
        m_lightDirectionUniformLocation = 0;
        m_lightViewProjectionMatrixUniformLocation = 0;
        m_hasMainTextureUniformLocation = 0;
        m_hasDepthTextureUniformLocation = 0;
        m_mainTextureUniformLocation = 0;
        m_depthTextureUniformLocation = 0;
        m_depthTextureSizeUniformLocation = 0;
        m_enableSoftShadowUniformLocation = 0;
        m_opacityUniformLocation = 0;
    }

    void setLightColor(const Vector3 &value) {
        glUniform3fv(m_lightColorUniformLocation, 1, value);
    }
    void setLightDirection(const Vector3 &value) {
        glUniform3fv(m_lightDirectionUniformLocation, 1, value);
    }
    void setLightViewProjectionMatrix(const GLfloat value[16]) {
        glUniformMatrix4fv(m_lightViewProjectionMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setNormalMatrix(const float value[16]) {
        float m[] = {
            value[0], value[1], value[2],
            value[4], value[5], value[6],
            value[8], value[9], value[10]
        };
        glUniformMatrix3fv(m_normalMatrixUniformLocation, 1, GL_FALSE, m);
    }
    void setMainTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_mainTextureUniformLocation, 0);
            glUniform1i(m_hasMainTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasMainTextureUniformLocation, 0);
        }
    }
    void setDepthTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_depthTextureUniformLocation, 3);
            glUniform1i(m_hasDepthTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasDepthTextureUniformLocation, 0);
        }
    }
    void setDepthTextureSize(const Vector3 &value) {
        glUniform2fv(m_depthTextureSizeUniformLocation, 1, value);
    }
    void setSoftShadowEnable(bool value) {
        glUniform1f(m_enableSoftShadowUniformLocation, GLfloat(value ? 1 : 0));
    }
    void setOpacity(const Scalar &value) {
        glUniform1f(m_opacityUniformLocation, value);
    }

protected:
    virtual void bindAttributeLocations() {
        BaseShaderProgram::bindAttributeLocations();
        glBindAttribLocation(m_program, IModel::IBuffer::kNormalStride, "inNormal");
        glBindAttribLocation(m_program, IModel::IBuffer::kTextureCoordStride, "inTexCoord");
    }
    virtual void getUniformLocations() {
        BaseShaderProgram::getUniformLocations();
        m_normalMatrixUniformLocation = glGetUniformLocation(m_program, "normalMatrix");
        m_lightColorUniformLocation = glGetUniformLocation(m_program, "lightColor");
        m_lightDirectionUniformLocation = glGetUniformLocation(m_program, "lightDirection");
        m_lightViewProjectionMatrixUniformLocation = glGetUniformLocation(m_program, "lightViewProjectionMatrix");
        m_hasMainTextureUniformLocation = glGetUniformLocation(m_program, "hasMainTexture");
        m_hasDepthTextureUniformLocation = glGetUniformLocation(m_program, "hasDepthTexture");
        m_mainTextureUniformLocation = glGetUniformLocation(m_program, "mainTexture");
        m_depthTextureUniformLocation = glGetUniformLocation(m_program, "depthTexture");
        m_depthTextureSizeUniformLocation = glGetUniformLocation(m_program, "depthTextureSize");
        m_enableSoftShadowUniformLocation = glGetUniformLocation(m_program, "useSoftShadow");
        m_opacityUniformLocation = glGetUniformLocation(m_program, "opacity");
    }

private:
    GLuint m_normalAttributeLocation;
    GLuint m_texCoordAttributeLocation;
    GLuint m_normalMatrixUniformLocation;
    GLuint m_lightColorUniformLocation;
    GLuint m_lightDirectionUniformLocation;
    GLuint m_lightViewProjectionMatrixUniformLocation;
    GLuint m_hasMainTextureUniformLocation;
    GLuint m_hasDepthTextureUniformLocation;
    GLuint m_mainTextureUniformLocation;
    GLuint m_depthTextureUniformLocation;
    GLuint m_depthTextureSizeUniformLocation;
    GLuint m_enableSoftShadowUniformLocation;
    GLuint m_opacityUniformLocation;
};

class ZPlotProgram : public BaseShaderProgram
{
public:
    ZPlotProgram(IRenderContext *renderContextRef)
        : BaseShaderProgram(renderContextRef),
          m_transformUniformLocation(0)
    {
    }
    virtual ~ZPlotProgram() {
        m_transformUniformLocation = 0;
    }

    void setTransformMatrix(const float value[16]) {
        glUniformMatrix4fv(m_transformUniformLocation, 1, GL_FALSE, value);
    }

protected:
    virtual void getUniformLocations() {
        BaseShaderProgram::getUniformLocations();
        m_transformUniformLocation = glGetUniformLocation(m_program, "transformMatrix");
    }

private:
    GLuint m_transformUniformLocation;
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
