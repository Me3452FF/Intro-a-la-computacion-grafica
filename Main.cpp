#include <iostream> // Permite mostrar informacion en la consola con cout
#include <vector>   // Permite guardar todas las celdas y vertices en vectores
#include <cmath>    // Se usa para abs, floor, sqrt y round
#include <glad/glad.h> // Carga las funciones de OpenGL
#include <GLFW/glfw3.h> // Maneja la ventana, teclado y mouse
#include "shaderClass.h" // Clase que permite cargar y utilizar los shaders
#include "VAO.h" // Maneja el VAO, que guarda como se interpretan los vertices
#include "VBO.h" // Maneja el VBO, donde se almacenan los vertices
#include "ImGui/imgui.h"// Biblioteca principal de Dear ImGui para crear botones, ventanas y controles
#include "ImGui/imgui_impl_glfw.h"// Permite que Dear ImGui funcione con GLFW y reciba eventos del mouse y teclado
#include "ImGui/imgui_impl_opengl3.h"// Permite dibujar la interfaz de Dear ImGui utilizando OpenGL
#include <algorithm>// Permite utilizar funciones para obtener valores minimos y maximos

// Tamaño inicial de la ventana
const unsigned int width = 800;
const unsigned int height = 800;

// Cantidad de filas y columnas de la grilla
// 150x150 crea 22500 celdas que funcionan como pixeles
const int FILAS = 200;
const int COLUMNAS = 200;

// La grilla es 2D, pero sus estados se guardan en un vector de una dimension
 
// Guarda las celdas que pertenecen a una figura
struct Figura
{
    std::vector<int> celdas;

    // Indica si la figura tiene relleno
    bool tieneRelleno = false;
};
// false significa celda gris y true significa celda roja

std::vector<bool> celdasSeleccionadas(FILAS* COLUMNAS, false);

 // Guarda todas las figuras dibujadas
std::vector<Figura> figuras;

// Declara la funcion que actualiza las figuras
void actualizarFiguras();

// Guarda cual figura esta seleccionada
// -1 significa que no hay ninguna seleccionada
int figuraSeleccionada = -1;

// Guarda temporalmente las celdas de la figura actual
Figura figuraActual;

// Indica si estamos registrando una figura
bool registrandoFigura = false;

// Prepara el registro de una figura nuevaahora si ya
void iniciarFigura()
{
    // Borra las celdas guardadas anteriormente
    figuraActual.celdas.clear();

    // Activa el registro de celdas
    registrandoFigura = true;
}

// Guarda la figura cuando terminamos de dibujarla
void finalizarFigura()
{
    // Detiene el registro de celdas
    registrandoFigura = false;

    // Comprueba que la figura tenga celdas
    if (!figuraActual.celdas.empty())
    {
        // Guarda la figura en nuestra lista
        figuras.push_back(figuraActual);
    }
}


// Busca una figura que contenga la celda seleccionada
void seleccionarFigura(int fila, int columna)
{
    // Calcula la posicion de la celda
    int indice = fila * COLUMNAS + columna;

    // Inicialmente no hay ninguna figura seleccionada
    figuraSeleccionada = -1;

    // Recorre las figuras desde la ultima dibujada
    for (int i = (int)figuras.size() - 1; i >= 0; i--)
    {
        // Revisa las celdas de cada figura
        for (int celda : figuras[i].celdas)
        {
            // Comprueba si hicimos clic sobre la figura
            if (celda == indice)
            {
                // Guarda la figura seleccionada
                figuraSeleccionada = i;
                return;
            }
        }
    }
}


// Busca a que figura pertenece una celda
int buscarFigura(int fila, int columna)
{
    // Calcula el indice de la celda
    int indice = fila * COLUMNAS + columna;

    // Recorre las figuras guardadas
    for (int i = (int)figuras.size() - 1; i >= 0; i--)
    {
        // Revisa las celdas de cada figura
        for (int celda : figuras[i].celdas)
        {
            if (celda == indice)
            {
                return i;
            }
        }
    }

    // No encontro ninguna figura
    return -1;
}


// Une las celdas del relleno con una figura existente
void unirRelleno(int numeroFigura, const std::vector<int>& relleno)
{
    // Comprueba que la figura exista
    if (numeroFigura < 0 || numeroFigura >= (int)figuras.size())
        return;

    // Recorre las celdas del relleno
    for (int celda : relleno)
    {
        // Evita guardar celdas repetidas
        if (std::find(
            figuras[numeroFigura].celdas.begin(),
            figuras[numeroFigura].celdas.end(),
            celda
        ) == figuras[numeroFigura].celdas.end())
        {
            figuras[numeroFigura].celdas.push_back(celda);
        }
    }

    // Indica que la figura tiene relleno
    figuras[numeroFigura].tieneRelleno = true;
}


// Busca una figura que rodee el relleno
int encontrarContorno(const std::vector<int>& relleno)
{
    // Recorre las celdas rellenadas
    for (int celda : relleno)
    {
        int fila = celda / COLUMNAS;
        int columna = celda % COLUMNAS;

        // Revisa las cuatro celdas vecinas
        int vecinos[4][2] = {
            {fila - 1, columna},
            {fila + 1, columna},
            {fila, columna - 1},
            {fila, columna + 1}
        };

        for (int i = 0; i < 4; i++)
        {
            int f = vecinos[i][0];
            int c = vecinos[i][1];

            // Comprueba los limites de la cuadricula
            if (f >= 0 && f < FILAS && c >= 0 && c < COLUMNAS)
            {
                int figura = buscarFigura(f, c);

                if (figura != -1)
                {
                    return figura;
                }
            }
        }
    }

    return -1;
}


