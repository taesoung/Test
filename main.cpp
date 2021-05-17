#define WIN32_LEAN_AND_MEAN

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

bool g_keys[256];
LPCWSTR g_applicationName;
HINSTANCE g_hinstance;
HWND g_hwnd;

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

struct MatrixBufferType
{
	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX projection;
};

DWORD g_indices[] = {
	// front face
	0, 1, 2,
	0, 2, 3,

	// back face
	4, 6, 5,
	4, 7, 6,

	// left face
	4, 5, 1,
	4, 1, 0,

	// right face
	3, 2, 6,
	3, 6, 7,

	// top face
	1, 5, 6,
	1, 6, 2,

	// bottom face
	4, 0, 3,
	4, 3, 7
};

int g_vertexCount = 8;
int g_indexCount = 36;

// Do NOT modify the followiong codes
class GraphicsClass
{
public:
	GraphicsClass();
	void Shutdown();

	bool Initialize(int, int, HWND);

	bool InitializeD3D(int, int, HWND);
	bool InitializeBuffers();
	bool InitializeShader(HWND, const WCHAR*, const WCHAR*);
	bool RenderShader(D3DXMATRIX, D3DXMATRIX, D3DXMATRIX);
	bool Frame();

private:
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;

	D3DXMATRIX m_projectionMatrix;
	D3DXMATRIX m_worldMatrix;
	D3DXMATRIX m_viewMatrix;

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;

	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
};

// Do NOT modify the followiong codes
GraphicsClass::GraphicsClass()
{
	m_swapChain = 0, m_device = 0, m_deviceContext = 0, m_renderTargetView = 0;
	m_depthStencilBuffer = 0, m_depthStencilState = 0, m_depthStencilView = 0, m_rasterState = 0;
	m_vertexBuffer = 0, m_indexBuffer = 0;
	m_vertexShader = 0, m_pixelShader = 0, m_layout = 0, m_matrixBuffer = 0;
}

// Do NOT modify the followiong codes
void GraphicsClass::Shutdown()
{
	if (m_swapChain)			{		m_swapChain->SetFullscreenState(false, NULL);	}
	if (m_rasterState)			{		m_rasterState->Release();			m_rasterState = 0;	}
	if (m_depthStencilView)		{		m_depthStencilView->Release();		m_depthStencilView = 0;	}
	if (m_depthStencilState)	{		m_depthStencilState->Release();		m_depthStencilState = 0;	}
	if (m_depthStencilBuffer)	{		m_depthStencilBuffer->Release();	m_depthStencilBuffer = 0;	}
	if (m_renderTargetView)		{		m_renderTargetView->Release();		m_renderTargetView = 0;	}
	if (m_deviceContext)		{		m_deviceContext->Release();			m_deviceContext = 0;	}
	if (m_device)				{		m_device->Release();				m_device = 0;	}
	if (m_swapChain)			{		m_swapChain->Release();				m_swapChain = 0;	}
	if (m_indexBuffer)			{		m_indexBuffer->Release();			m_indexBuffer = 0;	}
	if (m_vertexBuffer)			{		m_vertexBuffer->Release();			m_vertexBuffer = 0;	}
	if (m_matrixBuffer)			{		m_matrixBuffer->Release();			m_matrixBuffer = 0;	}
	if (m_layout)				{		m_layout->Release();				m_layout = 0;		}
	if (m_pixelShader)			{		m_pixelShader->Release();			m_pixelShader = 0;	}
	if (m_vertexShader)			{		m_vertexShader->Release();			m_vertexShader = 0;	}
}

// Do NOT modify the followiong codes
bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	result = InitializeD3D(screenWidth, screenHeight, hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	result = InitializeBuffers();
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model buffers.", L"Error", MB_OK);
		return false;
	}

	result = InitializeShader(hwnd, L"./VertexColor.vs", L"./VertexColor.ps");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the shaders.", L"Error", MB_OK);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
// START OF MODIFICATION
///////////////////////////////////////////////////////////////////////////////////////

struct VertexType
{
	VertexType() {}
	VertexType(float x, float y, float z)
	: pos(x, y, z) {}

	D3DXVECTOR3 pos;
};

