#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QImage>
#include <QOpenGLWindow>

class Window : public QOpenGLWindow
{
public:
    void paintGL() override {
        QOpenGLFunctions *f = context()->functions();
        f->glClearColor(0, 0, 1, 1);
        f->glClear(GL_COLOR_BUFFER_BIT);
    }
};

int main(int argc, char **argv)
{
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=true");
    qputenv("QT_QPA_PLATFORM", "eglfs");
    qputenv("QT_QPA_EGLFS_INTEGRATION", "eglfs_kms");
    qputenv("QT_QPA_EGLFS_KMS_CONFIG", "config.json");

    QGuiApplication app(argc, argv);

    QOpenGLContext ctx;
    if (!ctx.create())
        qFatal("Failed to create context");

    // Approach #1: Use a "window" normally (render into the underlying gbm_surface)
    {
        Window w;
        w.show();
        // the expose triggers paintGL, grab that frame
        w.grabFramebuffer().save("output_blue.png");

        // could do more frames, but note that there is no vsync-based throttling
    }

    // Approach #2: The usual offscreen approach: QOffscreenSurface + FBO
    {
        QOffscreenSurface s; // 1x1 gbm_surface or nothing (surfaceless contexts), we don't care here (going to use an FBO anyway)
        s.setFormat(ctx.format());
        s.create();

        if (!ctx.makeCurrent(&s))
            qFatal("Failed to make surface current");

        QOpenGLFramebufferObject fbo(1024, 768);
        fbo.bind();

        ctx.functions()->glClearColor(1, 0, 0, 1);
        ctx.functions()->glClear(GL_COLOR_BUFFER_BIT);
        fbo.toImage().save("output_red.png");

        ctx.doneCurrent();
    }

    return 0;
}