// Rota 90 grados la figura seleccionada
void rotarFigura()
{
    // Comprueba que exista una figura seleccionada
    if (figuraSeleccionada == -1)
        return;

    // Obtiene las celdas de la figura
    std::vector<int>& celdas = figuras[figuraSeleccionada].celdas;

    // Busca los limites de la figura
    int minX = COLUMNAS;
    int maxX = 0;
    int minY = FILAS;
    int maxY = 0;

    for (int indice : celdas)
    {
        int x = indice % COLUMNAS;
        int y = indice / COLUMNAS;

        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }

    // Calcula el centro de la figura
    float centroX = (minX + maxX) / 2.0f;
    float centroY = (minY + maxY) / 2.0f;

    // Guarda las nuevas posiciones
    std::vector<int> nuevasCeldas;

    for (int indice : celdas)
    {
        int x = indice % COLUMNAS;
        int y = indice / COLUMNAS;

        // Rota 90 grados utilizando coordenadas enteras
       // Calcula el giro de 90 grados sin utilizar decimales
        int nuevoX = (minX + maxX + minY + maxY) / 2 - y;
        int nuevoY = (minY + maxY - minX - maxX) / 2 + x;

        // Comprueba que la celda quede dentro de la grilla
        if (nuevoX >= 0 && nuevoX < COLUMNAS &&
            nuevoY >= 0 && nuevoY < FILAS)
        {
            nuevasCeldas.push_back(nuevoY * COLUMNAS + nuevoX);
        }
    }



    // Actualiza las celdas de la figura
    celdas = nuevasCeldas;

    // Actualiza la cuadricula para mostrar la rotacion
    actualizarFiguras();
}

// Traslada la figura seleccionada en los ejes X e Y
void trasladarFigura(int dx, int dy)
{
    // Comprueba que exista una figura seleccionada
    if (figuraSeleccionada == -1)
        return;

    // Evita actualizar si no hay movimiento
    if (dx == 0 && dy == 0)
        return;

    // Obtiene las celdas de la figura
    std::vector<int>& celdas = figuras[figuraSeleccionada].celdas;

    // Comprueba que toda la figura permanezca dentro de la grilla
    for (int indice : celdas)
    {
        int columna = indice % COLUMNAS;
        int fila = indice / COLUMNAS;

        int nuevaColumna = columna + dx;
        int nuevaFila = fila + dy;

        if (nuevaColumna < 0 || nuevaColumna >= COLUMNAS ||
            nuevaFila < 0 || nuevaFila >= FILAS)
        {
            return;
        }
    }

    // Desplaza todas las celdas de la figura
    for (int& indice : celdas)
    {
        int columna = indice % COLUMNAS;
        int fila = indice / COLUMNAS;

        columna += dx;
        fila += dy;

        indice = fila * COLUMNAS + columna;
    }

    // Actualiza la grilla para mostrar la nueva posicion
    actualizarFiguras();
}


// Actualiza la cuadricula con las figuras guardadas
void actualizarFiguras()
{
    // Limpia las celdas de la cuadricula
    for (int i = 0; i < FILAS * COLUMNAS; i++)
    {
        celdasSeleccionadas[i] = false;
    }

    // Recorre todas las figuras guardadas
    for (const Figura& figura : figuras)
    {
        // Vuelve a pintar las celdas de cada figura
        for (int indice : figura.celdas)
        {
            celdasSeleccionadas[indice] = true;
        }
    }
}

// Define las herramientas de dibujo disponibles en nuestro Paint
// Herramientas disponibles para dibujar en la cuadricula
// Herramientas disponibles para dibujar en la cuadricula
// Define los modos disponibles del programa
enum ModoDibujo
{
    LINEA,
    CIRCULO,
    RECTANGULO,
    TRIANGULO,
    RELLENO,
    ROTAR,
    TRASLADAR
};
// El programa empieza por defecto en modo linea
ModoDibujo modoActual = LINEA;

// Guarda el estado anterior del mouse para detectar un solo clic
// y no muchos clics mientras el boton permanezca presionado
bool clickAnterior = false;

// Indica si estamos arrastrando una figura
bool arrastrandoFigura = false;

// Guarda la posicion anterior del mouse en la grilla
int filaAnteriorArrastre = 0;
int columnaAnteriorArrastre = 0;

// Indica si ya se realizo el primer clic de la figura
bool primerPuntoSeleccionado = false;

// Guardan la posicion del primer clic
int puntoInicialFila = 0;
int puntoInicialColumna = 0;

// Cuenta cuantos vertices del triangulo hemos seleccionado
int clicsTriangulo = 0;

// Guarda la posicion del primer vertice
int trianguloX1 = 0;
int trianguloY1 = 0;

// Guarda la posicion del segundo vertice
int trianguloX2 = 0;
int trianguloY2 = 0;

// Se ejecuta cuando cambia el tamaño de la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Le dice a OpenGL que utilice todo el nuevo tamaño disponible
    glViewport(0, 0, width, height);
}

// Marca una celda para que posteriormente se dibuje roja
void pintarCelda(int fila, int columna)
{
    // Comprueba que la posicion exista dentro de la grilla
    // Esto tambien evita errores si parte de un circulo queda fuera
    if (fila >= 0 && fila < FILAS && columna >= 0 && columna < COLUMNAS)
    {
        // El vector es 1D, por eso convertimos fila y columna a un indice
        // Ejemplo: fila 2, columna 5 -> 2*150+5
        int indice = fila * COLUMNAS + columna;

        // true indica que esta celda debe mostrarse roja

        // Guarda la celda si estamos registrando una figura
        if (registrandoFigura)
        {
            figuraActual.celdas.push_back(indice);
        }

        celdasSeleccionadas[indice] = true;
    }
}


