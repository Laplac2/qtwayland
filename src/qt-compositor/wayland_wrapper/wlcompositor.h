/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#ifndef WL_COMPOSITOR_H
#define WL_COMPOSITOR_H

#include <QtCore/QSet>

#include "wloutput.h"
#include "wldisplay.h"
#include "wlshmbuffer.h"

#include <wayland-server.h>

class WaylandCompositor;
class GraphicsHardwareIntegration;
class QWidget;
class WindowManagerServerIntegration;

namespace Wayland {

class Surface;

class Compositor : public QObject, public Object<struct wl_compositor>
{
    Q_OBJECT

public:
    Compositor(WaylandCompositor *qt_compositor);
    ~Compositor();

    void frameFinished(Surface *surface = 0);
    void setInputFocus(Surface *surface);
    void setKeyFocus(Surface *surface);
    Surface *keyFocus() const;
    void setPointerFocus(Surface *surface, const QPoint &point = QPoint());
    Surface *pointerFocus() const;

    Surface *getSurfaceFromWinId(uint winId) const;
    struct wl_client *getClientFromWinId(uint winId) const;
    QImage image(uint winId) const;

    const struct wl_input_device *inputDevice() const { return &m_input; }
    struct wl_input_device *inputDevice() { return &m_input; }

    void createSurface(struct wl_client *client, int id);
    void surfaceDestroyed(Surface *surface);
    void markSurfaceAsDirty(Surface *surface);

    void destroyClientForSurface(Surface *surface);

    static uint currentTimeMsecs();

    QWidget *topLevelWidget() const;

    GraphicsHardwareIntegration *graphicsHWIntegration() const;
    void initializeHardwareIntegration();
    void initializeWindowManagerProtocol();
    bool setDirectRenderSurface(Surface *surface);
    Surface *directRenderSurface() const {return m_directRenderSurface;}

    QList<Surface*> surfacesForClient(wl_client* client);

    wl_input_device *defaultInputDevice();
    WaylandCompositor *qtCompositor() const { return m_qt_compositor; }

    struct wl_display *wl_display() { return m_display->handle(); }

    static Compositor *instance();

    QList<struct wl_client *> clients() const;

    void setScreenOrientation(qint32 orientationInDegrees);

signals:
    void clientAdded(wl_client *client);

private slots:
    void processWaylandEvents();

private:
    Display *m_display;

    /* Input */
    struct wl_input_device m_input;

    /* Output */
    Output m_output;

    struct wl_object m_shell;

    /* shm/*/
    ShmHandler m_shm;
    QList<Surface *> m_surfaces;
    QSet<Surface *> m_dirty_surfaces;

    /* Render state */
    uint32_t m_current_frame;
    int m_last_queued_buf;

    wl_event_loop *m_loop;

    WaylandCompositor *m_qt_compositor;

    Surface *m_pointerFocusSurface;
    Surface *m_keyFocusSurface;
    Surface *m_directRenderSurface;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    GraphicsHardwareIntegration *m_graphics_hw_integration;
#endif
    WindowManagerServerIntegration *m_windowManagerWaylandProtocol;
};

}

#endif //WL_COMPOSITOR_H