VertexType g_vertices[] =
{
	VertexType(-1.0f, -1.0f, -1.0f),
	VertexType(-1.0f, +1.0f, -1.0f),
	VertexType(+1.0f, +1.0f, -1.0f),
	VertexType(+1.0f, -1.0f, -1.0f),
	VertexType(-1.0f, -1.0f, +1.0f),
	VertexType(-1.0f, +1.0f, +1.0f),
	VertexType(+1.0f, +1.0f, +1.0f),
	VertexType(+1.0f, -1.0f, +1.0f),
};

bool GraphicsClass::InitializeD3D(int screenWidth, int screenHeight, HWND hwnd)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator, stringLength;
	DXGI_MODE_DESC* displayModeList; 
	DXGI_ADAPTER_DESC adapterDesc; 
	int error; DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr; 
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc; 
	D3D11_RASTERIZER_DESC rasterDesc; D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;

	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result)) { return false; }
	result = factory->EnumAdapters(0, &adapter);
	if(FAILED(result)) { return false; } 
	result = adapter->EnumOutputs(0, &adapterOutput); 
	if(FAILED(result)) { return false; }
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL); if(FAILED(result)) { return false; }
	displayModeList = new DXGI_MODE_DESC[numModes]; if(!displayModeList) { return false; } 
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if(FAILED(result)) { return false; } 
	for(i=0; i<numModes; i++) { 
		if(displayModeList[i].Width == (unsigned int)screenWidth) { 
			if(displayModeList[i].Height == (unsigned int)screenHeight) {
				numerator = displayModeList[i].RefreshRate.Numerator; denominator = displayModeList[i].RefreshRate.Denominator; 
			} 
		} 
	}
	
	delete [] displayModeList; 
	displayModeList = 0; 
	adapterOutput->Release();
	adapterOutput = 0; 
	adapter->Release(); 
	adapter = 0; 
	factory->Release(); factory = 0;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc)); 
	swapChainDesc.BufferCount = 1; 
	swapChainDesc.BufferDesc.Width = screenWidth; swapChainDesc.BufferDesc.Height = screenHeight; 
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator; 
swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; 
swapChainDesc.OutputWindow = hwnd; 
swapChainDesc.SampleDesc.Count = 1;
swapChainDesc.SampleDesc.Quality = 0; 
swapChainDesc.Windowed = true; 
swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; 
swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; 
swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; 
swapChainDesc.Flags = 0;
featureLevel = D3D_FEATURE_LEVEL_11_0;
result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext); 
if (FAILED(result)) { return false; }
result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
if (FAILED(result))
{
	return false;
}

// Create the render target view with the back buffer pointer.
result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
if (FAILED(result))
{
	return false;
}

// Release pointer to the back buffer as we no longer need it.
backBufferPtr->Release();
backBufferPtr = 0;

// Initialize the description of the depth buffer.
ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

// Set up the description of the depth buffer.
depthBufferDesc.Width = screenWidth;
depthBufferDesc.Height = screenHeight;
depthBufferDesc.MipLevels = 1;
depthBufferDesc.ArraySize = 1;
depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
depthBufferDesc.SampleDesc.Count = 1;
depthBufferDesc.SampleDesc.Quality = 0;
depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
depthBufferDesc.CPUAccessFlags = 0;
depthBufferDesc.MiscFlags = 0;

// Create the texture for the depth buffer using the filled out description.
result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
if (FAILED(result))
{
	return false;
}

// Initialize the description of the stencil state.
ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

// Set up the description of the stencil state.
depthStencilDesc.DepthEnable = true;
depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

depthStencilDesc.StencilEnable = true;
depthStencilDesc.StencilReadMask = 0xFF;
depthStencilDesc.StencilWriteMask = 0xFF;

// Stencil operations if pixel is front-facing.
depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

// Stencil operations if pixel is back-facing.
depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

// Create the depth stencil state.
result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
if (FAILED(result))
{
	return false;
}

// Set the depth stencil state.
m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

// Initialize the depth stencil view.
ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

// Set up the depth stencil view description.
depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
depthStencilViewDesc.Texture2D.MipSlice = 0;

// Create the depth stencil view.
result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
if (FAILED(result))
{
	return false;
}

// Bind the render target view and depth stencil buffer to the output render pipeline.
m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