// Rellena las celdas vacias conectadas desde el punto seleccionado
void rellenarZona(int fila, int columna)
{
    // Comprueba que el punto este dentro de la cuadricula
    if (fila < 0 || fila >= FILAS ||
        columna < 0 || columna >= COLUMNAS)
    {
        return;
    }

    // Guarda las celdas pendientes de pintar
    std::vector<int> pendientes;

    // Agrega la celda seleccionada
    pendientes.push_back(fila * COLUMNAS + columna);

    // Repite hasta terminar de revisar las celdas
    while (!pendientes.empty())
    {
        // Obtiene una celda pendiente
        int posicion = pendientes.back();
        pendientes.pop_back();

        // Comprueba si ya esta pintada
        if (celdasSeleccionadas[posicion])
        {
            continue;
        }

        // Pinta la celda actual
        celdasSeleccionadas[posicion] = true;

        // Guarda la celda dentro del relleno actual
        if (registrandoFigura)
        {
            figuraActual.celdas.push_back(posicion);
        }

        // Calcula su fila y columna
        int f = posicion / COLUMNAS;
        int c = posicion % COLUMNAS;

        // Revisa la celda superior
        if (f > 0 && !celdasSeleccionadas[(f - 1) * COLUMNAS + c])
        {
            pendientes.push_back((f - 1) * COLUMNAS + c);
        }

        // Revisa la celda inferior
        if (f < FILAS - 1 && !celdasSeleccionadas[(f + 1) * COLUMNAS + c])
        {
            pendientes.push_back((f + 1) * COLUMNAS + c);
        }

        // Revisa la celda izquierda
        if (c > 0 && !celdasSeleccionadas[f * COLUMNAS + c - 1])
        {
            pendientes.push_back(f * COLUMNAS + c - 1);
        }

        // Revisa la celda derecha
        if (c < COLUMNAS - 1 && !celdasSeleccionadas[f * COLUMNAS + c + 1])
        {
            pendientes.push_back(f * COLUMNAS + c + 1);
        }
    }
}


void limpiarGrilla()
{
    // Borra las figuras guardadas
    figuras.clear();

    // Quita cualquier figura seleccionada
    figuraSeleccionada = -1;

    // Cancela el registro de una figura
    registrandoFigura = false;

    // Borra las celdas temporales
    figuraActual.celdas.clear();

    // Reinicia los clics del triangulo
    clicsTriangulo = 0;

    // Recorre las 22500 celdas
    for (int i = 0; i < FILAS * COLUMNAS; i++)
        celdasSeleccionadas[i] = false;

    // Cancela cualquier figura que hubiera quedado a medio hacer
    primerPuntoSeleccionado = false;

    std::cout << "\nGRILLA LIMPIA\n" << std::endl;
}

// LINEA 

// Algoritmo de Bresenham para lineas en cualquier direccion
void dibujarLineaBresenham(int columnaInicial, int filaInicial,
    int columnaFinal, int filaFinal)
{
    // Copiamos los puntos para poder modificarlos durante el algoritmo
    int x0 = columnaInicial;
    int y0 = filaInicial;
    int x1 = columnaFinal;
    int y1 = filaFinal;

    // Diferencia horizontal entre el inicio y el final
    // abs hace que la distancia siempre sea positiva
    int dx = std::abs(x1 - x0);

    // Indica hacia donde debe avanzar X
    // 1 = derecha, -1 = izquierda
    int sx = (x0 < x1) ? 1 : -1;

    // Diferencia vertical
    // Se utiliza negativa por la forma de esta version de Bresenham
    int dy = -std::abs(y1 - y0);

    // Indica hacia donde debe avanzar Y
    // 1 o -1 dependiendo de donde se encuentre el punto final
    int sy = (y0 < y1) ? 1 : -1;

    // Guarda el error entre la linea matematica ideal
    // y la celda que estamos seleccionando
    int error = dx + dy;

    // Continua hasta alcanzar el punto final
    while (true)
    {
        // La posicion actual pertenece a la aproximacion de la linea
        pintarCelda(y0, x0);

        // Muestra los pixeles elegidos para poder observar el algoritmo
        std::cout << "Linea -> Fila: " << y0
            << " | Columna: " << x0 << std::endl;

        // Cuando llegamos exactamente al segundo punto termina
        if (x0 == x1 && y0 == y1)
            break;

        // Duplica el error para realizar las comparaciones
        // sin necesitar divisiones ni numeros decimales
        int errorDoble = 2 * error;

        // Comprueba si es necesario avanzar horizontalmente
        if (errorDoble >= dy)
        {
            // Actualiza el error despues del movimiento en X
            error += dy;

            // Avanza derecha o izquierda dependiendo de sx
            x0 += sx;
        }

        // Comprueba si tambien es necesario avanzar verticalmente
        if (errorDoble <= dx)
        {
            // Actualiza el error despues del movimiento en Y
            error += dx;

            // Avanza en la direccion vertical indicada por sy
            y0 += sy;
        }
    }
}

// Dibuja un rectangulo utilizando cuatro lineas de Bresenham
void dibujarRectanguloBresenham(int columnaInicial, int filaInicial,
    int columnaFinal, int filaFinal)
{
    // Calcula las cuatro esquinas del rectangulo
    int izquierda = std::min(columnaInicial, columnaFinal);
    int derecha = std::max(columnaInicial, columnaFinal);
    int arriba = std::min(filaInicial, filaFinal);
    int abajo = std::max(filaInicial, filaFinal);

    // Dibuja el lado superior
    dibujarLineaBresenham(izquierda, arriba, derecha, arriba);

    // Dibuja el lado derecho
    dibujarLineaBresenham(derecha, arriba, derecha, abajo);

    // Dibuja el lado inferior
    dibujarLineaBresenham(derecha, abajo, izquierda, abajo);

    // Dibuja el lado izquierdo
    dibujarLineaBresenham(izquierda, abajo, izquierda, arriba);
}

