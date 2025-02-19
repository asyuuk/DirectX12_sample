#include"RenderManager.h"	

int wmain(int argc, wchar_t** argv, wchar_t** evnp)
{
#if defined(DEBUG)||defined(_DEBUG)	
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	RenderManager render(960, 540);
	render.Run();

	return 0;
}