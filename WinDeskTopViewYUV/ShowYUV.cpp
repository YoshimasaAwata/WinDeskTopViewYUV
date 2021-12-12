#pragma warning(disable:4996)

#include "ShowYUV.h"
#include "Resource.h"

ShowYUV::ShowYUV():
	m_pInFile(NULL),
    m_pBitmap(NULL)
{
	m_pY = new BYTE[CIF_WIDTH * CIF_HEIGHT];
	m_pU = new BYTE[(CIF_WIDTH / 2) * (CIF_HEIGHT / 2)];
	m_pV = new BYTE[(CIF_WIDTH / 2) * (CIF_HEIGHT / 2)];
	m_pRGB = new UINT[CIF_WIDTH * CIF_HEIGHT];
    ZeroMemory(m_pRGB, sizeof(UINT) * CIF_WIDTH * CIF_HEIGHT);
}

ShowYUV::~ShowYUV()
{
	if (NULL != m_pInFile)
	{
		m_pInFile->close();
	}
	delete[] m_pY;
	delete[] m_pU;
	delete[] m_pV;
	delete[] m_pRGB;
    SafeRelease(&m_pBitmap);
}

HRESULT ShowYUV::Initialize()
{
    HRESULT hr;

    // Initialize device-indpendent resources, such
    // as the Direct2D factory.
    hr = CreateDeviceIndependentResources();

    if (SUCCEEDED(hr))
    {
        // Register the window class.
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = DemoApp::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = HINST_THISCOMPONENT;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDESKTOPVIEWYUV);
        wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
        wcex.lpszClassName = L"D2DShowYUV";

        RegisterClassEx(&wcex);

        // Because the CreateWindow function takes its size in pixels,
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX, dpiY;

        // The factory returns the current system DPI. This is also the value it will use
        // to create its own windows.
        GetFactory()->GetDesktopDpi(&dpiX, &dpiY);

        // Create the window.
        HWND hwnd = CreateWindow(
            L"D2DShowYUV",
            L"Direct2D Show YUV",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(CIF_WIDTH * dpiX / 96.f)),
            static_cast<UINT>(ceil(CIF_HEIGHT * dpiY / 96.f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
        );
        hr = hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            SetHWnd(hwnd);
            ShowWindow(hwnd, SW_SHOWNORMAL);
            UpdateWindow(hwnd);
        }
    }

    return hr;
}

HRESULT ShowYUV::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!GetHwndRenderTarget())
    {
        RECT rc;
        GetClientRect(GetHWnd(), &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
        );

        // Create a Direct2D render target.
        ID2D1HwndRenderTarget* pRenderTarget;
        hr = GetFactory()->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(GetHWnd(), size),
            &pRenderTarget
        );
        if (SUCCEEDED(hr))
        {
            SetHwndRenderTarget(pRenderTarget);
        }

        if (SUCCEEDED(hr))
        {
            // Create a bitmap.
            D2D1_BITMAP_PROPERTIES bitmapProperties;
            GetFactory()->GetDesktopDpi(&(bitmapProperties.dpiX), &(bitmapProperties.dpiY));
            bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8X8_UNORM;
            bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
            hr = GetHwndRenderTarget()->CreateBitmap(
                D2D1::SizeU(CIF_WIDTH, CIF_HEIGHT),
                m_pRGB,
                sizeof(UINT) * CIF_WIDTH,
                &bitmapProperties,
                &m_pBitmap
            );
        }
    }

    return hr;
}

void ShowYUV::DiscardDeviceResources()
{
    SafeRelease(&m_pBitmap);
    DemoApp::DiscardDeviceResources();
}

HRESULT ShowYUV::OnRender()
{
    HRESULT hr = S_OK;

    hr = CreateDeviceResources();
    if (SUCCEEDED(hr))
    {
        GetHwndRenderTarget()->BeginDraw();

        GetHwndRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());

        D2D1_SIZE_F rtSize = GetHwndRenderTarget()->GetSize();

        GetHwndRenderTarget()->DrawBitmap(
            m_pBitmap,
            D2D1::RectF(0.0f, 0.0f, rtSize.width, rtSize.height),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            D2D1::RectF(0.0f, 0.0f, CIF_WIDTH, CIF_HEIGHT)
        );

        hr = GetHwndRenderTarget()->EndDraw();
    }

    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardDeviceResources();
    }

    return hr;
}

LRESULT CALLBACK ShowYUV::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        ShowYUV* pShowYUV = (ShowYUV*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pShowYUV)
        );

        result = 1;
    }
    else
    {
        ShowYUV* pShowYUV = reinterpret_cast<ShowYUV*>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
            )));

        bool wasHandled = false;

        if (pShowYUV)
        {
            switch (message)
            {
            case WM_COMMAND:
            {
                int wmId = LOWORD(wParam);
                // ‘I‘ð‚³‚ê‚½ƒƒjƒ…[‚Ì‰ðÍ:
                switch (wmId)
                {
                case IDM_ABOUT:
                    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    break;
                default:
                    return DefWindowProc(hwnd, message, wParam, lParam);
                }
            }
            break;
            case WM_SIZE:
            {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                pShowYUV->OnResize(width, height);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_DISPLAYCHANGE:
            {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_PAINT:
            {
                pShowYUV->OnRender();
                ValidateRect(hwnd, NULL);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            result = 1;
            wasHandled = true;
            break;
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}
