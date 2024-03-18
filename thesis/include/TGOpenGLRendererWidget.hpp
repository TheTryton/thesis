#pragma once

#include <QOpenGLWidget>
#include <memory>

class TGOpenGLRendererWidgetImpl;

class TGOpenGLRendererWidget
    : public QOpenGLWidget
{
private:
    std::unique_ptr<TGOpenGLRendererWidgetImpl> _impl;
public:
    TGOpenGLRendererWidget(QWidget* parent);
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
};