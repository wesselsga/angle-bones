#pragma once

#include <memory>

namespace opengl {

   class Context
   {
   public:
	   virtual ~Context(){}

      virtual void bind()=0;
	   virtual void unbind()=0;
	   virtual void clear()=0;
	   virtual void swapBuffers()=0;

   };

   std::shared_ptr<Context> create_context(void* native_window);   

}