#include <windows.h>
#include <cmath>
#include <cstdlib>

const int WIDTH = 800;
const int HEIGHT = 600;
int clientWidth = WIDTH;
int clientHeight = HEIGHT;
bool collided = false;

struct Ball {
    float x, y;   // 중심 좌표
    float vx, vy; // 속도
    float r;      // 반지름
    COLORREF color;
};

Ball balls[4] = {
    { 150, 300,  3.0f, -2.0f, 60, RGB(220, 50, 50) },   // 큰 공
    { 650, 300, -3.0f, -1.0f, 30, RGB(50, 100, 220) },   // 작은 공
    { 400, 100,  1.5f,  2.0f, 45, RGB(50, 200, 100) },   // 중간 공
    { 300, 450, -2.0f, -2.5f, 25, RGB(220, 180, 50) },   // 작은 공
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_SIZE) {
        clientWidth = LOWORD(lParam);
        clientHeight = HIWORD(lParam);
    }
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 배경 지우기
        RECT rc; GetClientRect(hwnd, &rc);
        HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 35));
        FillRect(hdc, &rc, bgBrush);
        DeleteObject(bgBrush);

        // 공 그리기
        for (auto& b : balls) {
            HBRUSH brush = CreateSolidBrush(b.color);
            SelectObject(hdc, brush);
            Ellipse(hdc, (int)(b.x - b.r), (int)(b.y - b.r),
                (int)(b.x + b.r), (int)(b.y + b.r));
            DeleteObject(brush);
        }

        EndPaint(hwnd, &ps);
    }
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"PhysicsDemo";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(L"PhysicsDemo", L"Physics Demo - Day2",
        WS_OVERLAPPEDWINDOW, 100, 100, WIDTH, HEIGHT,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (true) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return 0;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 중력
        const float GRAVITY = 0.3f;
        for (auto& b : balls) {
            b.vy += GRAVITY;
        }

        // 물체 이동
        for (auto& b : balls) {
            b.x += b.vx;
            b.y += b.vy;
        }

        // 벽 충돌
        for (auto& b : balls) {
            if (b.x - b.r < 0) { b.x = b.r; b.vx *= -0.85f; }
            if (b.x + b.r > clientWidth) { b.x = clientWidth - b.r; b.vx *= -0.85f; }
            if (b.y - b.r < 0) { b.y = b.r; b.vy *= -0.85f; }
            if (b.y + b.r > clientHeight) {
                b.y = clientHeight - b.r;
                b.vy *= -0.85f;
                b.vx *= 0.92f;  // 바닥 마찰력
            }
        }

        // 공끼리 충돌 (모든 조합)
        for (int i = 0; i < 4; i++) {
            for (int j = i + 1; j < 4; j++) {
                float dx = balls[j].x - balls[i].x;
                float dy = balls[j].y - balls[i].y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist < balls[i].r + balls[j].r && dist > 0) {
                    float nx = dx / dist;
                    float ny = dy / dist;
                    float dvx = balls[i].vx - balls[j].vx;
                    float dvy = balls[i].vy - balls[j].vy;
                    float dot = dvx * nx + dvy * ny;
                    if (dot > 0) {
                        balls[i].vx -= dot * nx;
                        balls[i].vy -= dot * ny;
                        balls[j].vx += dot * nx;
                        balls[j].vy += dot * ny;
                        float overlap = (balls[i].r + balls[j].r - dist) / 2.0f;
                        balls[i].x -= overlap * nx;
                        balls[i].y -= overlap * ny;
                        balls[j].x += overlap * nx;
                        balls[j].y += overlap * ny;

                        balls[i].color = RGB(rand() % 255, rand() % 255, rand() % 255);
                        balls[j].color = RGB(rand() % 255, rand() % 255, rand() % 255);
                    }
                }
            }
        }

        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);

        Sleep(16);  // 약 60FPS
    }
    return 0;
}