# Paint con OpenGL y algoritmos de Bresenham

## Información del proyecto

**Nombre del estudiante:Danniel Alejandro Rico Pardo 
**Código estudiantil:6000838
**Asignatura:** Introducción a la Computación Gráfica  
**Lenguaje:** C++  
**Entorno de desarrollo:** Visual Studio  
**Repositorio:** [Intro-a-la-computacion-grafica](https://github.com/Me3452FF/Intro-a-la-computacion-grafica)

## Descripción

Este proyecto consiste en una aplicación de dibujo tipo Paint desarrollada en C++ con OpenGL. Su objetivo es representar figuras geométricas sobre una cuadrícula y aplicar algoritmos de computación gráfica para determinar qué celdas deben colorearse.

La aplicación dispone de una cuadrícula de **200 × 200 celdas** y permite dibujar líneas, círculos, rectángulos y triángulos. También incorpora relleno de regiones, rotación individual de figuras en pasos de 90 grados y una opción para limpiar la cuadrícula.

## Arquitectura del proyecto

El proyecto está organizado en componentes que separan la interacción del usuario, el procesamiento de figuras, el estado del dibujo y su representación gráfica.



### 1. Entrada e interfaz

- **GLFW:** crea la ventana y recibe los eventos del teclado y del mouse.
- **Dear ImGui:** genera el panel de modos para escoger línea, círculo, rectángulo, triángulo, relleno, rotación y limpieza.
- **Main.cpp:** interpreta los clics, transforma las posiciones del cursor en coordenadas de la cuadrícula y ejecuta la operación seleccionada.

### 2. Algoritmos de procesamiento

- **Líneas:** algoritmo de Bresenham con una variable de error para decidir las celdas del recorrido.
- **Rectángulos:** construcción de sus cuatro lados mediante líneas de Bresenham.
- **Triángulos:** unión de tres vértices mediante tres líneas de Bresenham.
- **Círculos:** algoritmo incremental que utiliza un parámetro de decisión y simetría en ocho octantes.
- **Relleno:** recorrido iterativo de celdas vecinas (*flood fill*).
- **Rotación:** transformación de coordenadas de las celdas de la figura seleccionada para girarla 90 grados.

### 3. Almacenamiento de información

El estado visual de cada celda se guarda en `celdasSeleccionadas`, un `std::vector<bool>`. El valor `true` indica que debe aparecer pintada y `false` que permanece sin seleccionar.

Las figuras se representan mediante la estructura `Figura`, que almacena las posiciones de sus celdas y si tienen relleno. El vector `figuras` permite conservar varios dibujos y seleccionar uno para aplicar una rotación individual.

### 4. Representación gráfica

- **OpenGL y GLAD:** permiten dibujar la cuadrícula y acceder a las funciones gráficas.
- **crearGrilla():** genera la geometría de las celdas; cada una se representa mediante dos triángulos (seis vértices).
- **VBO:** almacena los datos de los vértices.
- **VAO:** configura cómo se interpretan esos vértices.
- **Shaders (`default.vert` y `default.frag`):** intervienen en la transformación y el color de los elementos dibujados.
- **Ciclo principal:** actualiza la entrada, procesa el dibujo y representa las celdas de color rojo o gris.

## Archivos y carpetas principales

| Archivo o carpeta | Función |
|---|---|
| `Main.cpp` | Lógica de interacción, algoritmos, figuras y ciclo principal. |
| `VAO.h` / `VAO.cpp` | Configuración del objeto de vértices (VAO). |
| `VBO.h` / `VBO.cpp` | Gestión del búfer de vértices (VBO). |
| `EBO.h` / `EBO.cpp` | Clase auxiliar de búfer de índices presente en el proyecto. |
| `shaderClass.h` y archivos asociados | Gestión de shaders. |
| `default.vert` / `default.frag` | Vertex Shader y Fragment Shader. |
| `glad.c` | Implementación del cargador GLAD. |
| `ImGui/` | Archivos de Dear ImGui para la interfaz. |
| `Libraries/` | Dependencias y archivos de apoyo del proyecto. |
| Archivos `.sln` y `.vcxproj` | Solución y configuración de Visual Studio. |

## Uso básico

1. Abrir la solución del proyecto en Visual Studio.
2. Compilar y ejecutar la aplicación con las dependencias configuradas.
3. Elegir un modo desde el panel de la interfaz.
4. Seleccionar los puntos necesarios sobre la cuadrícula: dos para línea, círculo o rectángulo, y tres para triángulo.
5. Utilizar **Rellenar** dentro de una región cerrada, **Rotar** sobre una figura o **Limpiar** para reiniciar el dibujo.

## Consideraciones

La rotación está diseñada para operar sobre figuras seleccionadas dentro de los límites de la cuadrícula. Si al rotar una figura parte de sus celdas queda fuera del área disponible, esas posiciones no se conservan. La visualización puede cambiar de proporción si la ventana se maximiza en una pantalla panorámica.
