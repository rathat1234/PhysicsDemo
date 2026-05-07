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

Ball balls[2] = {
    { 150, 300, 3.0f, 0, 50, RGB(220, 50, 50) },   // 빨강: 오른쪽으로 이동
    { 650, 300, -3.0f, 0, 50, RGB(50, 100, 220) },  // 파랑: 왼쪽으로 이동
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
            if (b.x + b.r > clientWidth) { b.x = clientWidth - b.r; b.vx *= -0.85; }
            if (b.y - b.r < 0) { b.y = b.r; b.vy *= -0.85f; }
            if (b.y + b.r > clientHeight) { b.y = clientHeight - b.r; b.vy *= -0.85; }
        }

        // 공끼리 충돌
        float dx = balls[1].x - balls[0].x;
        float dy = balls[1].y - balls[0].y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < balls[0].r + balls[1].r && dist > 0) {

            if (!collided) {
                // 충돌 시 색상 랜덤 변경
                balls[0].color = RGB(rand() % 255, rand() % 255, rand() % 255);
                balls[1].color = RGB(rand() % 255, rand() % 255, rand() % 255);
                collided = true;
            }

            // 충돌 법선 벡터 (단위 벡터)
            float nx = dx / dist;
            float ny = dy / dist;

            // 상대 속도
            float dvx = balls[0].vx - balls[1].vx;
            float dvy = balls[0].vy - balls[1].vy;

            // 법선 방향 상대 속도
            float dot = dvx * nx + dvy * ny;

            // 이미 멀어지고 있으면 스킵
            if (dot > 0) {
                balls[0].vx -= dot * nx;
                balls[0].vy -= dot * ny;
                balls[1].vx += dot * nx;
                balls[1].vy += dot * ny;

                // 겹침 해소
                float overlap = (balls[0].r + balls[1].r - dist) / 2.0f;
                balls[0].x -= overlap * nx;
                balls[0].y -= overlap * ny;
                balls[1].x += overlap * nx;
                balls[1].y += overlap * ny;
            }
        }
        else {
            collided = false;
        }

        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);

        Sleep(16);  // 약 60FPS
    }
    return 0;
}