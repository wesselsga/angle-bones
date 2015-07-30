#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#endif

//
// a basic Win32 HWND wrapper
//
class Window
{
public:
   Window();   
   virtual ~Window();

   void* handle() const;
	void  show() const;
	void  update() const;

   uint32_t width() const;
   uint32_t height() const;
	
   void bind(
         std::string const& name, 
         std::function<void(Window*)> const& handler);

   void unbind(std::string const& name);

   int32_t process_events();

private:

   static LRESULT CALLBACK _winProc(HWND, UINT, WPARAM, LPARAM);
   bool winProc(uint32_t, WPARAM, LPARAM, LRESULT&);

   void fire_event(std::string const& name);

   HWND _hwnd;
   std::map<std::string, std::function<void(Window*)>> _events;

};

std::shared_ptr<Window> create_window(
            std::string const& title,
            uint32_t width,
            uint32_t height);