// Setup the raster description which will determine how and what polygons will be drawn.
rasterDesc.AntialiasedLineEnable = false;
rasterDesc.CullMode = D3D11_CULL_BACK;
rasterDesc.DepthBias = 0;
rasterDesc.DepthBiasClamp = 0.0f;
rasterDesc.DepthClipEnable = true;
rasterDesc.FillMode = D3D11_FILL_SOLID;
rasterDesc.FrontCounterClockwise = false;
rasterDesc.MultisampleEnable = false;
rasterDesc.ScissorEnable = false;
rasterDesc.SlopeScaledDepthBias = 0.0f;

// Create the rasterizer state from the description we just filled out.
result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
if (FAILED(result))
{
	return false;
}

// Now set the rasterizer state.
m_deviceContext->RSSetState(m_rasterState);

// Setup the viewport for rendering.
viewport.Width = (float)screenWidth;
viewport.Height = (float)screenHeight;
viewport.MinDepth = 0.0f;
viewport.MaxDepth = 1.0f;
viewport.TopLeftX = 0.0f;
viewport.TopLeftY = 0.0f;

// Create the viewport.
m_deviceContext->RSSetViewports(1, &viewport);

// Setup the projection matrix.
fieldOfView = (float)D3DX_PI / 4.0f;
screenAspect = (float)screenWidth / (float)screenHeight;

// Create the projection matrix for 3D rendering.
D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, 1000.0f, 0.1f);

// Initialize the world matrix to the identity matrix.
D3DXMatrixIdentity(&m_worldMatrix);


return true;

}

bool GraphicsClass::InitializeBuffers()
{
	// Add codes here
	return true;
}

bool GraphicsClass::InitializeShader(HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename)
{
	// Add codes here
	return true;
}

bool GraphicsClass::RenderShader(D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
{
	// Add codes here
	return true;
}

bool GraphicsClass::Frame()
{
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = 0;
	color[1] = 0;
	color[2] = 1;
	color[3] = 0.0f;
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	bool result;
	int screenWidth, screenHeight;

	screenWidth = 800;
	screenHeight = 600;

	// Initialize the windows api.
	WNDCLASSEX wc;
	int posX, posY;

	// Get the instance of this application.
	g_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	g_applicationName = L"Template";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Place the window in the middle of the screen.
	posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
	posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

	// Create the window with the screen settings and get the handle to it.
	g_hwnd = CreateWindowEx(WS_EX_APPWINDOW, g_applicationName, g_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, g_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(g_hwnd, SW_SHOW);
	SetForegroundWindow(g_hwnd);
	SetFocus(g_hwnd);

	// Hide the mouse cursor.
	ShowCursor(false);

	for (int i = 0; i < 256; i++)
	{
		g_keys[i] = false;
	}

	GraphicsClass *pGraphics;
	pGraphics = new GraphicsClass;
	if (!pGraphics)
	{
		return 0;
	}

	result = pGraphics->Initialize(screenWidth, screenHeight, g_hwnd);
	if (!result)
	{
		return 0;
	}

	if (result)
	{
		MSG msg;
		bool done;

		// Initialize the message structure.
		ZeroMemory(&msg, sizeof(MSG));

		// Loop until there is a quit message from the window or the user.
		done = false;
		while (!done)
		{
			// Handle the windows messages.
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// If windows signals to end the application then exit out.
			if (msg.message == WM_QUIT)
			{
				done = true;
			}
			else
			{
				// Otherwise do the frame processing.

				// Check if the user pressed escape and wants to exit the application.
				if (g_keys[VK_ESCAPE])
				{
					done = true;
				}

				// Do the frame processing for the graphics object.
				if (!pGraphics->Frame())
				{
					done = true;
				}
			}
		}
	}

	// Shutdown and release the system object.
	// Release the graphics object.
	pGraphics->Shutdown();
	delete pGraphics;
	pGraphics = 0;

	// Shutdown the window.
	// Show the mouse cursor.
	ShowCursor(true);

	// Remove the window.
	DestroyWindow(g_hwnd);
	g_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(g_applicationName, g_hinstance);
	g_hinstance = NULL;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// END OF MODIFICATION
///////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_KEYDOWN:
	{
		g_keys[(unsigned int)wparam] = true;
		return 0;
	}

	case WM_KEYUP:
	{
		g_keys[(unsigned int)wparam] = false;
		return 0;
	}

	default:
	{
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}

	}

}