// Dibuja un triangulo utilizando tres lineas de Bresenham
void dibujarTrianguloBresenham(
    int x1, int y1,
    int x2, int y2,
    int x3, int y3)
{
    // Dibuja el primer lado, desde el vertice 1 hasta el vertice 2
    dibujarLineaBresenham(x1, y1, x2, y2);

    // Dibuja el segundo lado, desde el vertice 2 hasta el vertice 3
    dibujarLineaBresenham(x2, y2, x3, y3);

    // Dibuja el tercer lado, desde el vertice 3 hasta el vertice 1
    // Esto permite cerrar el triangulo
    dibujarLineaBresenham(x3, y3, x1, y1);
}

// CIRCULO 

// A partir de un punto calculado, pinta sus puntos simetricos
void pintarPuntosCirculo(int xc, int yc, int x, int y)
{
    // xc,yc representan el centro
    // x,y representan el desplazamiento respecto al centro

    // Estos 4 puntos utilizan combinaciones de +x,-x,+y,-y
    pintarCelda(yc + y, xc + x);
    pintarCelda(yc + y, xc - x);
    pintarCelda(yc - y, xc + x);
    pintarCelda(yc - y, xc - x);

    // Intercambiar X y Y permite obtener los otros 4 octantes
    pintarCelda(yc + x, xc + y);
    pintarCelda(yc + x, xc - y);
    pintarCelda(yc - x, xc + y);
    pintarCelda(yc - x, xc - y);

    // Gracias a esto calculamos una parte del circulo
    // y aprovechamos su simetria para completar el resto
}

// Bresenham/Midpoint para dibujar un circulo
void dibujarCirculoBresenham(int xc, int yc, int radio)
{
    // X comienza en 0 porque iniciamos en un extremo del circulo
    int x = 0;

    // Y comienza a una distancia igual al radio desde el centro
    // Se usa negativo por la orientacion utilizada en esta implementacion
    int y = -radio;

    // Parametro de decision inicial
    // Indica que pixel se aproxima mejor al circulo matematico ideal
    int p = 1 - radio;

    std::cout << "\nCIRCULO BRESENHAM" << std::endl;
    std::cout << "Centro -> Fila: " << yc
        << " | Columna: " << xc << std::endl;
    std::cout << "Radio = " << radio
        << " | P0 = " << p << std::endl;

    // Solo necesitamos calcular un octante
    // Cuando X supera a -Y llegamos aproximadamente a los 45 grados
    while (x <= -y)
    {
        // El punto calculado se refleja hacia los 8 octantes
        pintarPuntosCirculo(xc, yc, x, -y);

        // Muestra como cambia el algoritmo
        std::cout << "x = " << x
            << " | y = " << y
            << " | P = " << p << std::endl;

        // En cada iteracion avanzamos horizontalmente
        x++;

        // Si p es negativo significa que podemos continuar
        // sin modificar Y porque ese pixel aproxima mejor la curva
        if (p < 0)
            p = p + 2 * x + 1;

        // Si p ya no es negativo debemos modificar tambien Y
        // para seguir aproximadamente la curvatura
        else
        {
            y++;

            // Actualiza el parametro para decidir el siguiente pixel
            p = p + 2 * x + 2 * y + 1;
        }
    }

    std::cout << "CIRCULO TERMINADO\n" << std::endl;
}

//  GRILLA 

// Crea todos los vertices necesarios para formar la grilla
std::vector<GLfloat> crearGrilla()
{
    // Aqui se guardaran todos los vertices
    std::vector<GLfloat> vertices;

    // OpenGL trabaja desde -1 hasta 1
    // Eso significa que tenemos un espacio total de 2 unidades
    // Dividir 2 entre las columnas da el ancho de cada celda
    float tamanoX = 2.0f / (float)COLUMNAS;

    // Lo mismo pero para la altura
    float tamanoY = 2.0f / (float)FILAS;

    // Separacion original entre las celdas
    float margen = 0.003f;
    // Recorre cada posicion de la grilla
    for (int fila = 0; fila < FILAS; fila++)
    {
        for (int columna = 0; columna < COLUMNAS; columna++)
        {
            // Calcula el lado izquierdo de la celda
            // -1 representa el extremo izquierdo de OpenGL
            float x1 = -1.0f + columna * tamanoX;

            // Calcula el lado derecho
            float x2 = x1 + tamanoX;

            // Empieza arriba en 1 y va bajando segun la fila
            float y2 = 1.0f - fila * tamanoY;

            // Calcula el limite inferior
            float y1 = y2 - tamanoY;

            // Reduce ligeramente la figura para crear separacion
            // Utiliza todo el espacio de cada celda
            // Deja una pequeña separacion entre las celdas
            float dibujoX1 = x1 + margen;
            float dibujoX2 = x2 - margen;
            float dibujoY1 = y1 + margen;
            float dibujoY2 = y2 - margen;

            // OpenGL dibuja triangulos, por eso cada cuadrado
            // se divide en dos triangulos de 3 vertices cada uno

            // Primer triangulo
            vertices.push_back(dibujoX1);
            vertices.push_back(dibujoY1);
            vertices.push_back(0.0f);

            vertices.push_back(dibujoX2);
            vertices.push_back(dibujoY1);
            vertices.push_back(0.0f);

            vertices.push_back(dibujoX2);
            vertices.push_back(dibujoY2);
            vertices.push_back(0.0f);

            // Segundo triangulo
            vertices.push_back(dibujoX1);
            vertices.push_back(dibujoY1);
            vertices.push_back(0.0f);

            vertices.push_back(dibujoX2);
            vertices.push_back(dibujoY2);
            vertices.push_back(0.0f);

            vertices.push_back(dibujoX1);
            vertices.push_back(dibujoY2);
            vertices.push_back(0.0f);
        }
    }

    // Devuelve todos los vertices creados
    return vertices;
}

