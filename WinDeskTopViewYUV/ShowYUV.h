#pragma once

#include "DemoApp.h"
#include <fstream>

class ShowYUV :
    public DemoApp
{
public:
    static const int CIF_WIDTH = 352;
    static const int CIF_HEIGHT = 288;

private:
    BYTE* m_pY;
    BYTE* m_pU;
    BYTE* m_pV;
    UINT* m_pRGB;
    std::ifstream* m_pInFile;
    ID2D1Bitmap* m_pBitmap;

public:
    ShowYUV();
    virtual ~ShowYUV();

    // Register the window class and call methods for instantiating drawing resources
    virtual HRESULT Initialize();

protected:
    // Initialize device-dependent resources.
    virtual HRESULT CreateDeviceResources();

    // Release device-dependent resource.
    virtual void DiscardDeviceResources();

    // Draw content.
    virtual HRESULT OnRender();

    // Open YUV File using FileOpenDialog.
    virtual void OpenYUVFile();

    // Read YUV data.
    virtual LRESULT ReadYUV();

    // Convert YUV to RGB and create Bitmap.
    virtual void YUV2RGB();

    // The windows procedure.
    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );
};

