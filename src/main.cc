#include "window.h"
#include "opengl.h"

#include <iostream>

#include <GLES2/gl2.h>

//
// simple helper to compile a shader and return the identifier
//
// returns > 0 on success
//
GLuint load_shader(char const* source, GLenum type)
{
	// create the shader object
   GLuint shader = glCreateShader(type);
   if (!shader){
      return 0;
	}

   // Load the shader source
   glShaderSource(shader, 1, &source, nullptr);
   
   // Compile the shader
   glCompileShader(shader);

   // Check the compile status
   GLint status;
   glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) 
   {
      GLint msglen = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &msglen);
      
      if (msglen > 1)
      {
         std::shared_ptr<char> msg(
               reinterpret_cast<char*>(malloc(sizeof(char) * msglen)), free);
         glGetShaderInfoLog(shader, msglen, nullptr, msg.get());
			std::cerr << "error compiling shader: " << std::endl << msg.get();
      }

      glDeleteShader(shader);
      shader = 0;
   }

   return shader;
}

//
// compile+link our shaders
//
bool setup_world()
{
   const char* basic_vertex = R"GLSL(

      attribute vec4 a_pos;
      void main()
      {
	      gl_Position = a_pos;
      }

      )GLSL";

   const char* basic_frag = R"GLSL(

      precision mediump float;
      void main()
      {
         gl_FragColor = vec4(0.25,0.50,1.0,1.0);
      }

      )GLSL";

   auto const vertex_id = load_shader(basic_vertex, GL_VERTEX_SHADER);
   auto const frag_id = load_shader(basic_frag, GL_FRAGMENT_SHADER);

   // create the program object
   GLuint program = glCreateProgram();
   if (!program)
   {
      glDeleteShader(vertex_id);
      glDeleteShader(frag_id);      
      return false;
	}

   glAttachShader(program, vertex_id);
   glAttachShader(program, frag_id);

   glBindAttribLocation(program, 0, "a_pos");
	
	// Link the program
   glLinkProgram(program);

   // done with the shaders
   glDeleteShader(vertex_id);
   glDeleteShader(frag_id);
   
	// Check the link status
	GLint status;
   glGetProgramiv(program, GL_LINK_STATUS, &status);   
	if (!status) 
   {
      GLint msglen = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &msglen);      
      if (msglen > 1)
      {
         std::shared_ptr<char> msg(
               reinterpret_cast<char*>(malloc(sizeof(char) * msglen)), free);
         glGetProgramInfoLog(program, msglen, nullptr, msg.get());
			std::cerr << "error linking program: " << std::endl << msg.get();
      }
      
		glDeleteProgram(program);
      return false;
   }

   glUseProgram(program);

   return true;
}

//
// draws a simple box
//
void render_scene()
{
   GLfloat quad[] = 
   {  
      -0.5f, 0.5f, 0.0f,
      -0.5f, -0.5f, 0.0f,
      0.5f, 0.5f, 0.0f,
      0.5f, -0.5f, 0.0f
   };
   
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, quad);
   glEnableVertexAttribArray(0);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);   
}

//
// called whenver our window is resized
//
void on_size(Window* win)
{
   // update the gl viewport
   auto const w = win->width();
   auto const h = win->height();
   glViewport(0, 0, w, h);
}

int main(int argc, char* argv[])
{
   auto win = create_window("ANGLE Bare-bones", 640, 480);
   
   // setup a handler so we know when the window is sized
   win->bind("onsize", on_size);

   auto gl_context = opengl::create_context(win->handle());

   if (gl_context)
   {
      win->show();

      // compile shaders etc...
      if (setup_world())
      {
         int32_t ret;
         for (;;)
         {
            ret = win->process_events();      
            if (!ret)
            {
               gl_context->clear();

               render_scene();
            
               gl_context->swapBuffers();
         
            }
            else if (ret < 0){
               break; // abort
            }
         }
      }
   }
   
	std::cout << "program exit." << std::endl;

	return 0;
}