#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <chrono>
#include <iomanip>

using namespace std;

/* ================================================================
   Este programa ejecuta la versión en C++ del kernel inspirado
   en STREAM TRIAD para usarla como referencia y poder compararla
   posteriormente con una versión en ensamblador x86.

   La idea es medir el tiempo de ejecución de un cálculo intensivo
   sobre vectores grandes y obtener además un checksum que permita
   verificar que el resultado es correcto.

   De esta forma, cuando se implemente la versión en ASM, se podrá
   comprobar que:
   - produce exactamente el mismo resultado
   - y comparar directamente los tiempos de ejecución

   ================================================================ */


void bench_cpp(int* a, int* b, int* c,int n,int iteraciones,int escalar,uint32_t& checksum);
void inicializar(int* a, int* b, int* c, int n);
void bench_asm(int* a, int* b, int* c, int n, int iteraciones, int escalar, uint32_t& checksum);
void bench_sse(int* a, int* b, int* c, int n, int iteraciones, int escalar, uint32_t& checksum);

int main()
{
    /* Tamaño del problema: 2^20 elementos (~1 millón).
       Este tamaño es lo bastante grande como para generar carga
       real sobre CPU y memoria, pero sin consumir demasiada RAM. */
    const int n = 1 << 20;

    int iteraciones = 300;                      // Número de veces que se repite el kernel
    const int escalar = 7;                      // Constante usada en el cálculo
    const int repeticiones = 20;                // Número de veces que llamaremos a las funciones del benchmark tanto en C++ como en c_86
    vector<double> tiempo_c(repeticiones);      // Vector donde se almacenara los tiempos de ejecucion del benchmark en C++
    vector<double> tiempo_asm(repeticiones);    // Vector donde se almacenara los tiempos de ejecucion del benchmark en asm x_86
    vector<double> tiempo_sse(repeticiones);    // Vector donde se almacenara los tiempos de ejecucion del benchmark en SSE
    // Se reservan tres vectores dinámicos de tamaño n.
    vector<int> a(n);
    vector<int> b(n);
    vector<int> c(n);



    vector<uint32_t> csc(20);       // Lista para guardar los 20 resultados (checksums) de la versión C
    vector<uint32_t> cs_asm(20);    // Lista para guardar los 20 resultados (checksums) de la versión x_86
    vector<uint32_t> cs_sse(20);    // Lista para guardar los 20 resultados (checksums) de la version SSE
    uint32_t  checksum_c = 0;       // Variable para el resultado de la suma de control en un entero de 32 bits sin signo en la version en C++
    uint32_t  checksum_asm = 0;     // Variable para el resultado de la suma de control en un entero de 32 bits sin signo en la version en x_86
    uint32_t checksum_sse = 0;      // Variable para el resultado de la suma de control en un entero de 32 bits sin signo en la version SSE


    // Llamada al benchmark en C/C++ y lo ejecuta 20 veces

    for (int i = 0; i < repeticiones; i++)
    {
        inicializar(a.data(), b.data(), c.data(), n);   /// Restablece los datos de entrada para garantizar que cada prueba parta de condiciones idénticas

        auto t0 = chrono::high_resolution_clock::now(); // // Captura el instante de tiempo justo antes de iniciar
        bench_cpp(a.data(), b.data(), c.data(), n, iteraciones, escalar, checksum_c);   //// Ejecución de la función de benchmark en C++ (cálculo principal)
        auto t1 = chrono::high_resolution_clock::now(); // Captura el instante de tiempo inmediatamente después de finalizar

        chrono::duration<double> dt = t1 - t0;  // Calcula el tiempo transcurrido (diferencia) en segundos

        // Almacena el tiempo de la repetición actual y el checksum para validación posterior
        tiempo_c[i] = dt.count();   
        csc[i] = checksum_c;
    }

    // Llamada al benchmark en x_86 y lo ejecuta 20 veces

    for (int i = 0; i < repeticiones; i++)
    {
        inicializar(a.data(), b.data(), c.data(), n);

        auto t0 = chrono::high_resolution_clock::now();
        bench_asm(a.data(), b.data(), c.data(), n, iteraciones, escalar, checksum_asm);
        auto t1 = chrono::high_resolution_clock::now();

        chrono::duration<double> dt = t1 - t0;

        tiempo_asm[i] = dt.count();   
        cs_asm[i] = checksum_asm;
    }

    // Llamada al benchmark en SSE y lo ejecuta 20 veces

    for (int i = 0; i < repeticiones; i++)
    {
        inicializar(a.data(), b.data(), c.data(), n);

        auto t0 = chrono::high_resolution_clock::now();
        bench_sse(a.data(), b.data(), c.data(), n, iteraciones, escalar, checksum_sse);
        auto t1 = chrono::high_resolution_clock::now();

        chrono::duration<double> dt = t1 - t0;

        tiempo_sse[i] = dt.count();
        cs_sse[i] = checksum_sse;
    }

    // TABLA DE RESULTADOS Y SPEEDUP 
// Imprime la cabecera y fija 4 decimales para que todo se vea limpio.
// El "if" es solo para que el número 10 no mueva las columnas de sitio.
// En la última columna calculamos cuántas veces es más rápido ASM que C.
    
    cout << "Iteracion        Tiempo_C        Tiempo_x86      Tiempo_SSE        Checksum_C        Checksum_x86      Checksum_SSE        Speedup_x86     Speedup_SSE" << endl;
    cout << "------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
                            
    cout << fixed << setprecision(4);

    for (int i = 0; i < repeticiones; i++)
    {
        if (i < 9)
        {
            cout << "     " << i + 1
                << "           " << tiempo_c[i] << "(s)"
                << "       " << tiempo_asm[i] << "(s)"
                << "       " << tiempo_sse[i] << "(s)"
                << "         " << csc[i]
                << "        " << cs_asm[i]
                << "        " << cs_sse[i]
                << "          " << tiempo_c[i] / tiempo_asm[i]
                << "          " << tiempo_c[i] / tiempo_sse[i]
                << endl;
        }
        else
        {
            cout << "     " << i + 1
                << "          " << tiempo_c[i] << "(s)"
                << "       " << tiempo_asm[i] << "(s)"
                << "       " << tiempo_sse[i] << "(s)"
                << "         " << csc[i]
                << "        " << cs_asm[i]
                << "        " << cs_sse[i]
                << "          " << tiempo_c[i] / tiempo_asm[i]
                << "          " << tiempo_c[i] / tiempo_sse[i]
                << endl;
        }
    }
    return 0;
}