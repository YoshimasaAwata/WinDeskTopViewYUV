#pragma once

#include "framework.h"

class DemoApp
{
private:
    HWND m_hwnd;
    ID2D1Factory* m_pDirect2dFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pLightSlateGrayBrush;
    ID2D1SolidColorBrush* m_pCornflowerBlueBrush; 

public:
    DemoApp();
    virtual ~DemoApp();

    // Register the window class and call methods for instantiating drawing resources
    virtual HRESULT Initialize();

    // Process and dispatch messages
    void RunMessageLoop();

protected:
    void SetHWnd(HWND hwnd)
    {
        m_hwnd = hwnd;
    }

    HWND GetHWnd() const
    {
        return m_hwnd;
    };

    void SetFactory(ID2D1Factory* pFactory)
    {
        m_pDirect2dFactory = pFactory;
    }

    ID2D1Factory* GetFactory() const
    {
        return m_pDirect2dFactory;
    }

    void SetHwndRenderTarget(ID2D1HwndRenderTarget* pRenderTarget)
    {
        m_pRenderTarget = pRenderTarget;
    }

    ID2D1HwndRenderTarget* GetHwndRenderTarget() const
    {
        return m_pRenderTarget;
    };

    // Initialize device-independent resources.
    HRESULT CreateDeviceIndependentResources();

    // Initialize device-dependent resources.
    virtual HRESULT CreateDeviceResources();

    // Release device-dependent resource.
    virtual void DiscardDeviceResources();

    // Draw content.
    virtual HRESULT OnRender();

    // Resize the render target.
    void OnResize(
        UINT width,
        UINT height
    );

    // The windows procedure.
    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    // 情報表示ダイアログ用コールバック
    static INT_PTR CALLBACK About(
        HWND hDlg, 
        UINT message, 
        WPARAM wParam, 
        LPARAM lParam
    );
};