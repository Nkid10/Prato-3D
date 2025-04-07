// main.cpp : Define o ponto de entrada para a aplicação.

#include <Windows.h>
#include <commctrl.h>
#include "Painter.h"

#include "gl/gl.h"

#include <iostream>

#if defined(_WIN64)
#pragma comment(lib, "win64_lib/opengl32.lib") // Linka a biblioteca OpenGL para sistemas 64 bits
#pragma comment(lib, "win64_lib/glew32.lib") // Linka a biblioteca GLEW para sistemas 64 bits
#elif defined(_WIN32)
#pragma comment(lib, "win32_lib/opengl32.lib") // Linka a biblioteca OpenGL para sistemas 32 bits
#pragma comment(lib, "win32_lib/glew32.lib") // Linka a biblioteca GLEW para sistemas 32 bits
#endif


// Declaração da função de tratamento de mensagens da janela
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

HWND hWndStatusBar; // Handle (descritor) do componente StatusBar

HGLRC hglrc; // Contexto OpenGL (Handle to GL Rendering Context)

// Função para criar uma janela de console para exibir mensagens de erro
void CreateLogConsole(void)
{
    AllocConsole(); // Aloca uma nova janela de console
    SetConsoleOutputCP(1251); // Define a página de código da console para suportar caracteres cirílicos
    SetConsoleCP(1251);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // Obtém o handle da console

    // Altera o atributo de cor do texto na console para vermelho brilhante
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);

    // Redireciona os fluxos stderr e stdout para a console
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stderr); // Redireciona stderr
    freopen_s(&stream, "CONOUT$", "w", stdout); // Redireciona stdout
}

// Ponto de entrada principal do programa
int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
    char szWinName[] = "Graphics Window Class"; // Nome da classe da janela
    HWND hWnd; // Handle da janela principal
    WNDCLASSA wcl; // Estrutura para definir a classe da janela

    // Configura a classe da janela
    wcl.hInstance = hThisInstance; // Instância do aplicativo
    wcl.lpszClassName = szWinName; // Nome da classe da janela
    wcl.lpfnWndProc = WindowProc; // Função de tratamento de mensagens
    wcl.style = 0; // Estilo padrão
    wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Ícone padrão
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW); // Cursor padrão
    wcl.lpszMenuName = NULL; // Sem menu
    wcl.cbClsExtra = 0; // Sem informações adicionais
    wcl.cbWndExtra = 0;
    wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // Fundo branco

    // Registra a classe da janela
    if (!RegisterClassA(&wcl))
        return 0;

    // Cria a janela principal
    hWnd = CreateWindowA(szWinName,
        "Laboratório de Trabalho №6. Aplicação OpenGL",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // Estilo da janela
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Posição e tamanho padrão
        HWND_DESKTOP, // Sem janela pai
        NULL, // Sem menu
        hThisInstance, // Instância do aplicativo
        NULL); // Sem argumentos adicionais

    // Cria um componente StatusBar
    hWndStatusBar = CreateWindowExA(
        0, STATUSCLASSNAMEA, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, // Estilo do StatusBar
        0, 0, 0, 0, // Posição e tamanho inicial
        hWnd, (HMENU)10001, // Identificador do StatusBar
        hThisInstance, NULL);

    // Define as larguras das partes do StatusBar
    int statwidths[] = { 150, 300, -1 };
    SendMessageA(hWndStatusBar, SB_SETPARTS, sizeof(statwidths) / sizeof(int), (LPARAM)statwidths);

    ShowWindow(hWnd, nWinMode); // Exibe a janela

    setvbuf(stderr, NULL, _IONBF, 0); // Desativa o buffer do fluxo de erro para atualizações imediatas

    // Obtém o contexto do dispositivo de exibição
    HDC hdc = GetDC(hWnd);

    // Configura o formato de pixel
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd); // Escolhe o formato de pixel
    SetPixelFormat(hdc, pixelFormat, &pfd); // Define o formato de pixel

    // Cria o contexto OpenGL
    hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);

    // Inicializa a biblioteca GLEW
    glewInit();

    // Inicializa OpenGL
    InitOpenGL();

    UpdateWindow(hWnd); // Atualiza a janela

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) // Loop de mensagens
    {
        TranslateMessage(&msg); // Processa mensagens do teclado
        DispatchMessage(&msg); // Envia mensagens para a função de tratamento
    }

    return (int)msg.wParam;
}

