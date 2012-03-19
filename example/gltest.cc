#include <v8.h>
#include <v8_webgl.h>
#include <iostream>
#include <QApplication>
#include <QGLWidget>

class GraphicContext : public v8_webgl::GraphicContext {
 public:
  //XXX configure antialiasing etc.
  GraphicContext(int width, int height)
      : gl_widget_(new QGLWidget()) {
  }
  ~GraphicContext() {
    delete gl_widget_;
  }
  void Resize(int width, int height) {
    gl_widget_->resize(width, height);
  }
  void MakeCurrent() {
    //XXX optimize this
    gl_widget_->makeCurrent();
  }
 private:
  QGLWidget* gl_widget_;
};

class Factory : public v8_webgl::Factory, v8_webgl::Logger {
 public:
  void Log(Level level, std::string& msg) {
    std::cerr << msg << std::endl;
  }
  v8_webgl::GraphicContext* CreateGraphicContext(int width, int height) {
    return new GraphicContext(width, height);
  }
  Logger* GetLogger() { return this; }
};

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  {
    v8::HandleScope handle_scope;

    const char* kExposeGC = "--expose-gc";
    v8::V8::SetFlagsFromString(kExposeGC, strlen(kExposeGC));

    v8::Handle<v8::ObjectTemplate> global = v8_webgl::Initialize(new Factory());
    v8::Persistent<v8::Context> context = v8::Context::New(NULL, global);

    v8::Context::Scope context_scope(context);

    v8::Handle<v8::String> source =
        v8::String::New
        ("var a = new ArrayBuffer(20);"
         "console.log('ArrayBuffer=' + a + ' el=' + a[1]);"
         "var ia = new Int8Array(a); ia[1] = 7;"
         "console.log('a[1]=' + a[1]);"
         "console.log('ia[1]=' + ia[1]);"
         "var i32a = new Int32Array(10);"
         "i32a[4] = 50;"
         "var ui32a = new Uint32Array(i32a.buffer, 16, 1);"
         "console.log('ui32a[0]=' + ui32a[0]);"
         "var canvas = new Canvas();"
         "canvas.width = canvas.height = 200;"
         "var gl = canvas.getContext();"
         "console.log('canvas=' + canvas);"
         "console.log('context=' + gl);"
         "console.log('width=' + canvas.width);"
         "console.log('FRONT=' + WebGLRenderingContext.FRONT);"
         "var buffer = gl.createBuffer();"
         "console.log('buffer=' + buffer);"
         //"new Canvas().getContext().deleteBuffer(buffer);"
         "gl.deleteBuffer(buffer);"
         "var program = gl.createProgram();"
         "console.log('program=' + program);"
         "gl.deleteProgram(program);"
         "var framebuffer = gl.createFramebuffer();"
         "console.log('framebuffer=' + framebuffer);"
         "gl.deleteFramebuffer(framebuffer);"
         "var renderbuffer = gl.createRenderbuffer();"
         "console.log('renderbuffer=' + renderbuffer);"
         "gl.deleteRenderbuffer(renderbuffer);"
         "var shader = gl.createShader(gl.FRAGMENT_SHADER);"
         "console.log('shader=' + shader);"
         "gl.deleteShader(shader);"
         "var texture = gl.createTexture();"
         "console.log('texture=' + texture);"
         "gl.deleteTexture(texture);"
         "console.log('error=' + gl.getError());"
         "canvas = null;"
         "/*gc();*/"
         );
    v8::Handle<v8::Script> script = v8::Script::Compile(source);

    v8::Handle<v8::Value> result = script->Run();

    context.Dispose();

    // Convert the result to an ASCII string and print it.
    v8::String::AsciiValue ascii(result);
    printf("%s\n", *ascii);
  }

  v8_webgl::Uninitialize();
  return 0;
}
