#pragma warning(disable:4996)

#include "ShowYUV.h"
#include "Resource.h"
#include <commdlg.h>

using namespace DirectX;

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
        wcex.lpfnWndProc = ShowYUV::WndProc;
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

void ShowYUV::OpenYUVFile()
{
    OPENFILENAME ofn;
    TCHAR file_name[MAX_PATH];

    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ZeroMemory(file_name, sizeof(TCHAR) * MAX_PATH);
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = GetHWnd();
    ofn.lpstrFile = file_name;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = _T("YUVファイル(*.yuv)\0*.yuv\0\0");
    ofn.lpstrTitle = _T("YUVファイルを選択します。");
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = _T("yuv");

    if (GetOpenFileName(&ofn))
    {
        if (m_pInFile != NULL)
        {
            m_pInFile->close();
            delete m_pInFile;
            m_pInFile = NULL;
        }
        m_pInFile = new std::ifstream(file_name, std::ios_base::in | std::ios_base::binary);
    }
}

LRESULT ShowYUV::ReadYUV()
{
    LRESULT result = 0;

    if ((m_pInFile != NULL) && m_pInFile->good())
    {
        m_pInFile->read(reinterpret_cast<char*>(m_pY), CIF_WIDTH * CIF_HEIGHT);
        m_pInFile->read(reinterpret_cast<char*>(m_pU), (CIF_WIDTH / 2) * (CIF_HEIGHT / 2));
        m_pInFile->read(reinterpret_cast<char*>(m_pV), (CIF_WIDTH / 2) * (CIF_HEIGHT / 2));
    }
    else
    {
        result = -1;
    }

    return result;
}

void ShowYUV::YUV2RGB()
{
    const XMVECTOR diff = { (16.0f / 256), (128.0f / 256), (128.0f / 256), 0.0f };

    for (int h = 0; h < CIF_HEIGHT; h++)
    {
        int y_pos = h * CIF_WIDTH;
        int uv_pos = (h / 2) * (CIF_WIDTH / 2);

        for (int w = 0; w < CIF_WIDTH; w++)
        {
            XMVECTOR yuvw =
            {
                static_cast<float>(m_pY[y_pos + w]),
                static_cast<float>(m_pU[uv_pos + (w / 2)]),
                static_cast<float>(m_pV[uv_pos + (w / 2)]),
                1.0f
            };
            yuvw /= 255.0f;
            yuvw = XMVectorSaturate(yuvw);
            yuvw -= diff;
            XMVECTOR rgba = XMColorYUVToRGB(yuvw);
            rgba = XMVectorSaturate(rgba);
            rgba *= 255.0f;
            rgba = XMVectorTruncate(rgba);
            UINT r = static_cast<UINT>(XMVectorGetX(rgba));
            UINT g = static_cast<UINT>(XMVectorGetY(rgba));
            UINT b = static_cast<UINT>(XMVectorGetZ(rgba));
            m_pRGB[y_pos + w] = (r << 16) | (g << 8) | b;
        }
    }

    if ((NULL != GetFactory()) && (NULL != GetHwndRenderTarget()))
    {
        SafeRelease(&m_pBitmap);

        D2D1_BITMAP_PROPERTIES bitmapProperties;
        GetFactory()->GetDesktopDpi(&(bitmapProperties.dpiX), &(bitmapProperties.dpiY));
        bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8X8_UNORM;
        bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
        GetHwndRenderTarget()->CreateBitmap(
            D2D1::SizeU(CIF_WIDTH, CIF_HEIGHT),
            m_pRGB,
            sizeof(UINT) * CIF_WIDTH,
            &bitmapProperties,
            &m_pBitmap
        );
    }

    return;
}

LRESULT ShowYUV::ShowNextFrame()
{
    LRESULT result = 0;
    result = ReadYUV();
    if (0 == result)
    {
        YUV2RGB();
        InvalidateRgn(GetHWnd(), NULL, FALSE);
        UpdateWindow(GetHWnd());
    }
    else
    {
        KillTimer(GetHWnd(), IDT_TIMER);
    }

    return result;
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
                // 選択されたメニューの解析:
                switch (wmId)
                {
                case IDM_OPEN:
                    pShowYUV->OpenYUVFile();
                    pShowYUV->ReadYUV();
                    pShowYUV->YUV2RGB();
                    InvalidateRgn(hwnd, NULL, FALSE);
                    UpdateWindow(hwnd);
                    SetTimer(hwnd, IDT_TIMER, INTERVAL, NULL);
                    break;
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
            result = 0;
            wasHandled = true;
            break;
            case WM_TIMER:
            {
                pShowYUV->ShowNextFrame();
            }
            result = 0;
            wasHandled = true;
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
