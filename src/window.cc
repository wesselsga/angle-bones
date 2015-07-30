#include "window.h"

wchar_t const* _winClass = L"_main_win";
HINSTANCE _winModule = GetModuleHandle(0);

Window::Window() : _hwnd(0) 
{
   // win class registration

	WNDCLASSEX wcls;
	wcls.cbSize = sizeof(WNDCLASSEX);
	wcls.cbClsExtra = 0;
	wcls.cbWndExtra = 0;
	wcls.hbrBackground = 0;
	wcls.hCursor = LoadCursor(0, IDC_ARROW);
	wcls.hIcon = 0;
	wcls.hIconSm = 0;
	wcls.hInstance = _winModule;
	wcls.lpfnWndProc = _winProc;
	wcls.lpszClassName = _winClass;
	wcls.lpszMenuName = 0;
	wcls.style = CS_OWNDC;
	RegisterClassEx(&wcls);
}

Window::~Window()
{
	if (IsWindow(_hwnd)) {
		CloseWindow(_hwnd);
	}
}

int32_t Window::process_events()
{
   int32_t count = 0;

	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT) {
			return -1;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
      ++count;
	}	

	return count;
}

void* Window::handle() const 
{ 
   return _hwnd; 
}	

void Window::show() const
{
	ShowWindow(_hwnd, SW_SHOWNORMAL);
}

void Window::update() const
{
	UpdateWindow(_hwnd);
}

uint32_t Window::width() const
{
   RECT r;
   GetClientRect(_hwnd, &r);
   return r.right - r.left;
}

uint32_t Window::height() const
{
   RECT r;
   GetClientRect(_hwnd, &r);
   return r.bottom - r.top;
}	

//
// binds a function to the given event name
//
void Window::bind(
         std::string const& name, 
         std::function<void(Window*)> const& handler)
{
   _events[name]=handler;
}

//
// removes the handler for the event name
//
void Window::unbind(std::string const& name)
{
   auto const i = _events.find(name);
   if (i != _events.end()){
      _events.erase(i);
   }
}

//
// fires the given event by checking for a binding
// and invoking the callback
//
void Window::fire_event(std::string const& name)
{
   auto const i = _events.find(name);
   if (i != _events.end()){
      i->second(this);
   }
}

LRESULT CALLBACK Window::_winProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
	Window* pwin;
	if (msg == WM_NCCREATE)
	{
		auto cs = reinterpret_cast<CREATESTRUCT*>(lp);		
		pwin = reinterpret_cast<Window*>(cs->lpCreateParams);
		if (pwin)
		{
			pwin->_hwnd = hwnd;
			SetWindowLongPtr(hwnd,GWLP_USERDATA,(DWORD_PTR)pwin);
		}		
	}
	else
	{
		pwin = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));		
	}

	if (pwin)
	{
		LRESULT lr;
		if (pwin->winProc(msg, wp, lp, lr)) {
			return lr;
		}
	}

	return DefWindowProc(hwnd, msg ,wp, lp);		
}

bool Window::winProc(uint32_t msg, WPARAM wp, LPARAM, LRESULT& result)
{
	result = 0;

	switch (msg)
	{
		case WM_KEYDOWN:
			if (wp == VK_ESCAPE){
				PostMessage(_hwnd, WM_CLOSE, 0, 0);
			}
			break;

		case WM_SIZE:
         fire_event("onsize");
         break;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				BeginPaint(_hwnd, &ps);

				EndPaint(_hwnd, &ps);
			}
			return true;
		
		case WM_DESTROY:
			PostMessage(_hwnd, WM_QUIT, 1, 0);
			break;

		case WM_NCDESTROY:
			_hwnd = 0;
			break;

		default: break;
	}

	return false;
}

std::shared_ptr<Window> create_window(
         std::string const& title,
         uint32_t width,
         uint32_t height)
{
   auto win = std::make_shared<Window>();
   
	DWORD style = WS_OVERLAPPEDWINDOW;
	DWORD exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

	// rc is the desired client size we want
	RECT rc = {0, 0, width, height};
	AdjustWindowRectEx(&rc, style, FALSE, 0);

   std::shared_ptr<wchar_t> caption;
   if (!title.empty())
   {
      int32_t cch = MultiByteToWideChar(
         CP_UTF8, 0, title.c_str(), title.size(), 0, 0);
      if (cch > 0)
      {
         caption.reset(new wchar_t[cch+1]);
         MultiByteToWideChar(
               CP_UTF8, 0, title.c_str(), title.size(), caption.get(), cch); 
         *(caption.get()+cch) = L'\0';
      }
   }
   	
	HWND hwnd = CreateWindowEx(exstyle,
					_winClass,
					caption.get(),
					style,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					100,100,
					0,
					0,
					_winModule,
					(LPVOID)win.get());

	if (!IsWindow(hwnd)) {
		return nullptr;
	}

   SetWindowPos(hwnd, 0, 0, 0, 
         rc.right-rc.left,
         rc.bottom-rc.top,
         SWP_NOZORDER|SWP_NOMOVE|SWP_NOSENDCHANGING);

	return win;
}