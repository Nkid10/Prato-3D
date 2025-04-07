# **Projeto de Renderização 3D de Prato com OpenGL**

![OpenGL Logo](https://upload.wikimedia.org/wikipedia/commons/5/5e/OpenGL_Logo.png)

Este projeto implementa um programa OpenGL que renderiza um prato 3D com várias camadas e efeitos de iluminação. Ele demonstra técnicas avançadas de renderização, incluindo geometria programática, iluminação Phong e interação do usuário.

---

## **Funcionalidades Principais**

- **Prato 3D com Múltiplas Camadas**:
  - O prato possui uma camada interna branca, uma divisória preta, uma camada externa branca, uma borda lateral e uma base.
  - Todas as partes são geradas programaticamente usando vértices, normais e cores.

- **Iluminação Phong**:
  - Implementa iluminação ambiente, difusa e especular.
  - A posição da luz pode ser ajustada dinamicamente.

- **Rotação Interativa**:
  - O usuário pode girar o prato ao mover o mouse.
  - A função `changeRotateMatrix` atualiza a matriz de rotação com base no movimento do mouse.

- **Renderização Otimizada**:
  - Usa Vertex Array Objects (VAOs) e Vertex Buffer Objects (VBOs) para armazenar e renderizar os dados de forma eficiente.

- **Animação Automática**:
  - O prato gira automaticamente ao longo do tempo, criando um efeito de animação contínua.

---

## **Requisitos de Sistema**

- **Sistema Operacional**: Windows, Linux ou macOS.
- **Bibliotecas Necessárias**:
  - OpenGL (via GLEW).
  - GLFW (para janela e contexto OpenGL).
  - GLM (para operações matemáticas de vetores e matrizes).
  - STL (biblioteca padrão do C++).
- **Hardware**:
  - Placa gráfica compatível com OpenGL 3.3 ou superior.

---

## **Configuração do Projeto**

### **1. Instalação das Dependências**
Certifique-se de que as seguintes bibliotecas estejam instaladas:

- **GLEW**: Para gerenciamento de extensões OpenGL.
- **GLFW**: Para criação de janelas e contexto OpenGL.
- **GLM**: Para operações matemáticas (vetores, matrizes, etc.).

Você pode instalar essas bibliotecas usando gerenciadores de pacotes como `vcpkg` (Windows) ou `apt` (Linux).

Exemplo no Linux:
```bash
sudo apt install libglew-dev libglfw3-dev libglm-dev
