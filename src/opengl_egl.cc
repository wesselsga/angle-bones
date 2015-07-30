#include "opengl.h"

#include <iostream>

#include <EGL/egl.h>	
#include <GLES2/gl2.h>

class EglContext : public opengl::Context
{
public:
  	explicit EglContext(
         EGLContext ctx, 
         EGLDisplay display,
         EGLSurface surface)
         : _ctx(ctx)
         , _display(display)
         , _surface(surface)
   {
   }

   ~EglContext(){}

	void bind()
   {
      eglMakeCurrent(_display, _surface, _surface, _ctx);
   }

	void unbind()
   {
   }

   void clear()
   {
      glClear(GL_COLOR_BUFFER_BIT);
   }
	
   void swapBuffers()
   {
      eglSwapBuffers(_display, _surface);
   }

private:
   EglContext()=delete;
   EglContext(EglContext const&)=delete;
   EglContext& operator=(EglContext const&)=delete;

	EGLContext const _ctx;
	EGLDisplay const _display;
	EGLSurface const _surface;
};


std::shared_ptr<opengl::Context> opengl::create_context(void* native_window)
{
   EGLContext context;
   EGLSurface surface;
   EGLConfig config;
   
   auto const display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (display == EGL_NO_DISPLAY) {
      return nullptr;
   }

   // initialize EGL
   EGLint major, minor;
   if (!eglInitialize(display, &major, &minor)) 
   {
      std::cerr << "egl: failed to initialize " 
         << major << "." << minor << std::endl;
      return nullptr;
   }

	std::cout << "egl: initialized version " 
      << major << "." << minor << std::endl;

   // get configs
   EGLint numConfigs;
   if (!eglGetConfigs(display, NULL, 0, &numConfigs)) {
      return nullptr;
   }

	EGLint attribs[] =
   {
       EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
       EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
       EGL_BUFFER_SIZE,     32,
       EGL_RED_SIZE,        8,
       EGL_GREEN_SIZE,      8,
       EGL_BLUE_SIZE,       8,
       EGL_ALPHA_SIZE,      8,
       EGL_NONE
   };

   // choose config
   if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
      return nullptr;
   }

	// bind the OpenGL API to the EGL
   if (!eglBindAPI(EGL_OPENGL_ES_API))
	{
		std::cerr << "egl: error binding api" << std::endl;
		return nullptr;
	}

	// Create a GL context
	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };   
   context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs );
   if (context == EGL_NO_CONTEXT) {
      return nullptr;
   }

   // create a surface
   surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)native_window, NULL);
   if (surface == EGL_NO_SURFACE) {
      return nullptr;
   }  
   
   auto ctx = std::make_shared<EglContext>(context, display, surface);

   // make current
   ctx->bind();

   // dump some context info
   std::cout << "opengl: vendor=" << glGetString(GL_VENDOR) << std::endl;
   std::cout << "opengl: version=" << glGetString(GL_VERSION) << std::endl;
   std::cout << "opengl: renderer=" << glGetString(GL_RENDERER) << std::endl;
   
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	return ctx;
}
