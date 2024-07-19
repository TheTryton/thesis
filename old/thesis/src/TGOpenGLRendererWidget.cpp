#include <TGOpenGLRendererWidget.hpp>

#include <QOpenGLFunctions_4_5_Core>

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>

#include <expected>

constexpr float vertexData[] = {
    // first triangle / second triangle interleaved
    -0.9f, -0.5f, 0.0f,// left
    0.0f, -0.5f, 0.0f,  // left
    -0.0f, -0.5f, 0.0f,  // right
    0.9f, -0.5f, 0.0f,  // right
    -0.45f, 0.5f, 0.0f,  // top
    0.45f, 0.5f, 0.0f   // top
};

GLuint unwrap(const std::expected<GLuint, QString>& v)
{
    if(!v)
    {
        qCritical() << v.error();
        return 0;
    }

    return v.value();
}

class TGOpenGLRendererWidgetImpl
    : public QOpenGLFunctions_4_5_Core
{
private:
    GLuint shaderProgram;
    GLuint vao[2];
    GLuint vbo;

private:
    static std::string readAllText(QString fileName)
    {
        QFile file(fileName);
        assert(file.open(QFile::ReadOnly | QFile::Text));
        QTextStream in(&file);
        return in.readAll().toStdString();
    }

    std::expected<GLuint, QString> createAndCompileShader(std::string source, GLenum type)
    {
        auto shader = glCreateShader(type);

        auto sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLint length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            QByteArray infoLog{length + 1, Qt::Uninitialized};
            GLsizei realLength;
            glGetShaderInfoLog(shader, static_cast<GLsizei>(length),&realLength, infoLog.data());
            infoLog.resize(realLength + 1);
            infoLog[realLength] = '\0';
            glDeleteShader(shader);
            qDebug() << QString::fromLatin1(infoLog);
            return std::unexpected("Error: shader compilation failed!\n" + QString::fromLatin1(infoLog));
        }

        return shader;
    }

    std::expected<GLuint, QString> linkProgram(GLuint vertex, GLuint fragment)
    {
        auto program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);

        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLint length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            QByteArray infoLog{length + 1, Qt::Uninitialized};
            GLsizei realLength;
            glGetProgramInfoLog(program, static_cast<GLsizei>(length),&realLength, infoLog.data());
            infoLog.resize(realLength + 1);
            infoLog[realLength] = '\0';
            glDeleteProgram(program);
            qDebug() << QString::fromLatin1(infoLog);
            return std::unexpected("Error: program linking failed!\n" + QString::fromLatin1(infoLog));
        }

        return program;
    }

    void initializeShaders()
    {
        auto vertexShaderSource = readAllText("shaders/vertex.glsl");
        auto fragmentShaderSource = readAllText("shaders/fragment.glsl");

        GLuint vertexShader = unwrap(createAndCompileShader(vertexShaderSource, GL_VERTEX_SHADER));
        GLuint fragmentShader =  unwrap(createAndCompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER));

        shaderProgram = unwrap(linkProgram(vertexShader, fragmentShader));

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void initializeBuffers()
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

        glGenVertexArrays(2, vao);

        glBindVertexArray(vao[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        glBindVertexArray(vao[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }
public:
    void initializeGL()
    {
        qDebug() << "OpenGL version: " << (char*)glGetString(GL_VERSION);
        initializeShaders();
        initializeBuffers();
    }
    void paintGL()
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(vao[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(vao[1]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    void resizeGL(int width, int height)
    {

    }
};

TGOpenGLRendererWidget::TGOpenGLRendererWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , _impl(std::make_unique<TGOpenGLRendererWidgetImpl>())
{
}

void TGOpenGLRendererWidget::initializeGL()
{
    assert(_impl->initializeOpenGLFunctions());
    _impl->initializeGL();
}
void TGOpenGLRendererWidget::paintGL()
{
    _impl->paintGL();
}
void TGOpenGLRendererWidget::resizeGL(int width, int height)
{
    _impl->resizeGL(width, height);
}