// Função de tratamento de mensagens da janela
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static DWORD start_time; // Tempo inicial do programa

    switch (message)
    {
        // Trata a mensagem de criação da janela
    case WM_CREATE:
    {
        // Cria um timer que envia mensagens aproximadamente 30 vezes por segundo
        SetTimer(hWnd, 1, 1000 / 30, NULL);
        start_time = GetTickCount(); // Armazena o tempo inicial
    }
    break;

    // Trata a mensagem de redesenho da janela
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Obtém as dimensões da janela
        RECT rect = ps.rcPaint;
        GetClientRect(hWnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        // Calcula a altura da barra de status
        RECT rect_status;
        GetWindowRect(hWndStatusBar, &rect_status);
        int statusBarHeight = rect_status.bottom - rect_status.top;
        height -= statusBarHeight; // Subtrai a altura da barra de status

        if (height < 0) height = 0;

        // Calcula o tempo necessário para desenhar um quadro
        char repaint_time[500];
        DWORD t1 = GetTickCount();
        glViewport(0, statusBarHeight, width, height); // Define a área de desenho
        Draw(width, height); // Desenha o conteúdo
        SwapBuffers(hdc); // Troca os buffers para exibir o conteúdo

        // Formata o texto com informações de desempenho
        sprintf_s(repaint_time, "Tempo de redesenho: %d milissegundos\nTempo: %0.3f segundos\n",
            GetTickCount() - t1, running_time);

        // Define as cores de fundo e texto
        SetBkColor(hdc, RGB(0, 0, 0)); // Fundo preto
        SetTextColor(hdc, RGB(255, 127, 40)); // Texto laranja

        // Desenha o texto na janela
        DrawTextA(hdc, repaint_time, -1, &rect, 0);

        EndPaint(hWnd, &ps);
    }
    break;

    // Trata a mensagem de movimento do mouse
    case WM_MOUSEMOVE:
    {
        char str[256];
        // Exibe as coordenadas do cursor na barra de status
        sprintf_s(str, "X = %d, Y = %d", LOWORD(lParam), HIWORD(lParam));
        SendMessageA(hWndStatusBar, SB_SETTEXTA, 0, (LPARAM)str);

        if (wParam == MK_LBUTTON) // Se o botão esquerdo do mouse estiver pressionado
        {
            // Calcula o deslocamento do cursor
            int x = LOWORD(lParam), y = HIWORD(lParam);
            int dx = x - mousePosition.x;
            int dy = y - mousePosition.y;

            // Atualiza a matriz de rotação com base no movimento do mouse
            changeRotateMatrix(dx, dy);

            // Atualiza a posição do mouse
            mousePosition = { x, y };

            InvalidateRect(hWnd, NULL, false); // Solicita a atualização da janela
        }
    }
    break;

    // Trata a mensagem de clique do botão esquerdo do mouse
    case WM_LBUTTONDOWN:
        // Armazena a posição atual do cursor
        mousePosition = { LOWORD(lParam), HIWORD(lParam) };
        InvalidateRect(hWnd, NULL, false); // Solicita a atualização da janela
        break;

        // Trata a mensagem de pressionamento de tecla
    case WM_KEYDOWN:
        if (wParam == VK_F1) // Se a tecla F1 for pressionada
        {
            MessageBoxA(hWnd, "Trabalho realizado pelo estudante do grupo PV-221 Kolesnikov A.I.",
                "Sobre o programa", MB_ICONINFORMATION);
        }
        if (wParam == VK_ESCAPE) // Se a tecla ESC for pressionada
        {
            PostQuitMessage(0); // Encerra o programa
        }
        break;

        // Trata a mensagem de redimensionamento da janela
    case WM_SIZE:
        // Ajusta o tamanho da barra de status
        SendMessageA(hWndStatusBar, WM_SIZE, 0, 0);
        InvalidateRect(hWnd, NULL, false); // Solicita a atualização da janela
        break;

        // Trata a mensagem do timer
    case WM_TIMER:
        // Atualiza o tempo decorrido desde o início do programa
        running_time = (GetTickCount() - start_time) / 1000.0f;
        InvalidateRect(hWnd, NULL, false); // Solicita a atualização da janela
        break;

        // Trata a mensagem de destruição da janela
    case WM_DESTROY:
        wglMakeCurrent(GetDC(hWnd), NULL); // Libera o contexto OpenGL
        wglDeleteContext(hglrc); // Deleta o contexto OpenGL
        PostQuitMessage(0); // Encerra o programa
        break;

    default:
        // Passa mensagens não tratadas para o manipulador padrão
        return DefWindowProcA(hWnd, message, wParam, lParam);
    }

    return 0;
}