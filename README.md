# Visualización de Arrays en C++ con Realidad Aumentada

## Objetivo

Este proyecto tiene como objetivo desarrollar un sistema de visualización de estructuras de datos en C++ utilizando realidad aumentada con marcadores Aruco y OpenCV. El sistema permite a los estudiantes interactuar de manera tangible con arrays, mejorando su comprensión de las estructuras de datos.

## Funcionamiento

El sistema utiliza marcadores Aruco para representar diferentes comandos de manipulación de arrays. Estos comandos son capturados por una cámara y procesados en tiempo real para generar una visualización en realidad aumentada del array. Las operaciones soportadas incluyen la creación de arrays, inserción y eliminación de elementos. Los pasos principales del sistema son:

1. **Detección y reconocimiento de marcadores Aruco**: Utilizando OpenCV, los marcadores son detectados y sus identificadores son reconocidos.
2. **Conversión a cadena de texto y análisis léxico**: Los identificadores de los marcadores se convierten en comandos de texto, que son descompuestos en tokens para su análisis.
3. **Análisis semántico y generación de código objeto**: Los tokens son validados semánticamente y se genera un código objeto que representa las operaciones a realizar en el array.
4. **Visualización en realidad aumentada**: El código objeto se utiliza para visualizar y manipular el array en un entorno de realidad aumentada.

## Ejecución

### Configuración de Docker

1. Instala [Docker](https://www.docker.com/)
2. Abre una terminal y ejecuta los siguientes comandos:
    ```sh
    cd <cualquier directorio que quieras usar>
    git clone 
    cd ar_compiler

    # Habilita xhost - requerido para GUI
    xhost +

    # Construye la imagen de Docker
    bash docker_build.sh

    # Inicia un contenedor
    bash docker_start.sh

    # Después de iniciar el contenedor, deberías estar dentro de Docker.
    # Ejecuta el script de instalación de OpenCV (solo necesario la primera vez).
    # Esto construye OpenCV y lo instala. El proceso puede tomar un tiempo considerable
    # dependiendo de tu computadora.
    cd aruco-markers
    bash docker_opencv_setup.sh

    # Después de ejecutar el script de instalación de OpenCV, para cualquier ejecución posterior,
    # solo necesitas ejecutar el script de inicio del contenedor bash docker_start.sh.
    ```

### Dibujar un Cubo

Para estimar la pose y dibujar un cubo sobre el marcador Aruco, ejecuta el siguiente código:
```sh
cd draw_cube

mkdir build && cd build
cmake ../
make

./draw_cube -l=<longitud del marcador>


