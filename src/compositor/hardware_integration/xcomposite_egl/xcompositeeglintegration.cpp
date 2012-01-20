/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include "xcompositeeglintegration.h"

#include "waylandobject.h"
#include "wayland_wrapper/wlcompositor.h"
#include "wayland-xcomposite-server-protocol.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QPlatformNativeInterface>
#include <QtGui/QPlatformOpenGLContext>

#include "xcompositebuffer.h"
#include "xcompositehandler.h"
#include <X11/extensions/Xcomposite.h>

#include <QtCore/QDebug>

QVector<EGLint> eglbuildSpec()
{
    QVector<EGLint> spec;

    spec.append(EGL_SURFACE_TYPE); spec.append(EGL_WINDOW_BIT | EGL_PIXMAP_BIT);
    spec.append(EGL_RENDERABLE_TYPE); spec.append(EGL_OPENGL_ES2_BIT);
    spec.append(EGL_BIND_TO_TEXTURE_RGBA); spec.append(EGL_TRUE);
    spec.append(EGL_ALPHA_SIZE); spec.append(8);
    spec.append(EGL_NONE);
    return spec;
}


struct wl_xcomposite_interface XCompositeHandler::xcomposite_interface = {
    XCompositeHandler::create_buffer
};

GraphicsHardwareIntegration *GraphicsHardwareIntegration::createGraphicsHardwareIntegration(WaylandCompositor *compositor)
{
    return new XCompositeEglIntegration(compositor);
}

XCompositeEglIntegration::XCompositeEglIntegration(WaylandCompositor *compositor)
    : GraphicsHardwareIntegration(compositor)
    , mDisplay(0)
{
    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (nativeInterface) {
        mDisplay = static_cast<Display *>(nativeInterface->nativeResourceForWindow("Display",m_compositor->window()));
        if (!mDisplay)
            qFatal("could not retireve Display from platform integration");
        mEglDisplay = static_cast<EGLDisplay>(nativeInterface->nativeResourceForWindow("EGLDisplay",m_compositor->window()));
        if (!mEglDisplay)
            qFatal("could not retrieve EGLDisplay from plaform integration");
    } else {
        qFatal("Platform integration doesn't have native interface");
    }
    mScreen = XDefaultScreen(mDisplay);
}

void XCompositeEglIntegration::initializeHardware(Wayland::Display *waylandDisplay)
{
    XCompositeHandler *handler = new XCompositeHandler(m_compositor->handle(),mDisplay,m_compositor->window());
    wl_display_add_global(waylandDisplay->handle(),&wl_xcomposite_interface,handler,XCompositeHandler::xcomposite_bind_func);
}

GLuint XCompositeEglIntegration::createTextureFromBuffer(wl_buffer *buffer, QOpenGLContext *)
{
    XCompositeBuffer *compositorBuffer = Wayland::wayland_cast<XCompositeBuffer *>(buffer);
    Pixmap pixmap = XCompositeNameWindowPixmap(mDisplay, compositorBuffer->window());

    QVector<EGLint> eglConfigSpec = eglbuildSpec();

    EGLint matching = 0;
    EGLConfig config;
    bool matched = eglChooseConfig(mEglDisplay,eglConfigSpec.constData(),&config,1,&matching);
    if (!matched || !matching) {
        qWarning("Could not retrieve a suitable EGL config");
        return 0;
    }

    QVector<EGLint> attribList;

    attribList.append(EGL_TEXTURE_FORMAT);
    attribList.append(EGL_TEXTURE_RGBA);
    attribList.append(EGL_TEXTURE_TARGET);
    attribList.append(EGL_TEXTURE_2D);
    attribList.append(EGL_NONE);

    EGLSurface surface = eglCreatePixmapSurface(mEglDisplay,config,pixmap,attribList.constData());
    if (surface == EGL_NO_SURFACE) {
        qDebug() << "Failed to create eglsurface" << pixmap << compositorBuffer->window();
    }

    compositorBuffer->setInvertedY(false);

    GLuint textureId;
    glGenTextures(1,&textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    if (!eglBindTexImage(mEglDisplay,surface,EGL_BACK_BUFFER)) {
        qDebug() << "Failed to bind";
    }

    //    eglDestroySurface(mEglDisplay,surface);

    return textureId;
}

bool XCompositeEglIntegration::isYInverted(wl_buffer *buffer) const
{
    XCompositeBuffer *compositorBuffer = Wayland::wayland_cast<XCompositeBuffer *>(buffer);
    return compositorBuffer->isYInverted();
}