int main()
{
    //  INICIAR OPENGL 

    // GLFW se encarga principalmente de crear la ventana
    // y recibir entradas del teclado y mouse
    if (!glfwInit())
    {
        std::cout << "ERROR: No se pudo iniciar GLFW." << std::endl;
        return -1;
    }

    // Indica que utilizaremos OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Utilizamos el perfil moderno Core de OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Crea una ventana de 800x800
    GLFWwindow* window = glfwCreateWindow(
        width, height, "Bresenham - Linea y Circulo", NULL, NULL
    );

    // Comprueba que la ventana se haya creado correctamente
    if (window == NULL)
    {
        std::cout << "ERROR: No se pudo crear la ventana." << std::endl;
        glfwTerminate();
        return -1;
    }

    // Hace que esta ventana sea donde OpenGL realizara los dibujos
    glfwMakeContextCurrent(window);

    // GLAD obtiene las funciones de OpenGL que utilizaremos
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR: No se pudo iniciar GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Indica que nuestra funcion debe ejecutarse al redimensionar
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Obtiene el tamaño real del area donde OpenGL dibuja
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

    // Define inicialmente el area completa de dibujo
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    // Comprueba que la version de Dear ImGui sea correcta
    IMGUI_CHECKVERSION();

    // Crea el contexto necesario para utilizar Dear ImGui
    ImGui::CreateContext();

    // Establece un estilo oscuro para la interfaz
    ImGui::StyleColorsDark();

    // Conecta Dear ImGui con nuestra ventana de GLFW
    // true permite que ImGui reciba eventos del mouse y teclado
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    // Prepara Dear ImGui para dibujar utilizando OpenGL 3.3
    ImGui_ImplOpenGL3_Init("#version 330");

    // ==================== SHADERS ====================

    // Carga el Vertex Shader y Fragment Shader
    Shader shaderProgram("default.vert", "default.frag");

    // Activa este programa de shaders
    shaderProgram.Activate();

    //VERTICES 

    // Ejecuta crearGrilla y recibe todos los vertices
    std::vector<GLfloat> vertices = crearGrilla();

    // El VAO guarda la configuracion que explica
    // como deben interpretarse los vertices
    VAO VAO1;
    VAO1.Bind();

    // El VBO envia y almacena los vertices en OpenGL
    VBO VBO1(vertices.data(), vertices.size() * sizeof(GLfloat));

    // Cada vertice contiene 3 valores: X, Y y Z
    // El shader recibe esos datos en la posicion 0
    VAO1.LinkAttrib(
        VBO1,
        0,
        3,
        GL_FLOAT,
        3 * sizeof(GLfloat),
        (void*)0
    );

    // Se desactivan para evitar modificarlos accidentalmente
    VAO1.Unbind();
    VBO1.Unbind();

    // Busca dentro del Fragment Shader la variable uniform "color"
    // Esta variable nos permite cambiar el color de cada celda
    GLint colorLocation =
        glGetUniformLocation(shaderProgram.ID, "color");

    // Define el color con el que se limpia el fondo
    // Los valores RGB bajos producen un gris casi negro
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // Muestra los controles
    std::cout << "\n==============================" << std::endl;
    std::cout << "      BRESENHAM OPENGL" << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << "L = Linea" << std::endl;
    std::cout << "C = Circulo" << std::endl;
    std::cout << "R = Limpiar" << std::endl;
    std::cout << "Modo actual: LINEA\n" << std::endl;

    // Guardan el estado anterior de cada tecla
    // Esto evita ejecutar una accion muchas veces
    // mientras una tecla permanece presionada
    bool teclaLAnterior = false;
    bool teclaCAnterior = false;
    bool teclaRAnterior = false;

    //  BUCLE PRINCIPAL 

    // Se ejecuta continuamente mientras la ventana siga abierta
    while (!glfwWindowShouldClose(window))
    {
        // Procesa eventos de teclado, mouse y ventana
        glfwPollEvents();

        // Prepara un nuevo fotograma de la interfaz de ImGui
        ImGui_ImplOpenGL3_NewFrame();

        // Actualiza la informacion del mouse, teclado y ventana
        ImGui_ImplGlfw_NewFrame();

        // Inicia un nuevo fotograma para crear los controles de nuestro Paint
        ImGui::NewFrame();

        // Borra visualmente el frame anterior
        // Las figuras no desaparecen porque su estado sigue guardado
        // en celdasSeleccionadas
        glClear(GL_COLOR_BUFFER_BIT);

        // Activa el shader antes de dibujar
        shaderProgram.Activate();

        // Comprueba si las teclas estan presionadas actualmente
        bool teclaL =
            glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;

        bool teclaC =
            glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;

        bool teclaR =
            glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;

        //  TECLA L

        // Solo cambia una vez cuando se acaba de presionar L
        if (teclaL && !teclaLAnterior)
        {
            // Selecciona el modo linea
            modoActual = LINEA;

            // Cancela un primer punto anterior si existia
            primerPuntoSeleccionado = false;

            std::cout << "\nMODO LINEA" << std::endl;
            std::cout << "Click 1 = Inicio | Click 2 = Final\n"
                << std::endl;
        }

        //  TECLA C

        // Detecta una nueva pulsacion de C
        if (teclaC && !teclaCAnterior)
        {
            // Cambia al modo circulo
            modoActual = CIRCULO;

            // Evita utilizar un punto guardado de otro modo
            primerPuntoSeleccionado = false;

            std::cout << "\nMODO CIRCULO" << std::endl;
            std::cout << "Click 1 = Centro | Click 2 = Radio\n"
                << std::endl;
        }

        // TECLA R 

        // Detecta una nueva pulsacion de R
        if (teclaR && !teclaRAnterior)
            limpiarGrilla();

        // El estado actual pasa a ser el anterior
        // para poder detectar cambios en el siguiente frame
        teclaLAnterior = teclaL;
        teclaCAnterior = teclaC;
        teclaRAnterior = teclaR;

        // PANEL DE HERRAMIENTAS DE PAINT

// Crea una ventana de ImGui con el titulo "Herramientas"
        ImGui::Begin("Modos");

        // Muestra un texto que indica la herramienta seleccionada
        // Guarda el nombre de la herramienta seleccionada
        const char* nombreModo = "Linea";

        // Comprueba si estamos utilizando la herramienta circulo
        if (modoActual == CIRCULO)
        {
            nombreModo = "Circulo";
        }

        // Comprueba si estamos utilizando la herramienta rectangulo
        else if (modoActual == RECTANGULO)
        {
            nombreModo = "Rectangulo";
        }

        // Comprueba si la herramienta seleccionada es triangulo
        else if (modoActual == TRIANGULO)
        {
            // Actualiza el nombre que aparece en el panel
            nombreModo = "Triangulo";
        }

        // Muestra en el panel el nombre de la herramienta activa
        ImGui::Text("Modo actual: %s", nombreModo);

        // Crea un boton para seleccionar el modo linea
        if (ImGui::Button("Linea"))
        {
            // Activa el algoritmo de Bresenham para lineas
            modoActual = LINEA;

            // Cancela cualquier figura que estuviera a medio dibujar
            primerPuntoSeleccionado = false;
        }

        // Crea un boton para seleccionar el modo circulo
        if (ImGui::Button("Circulo"))
        {
            // Activa el algoritmo de Bresenham para circulos
            modoActual = CIRCULO;

            // Cancela cualquier figura que estuviera a medio dibujar
            primerPuntoSeleccionado = false;
        }

        // Crea un boton para seleccionar la herramienta rectangulo
        if (ImGui::Button("Rectangulo"))
        {
            // Cambia el modo de dibujo a rectangulo
            modoActual = RECTANGULO;

            // Cancela cualquier figura que estuviera a medio dibujar
            primerPuntoSeleccionado = false;

            // Muestra en la consola la herramienta seleccionada
            std::cout << "\nMODO RECTANGULO" << std::endl;
            std::cout << "Click 1 = Primera esquina | Click 2 = Esquina opuesta\n";
        }

        // Crea un boton para seleccionar la herramienta triangulo
        if (ImGui::Button("Triangulo"))
        {
            // Cambia el modo de dibujo a triangulo
            modoActual = TRIANGULO;

            // Cancela cualquier figura que estuviera a medio dibujar
            primerPuntoSeleccionado = false;

            // Muestra en la consola que seleccionamos esta herramienta
            std::cout << "\nMODO TRIANGULO" << std::endl;

            // Explica cuantos clics necesitaremos para dibujar
            std::cout << "Selecciona tres vertices con el mouse\n";
        }


        // Crea un boton para activar la rotacion
        if (ImGui::Button("Rotar"))
        {
            // Activa la herramienta de rotacion
            modoActual = ROTAR;

            // Cancela cualquier dibujo incompleto
            primerPuntoSeleccionado = false;

            // Reinicia los clics del triangulo
            clicsTriangulo = 0;

            // Muestra un mensaje en la consola
            std::cout << "MODO ROTAR" << std::endl;
        }

        // Boton para activar la traslacion de figuras
        if (ImGui::Button("Trasladar"))
        {
            // Activa el modo de traslacion
            modoActual = TRASLADAR;

            // Cancela cualquier dibujo incompleto
            primerPuntoSeleccionado = false;
            clicsTriangulo = 0;

            // Quita la seleccion anterior
            figuraSeleccionada = -1;

            // Muestra el modo seleccionado
            std::cout << "MODO TRASLADAR" << std::endl;
        }



        // Crea el boton para activar la herramienta de relleno
        if (ImGui::Button("Rellenar"))
        {
            // Selecciona el modo de relleno
            modoActual = RELLENO;

            // Reinicia la seleccion de puntos anteriores
            primerPuntoSeleccionado = false;

            // Muestra en consola la herramienta seleccionada
            std::cout << "\nMODO RELLENO" << std::endl;
            std::cout << "Haz clic dentro de una figura cerrada\n";
        }

        // Crea un boton para borrar todas las figuras de la grilla
        if (ImGui::Button("Limpiar"))
        {
            // Utiliza la funcion que ya existe en nuestro programa
            limpiarGrilla();
        }

        // Finaliza la construccion de la ventana de herramientas
        ImGui::End();

        //  MOUSE 

        // Comprueba si el boton izquierdo esta presionado
        bool clickActual =
            glfwGetMouseButton(
                window,
                GLFW_MOUSE_BUTTON_LEFT
            ) == GLFW_PRESS;

       // Solo entra cuando pasa de no presionado a presionado
       // Esto evita registrar muchos clics por mantener el boton
       // Detecta un clic nuevo del mouse y comprueba que no sea sobre ImGui
       // Asi, al presionar un boton de la interfaz, no dibujamos en la grilla
        if (clickActual && !clickAnterior && !ImGui::GetIO().WantCaptureMouse)
        {
            // mouseX y mouseY guardaran la posicion del cursor
            double mouseX, mouseY;

            // Guardan el tamaño actual de la ventana
            int ventanaWidth, ventanaHeight;

            // Obtiene la posicion actual del mouse en pixeles
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // Obtiene el tamaño actual de la ventana
            // Esto permite que los clics funcionen incluso al redimensionar
            glfwGetWindowSize(
                window,
                &ventanaWidth,
                &ventanaHeight
            );

            // Evita dividir entre cero
            if (ventanaWidth > 0 && ventanaHeight > 0)
            {

                // mouseX/ventanaWidth produce un valor entre 0 y 1
                // Multiplicarlo por COLUMNAS indica la columna seleccionada
                int columna = (int)std::floor(
                    (mouseX / (double)ventanaWidth) * COLUMNAS
                );

                // Hace lo mismo con la posicion vertical para obtener la fila
                int fila = (int)std::floor(
                    (mouseY / (double)ventanaHeight) * FILAS
                );

                // Verifica que el clic pertenezca a una celda valida
                if (fila >= 0 && fila < FILAS &&
                    columna >= 0 && columna < COLUMNAS)
                {
                    //  PRIMER CLIC

                    // Si aun no tenemos un primer punto, lo guardamos

                    // Comprueba si estamos dibujando un triangulo

                    // Comprueba si seleccionamos la herramienta de relleno

                    // Comprueba si queremos seleccionar una figura

                    // Selecciona la figura que vamos a trasladar
                    if (modoActual == TRASLADAR)
                    {
                        // Busca la figura donde hicimos clic
                        seleccionarFigura(fila, columna);

                        // Comprueba que encontramos una figura
                        if (figuraSeleccionada != -1)
                        {
                            // Activa el arrastre
                            arrastrandoFigura = true;

                            // Guarda la posicion inicial del mouse
                            filaAnteriorArrastre = fila;
                            columnaAnteriorArrastre = columna;
                        }
                    }

                    else if (modoActual == ROTAR)
                    {
                        // Busca la figura sobre la que hicimos clic
                        seleccionarFigura(fila, columna);

                        // Comprueba que encontramos una figura
                        if (figuraSeleccionada != -1)
                        {
                            // Gira solamente esa figura 90 grados
                            rotarFigura();

                            // Muestra un mensaje en la consola
                            std::cout << "Figura rotada: "
                                << figuraSeleccionada << std::endl;
                        }
                        else
                        {
                            // Avisa si no seleccionamos ninguna figura
                            std::cout << "No se encontro ninguna figura"
                                << std::endl;
                        }
                    }


                    else if (modoActual == RELLENO)
                    {
                        // Comienza a registrar el relleno
                        iniciarFigura();

                        // Rellena la zona seleccionada
                        rellenarZona(fila, columna);

                        // Busca la figura que rodea el relleno
                        int contorno = encontrarContorno(figuraActual.celdas);

                        if (contorno != -1)
                        {
                            // Une el relleno con el contorno
                            unirRelleno(contorno, figuraActual.celdas);

                            // Termina el registro sin crear otra figura
                            registrandoFigura = false;
                        }
                        else
                        {
                            // Guarda el relleno como figura independiente
                            finalizarFigura();
                        }
                    }


                    else if (modoActual == TRIANGULO)
                    {
                        // PRIMER CLIC: guarda el primer vertice
                        if (clicsTriangulo == 0)
                        {
                            // Comienza a registrar el triangulo
                            iniciarFigura();
                            trianguloX1 = columna;
                            trianguloY1 = fila;

                            // Pinta el primer vertice
                            pintarCelda(fila, columna);

                            clicsTriangulo = 1;
                        }

                        // SEGUNDO CLIC: guarda el segundo vertice
                        else if (clicsTriangulo == 1)
                        {
                            trianguloX2 = columna;
                            trianguloY2 = fila;

                            // Pinta el segundo vertice
                            pintarCelda(fila, columna);

                            clicsTriangulo = 2;
                        }

                        // TERCER CLIC: dibuja el triangulo completo
                        else
                        {
                            // Une los tres vertices usando Bresenham
                            dibujarTrianguloBresenham(
                                trianguloX1, trianguloY1,
                                trianguloX2, trianguloY2,
                                columna, fila
                            );

                            // Guarda el triangulo como figura independiente
                            finalizarFigura();

                            // Reinicia el contador para dibujar otro triangulo
                            clicsTriangulo = 0;
                        }
                    }

                    else if (!primerPuntoSeleccionado)
                    {
                        puntoInicialFila = fila;
                        puntoInicialColumna = columna;

                        // Indica que ahora estamos esperando el segundo clic
                        primerPuntoSeleccionado = true;

                        // En modo linea el primer punto es el inicio
                        if (modoActual == LINEA)
                        {
                            // Marca el primer punto de la linea
                            pintarCelda(fila, columna);
                        }

                        // En modo circulo el primer punto es el centro
                        else
                        {
                            std::cout << "\nCENTRO -> Fila: "
                                << fila
                                << " | Columna: "
                                << columna << std::endl;
                        }
                    }

                    // SEGUNDO CLIC 

                    // Si ya habia un primer punto, este es el segundo
                    else
                    {
                        // Guarda la posicion del segundo clic
                        int puntoFinalFila = fila;
                        int puntoFinalColumna = columna;

                        //  DIBUJAR LINEA 

                        if (modoActual == LINEA)
                        {
                            std::cout << "FINAL -> Fila: "
                                << puntoFinalFila
                                << " | Columna: "
                                << puntoFinalColumna
                                << std::endl;

                            // Envia P1 y P2 al algoritmo de Bresenham

                            // Comienza a guardar las celdas de esta linea
                            iniciarFigura();

                            dibujarLineaBresenham(
                                puntoInicialColumna,
                                puntoInicialFila,
                                puntoFinalColumna,
                                puntoFinalFila
                            );

                            // Guarda la linea como una figura independiente
                            finalizarFigura();

                            std::cout << "LINEA TERMINADA\n"
                                << std::endl;
                        }

                        // DIBUJAR CIRCULO

                        // Comprueba que la herramienta seleccionada sea circulo
                        else if (modoActual == CIRCULO)
                        {
                            // Calcula cuanto se separa el segundo clic
                            // del centro horizontalmente
                            int diferenciaX =
                                puntoFinalColumna -
                                puntoInicialColumna;

                            // Calcula la diferencia vertical
                            int diferenciaY =
                                puntoFinalFila -
                                puntoInicialFila;

                            // Teorema de Pitagoras:
                            // distancia = sqrt(dx^2 + dy^2)
                            // Esa distancia sera utilizada como radio
                            int radio = (int)std::round(
                                std::sqrt(
                                    diferenciaX * diferenciaX +
                                    diferenciaY * diferenciaY
                                )
                            );

                            std::cout << "RADIO = "
                                << radio << std::endl;

                            // Un radio debe ser mayor que cero

                            if (radio > 0)
                            {
                                // Comienza a registrar el circulo
                                iniciarFigura();

                                // Envia centro y radio a Bresenham
                                dibujarCirculoBresenham(
                                    puntoInicialColumna,
                                    puntoInicialFila,
                                    radio
                                );

                                // Guarda el circulo como figura independiente
                                finalizarFigura();
                            }

                            else
                            {
                                std::cout
                                    << "ERROR: Radio invalido."
                                    << std::endl;
                            }
                        }

                        // DIBUJAR RECTANGULO
                       // Comprueba si seleccionamos la herramienta rectangulo
                        else if (modoActual == RECTANGULO)
                        {
                            // Dibuja los cuatro lados utilizando Bresenham
                            // El primer clic indica una esquina
                            // El segundo clic indica la esquina opuesta
                            // Comienza a registrar el rectangulo
                            iniciarFigura();
                            dibujarRectanguloBresenham(
                                puntoInicialColumna,
                                puntoInicialFila,
                                puntoFinalColumna,
                                puntoFinalFila
                            );

                            // Guarda el rectangulo como figura independiente
                            finalizarFigura();

                            // Confirma en la consola que terminamos el dibujo
                            std::cout << "RECTANGULO TERMINADO\n" << std::endl;
                        }

                        // La figura ya termino
                        // El siguiente clic sera un nuevo primer punto
                        primerPuntoSeleccionado = false;
                    }
                }
            }
        }

        // Guarda el estado actual para detectar
        // un nuevo clic en el siguiente frame
        clickAnterior = clickActual;

        // Controla el movimiento de las figuras con el mouse
        if (modoActual == TRASLADAR)
        {
            // Detiene el arrastre cuando soltamos el mouse
            if (!clickActual)
            {
                arrastrandoFigura = false;
            }
            // Mueve la figura mientras mantenemos presionado el mouse
            else if (arrastrandoFigura)
            {
                double mouseX, mouseY;
                int ventanaWidth, ventanaHeight;

                // Obtiene la posicion actual del mouse
                glfwGetCursorPos(window, &mouseX, &mouseY);
                glfwGetWindowSize(window, &ventanaWidth, &ventanaHeight);

                if (ventanaWidth > 0 && ventanaHeight > 0)
                {
                    // Convierte la posicion del mouse a celdas
                    int columna = (int)std::floor(
                        (mouseX / ventanaWidth) * COLUMNAS
                    );

                    int fila = (int)std::floor(
                        (mouseY / ventanaHeight) * FILAS
                    );

                    // Comprueba que el mouse este dentro de la grilla
                    if (fila >= 0 && fila < FILAS &&
                        columna >= 0 && columna < COLUMNAS)
                    {
                        // Calcula el desplazamiento en X e Y
                        int dx = columna - columnaAnteriorArrastre;
                        int dy = fila - filaAnteriorArrastre;

                        // Mueve la figura
                        trasladarFigura(dx, dy);

                        // Guarda la nueva posicion del mouse
                        columnaAnteriorArrastre = columna;
                        filaAnteriorArrastre = fila;
                    }
                }
            }
        }

        //  RENDERIZADO 

        // Activa el VAO que contiene la configuracion de la grilla
        VAO1.Bind();

        // Recorre todas las celdas para dibujarlas

        // Dibuja los triangulos completamente rellenos
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        for (int fila = 0; fila < FILAS; fila++)
        {
            for (int columna = 0;
                columna < COLUMNAS;
                columna++)
            {
                // Convierte la posicion 2D a indice del vector
                int indice =
                    fila * COLUMNAS + columna;

                // Si Bresenham selecciono esta celda, se pinta roja
                if (celdasSeleccionadas[indice])
                {
                    // RGBA = rojo
                    glUniform4f(
                        colorLocation,
                        1.0f, 0.0f, 0.0f, 1.0f
                    );
                }

                // Las celdas no seleccionadas permanecen grises
                else
                {
                    glUniform4f(
                        colorLocation,
                        0.25f, 0.25f, 0.25f, 1.0f
                    );
                }

                // Cada celda utiliza 6 vertices
                // porque esta formada por dos triangulos
                // indice*6 indica donde empiezan sus vertices
                glDrawArrays(
                    GL_TRIANGLES,
                    indice * 6,
                    6
                );
            }
        }

        // OpenGL dibuja primero en un buffer oculto
        // Esta instruccion intercambia buffers y muestra el frame terminado

        // Finaliza la construccion de la interfaz de este fotograma
        ImGui::Render();

        // Dibuja en pantalla las ventanas y botones de Dear ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
    }

    // CERRAR PROGRAMA 

    // Libera la configuracion de vertices

    // Cierra la conexion entre Dear ImGui y OpenGL
    ImGui_ImplOpenGL3_Shutdown();

    // Cierra la conexion entre Dear ImGui y GLFW
    ImGui_ImplGlfw_Shutdown();

    // Libera la memoria utilizada por la interfaz de Dear ImGui
    ImGui::DestroyContext();

    VAO1.Delete();

    // Libera la memoria donde estaban almacenados los vertices
    VBO1.Delete();

    // Elimina el programa de shaders
    shaderProgram.Delete();

    // Destruye la ventana
    glfwDestroyWindow(window);

    // Finaliza GLFW
    glfwTerminate();

    // 0 indica que el programa termino correctamente
    return 0;
}