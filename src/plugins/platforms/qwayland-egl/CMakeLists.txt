# Generated from qwayland-egl.pro.

#####################################################################
## QWaylandEglPlatformIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandEglPlatformIntegrationPlugin
    OUTPUT_NAME qwayland-egl
    TYPE platforms
    SOURCES
        ../../../hardwareintegration/client/wayland-egl/qwaylandeglclientbufferintegration.cpp ../../../hardwareintegration/client/wayland-egl/qwaylandeglclientbufferintegration.h
        ../../../hardwareintegration/client/wayland-egl/qwaylandeglinclude.h
        ../../../hardwareintegration/client/wayland-egl/qwaylandeglwindow.cpp ../../../hardwareintegration/client/wayland-egl/qwaylandeglwindow.h
        ../../../hardwareintegration/client/wayland-egl/qwaylandglcontext.cpp ../../../hardwareintegration/client/wayland-egl/qwaylandglcontext.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../hardwareintegration/client/wayland-egl
    PUBLIC_LIBRARIES
        EGL::EGL
        Qt::Core
        Qt::EglSupportPrivate
        Qt::Gui
        Qt::OpenGLPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
        Wayland::Egl
)

#### Keys ignored in scope 1:.:.:qwayland-egl.pro:<TRUE>:
# OTHER_FILES = "qwayland-egl.json"
