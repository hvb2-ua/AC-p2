#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <chrono>
#include <iomanip>

using namespace std;

/* ================================================================
   Este programa ejecuta la versión en C/C++ del kernel inspirado
   en STREAM TRIAD para usarla como referencia y poder compararla
   posteriormente con una versión en ensamblador x86.

   La idea es medir el tiempo de ejecución de un cálculo intensivo
   sobre vectores grandes y obtener además un checksum que permita
   verificar que el resultado es correcto.

   De esta forma, cuando se implemente la versión en ASM, se podrá
   comprobar que:
   - produce exactamente el mismo resultado
   - y comparar directamente los tiempos de ejecución

   Este archivo actúa como driver del benchmark en C/C++.
   ================================================================ */


   /* Prototipo de la función implementada en Benchmark_cpp.cpp.
      Se separa la lógica del benchmark en una función independiente
      para poder reutilizarla o sustituirla por la versión en ASM. */
void bench_cpp(int* a, int* b, int* c,int n,int iteraciones,int escalar,uint32_t& checksum);
void inicializar(int* a, int* b, int* c, int n);
void bench_asm(int* a, int* b, int* c, int n, int iteraciones, int escalar, uint32_t& checksum);

int main()
{
    /* Tamaño del problema: 2^20 elementos (~1 millón).
       Este tamaño es lo bastante grande como para generar carga
       real sobre CPU y memoria, pero sin consumir demasiada RAM. */
    const int n = 1 << 20;

    int iteraciones = 300;   // Número de veces que se repite el kernel
    const int escalar = 7;       // Constante usada en el cálculo
    const int repeticiones = 20;
    vector<double> tiempo_c(repeticiones);
    vector<double> tiempo_asm(repeticiones);

    /* Se reservan tres vectores dinámicos de tamaño n.
       std::vector garantiza memoria contigua, lo cual es importante
       para rendimiento y para poder pasar punteros a la función. */
    vector<int> a(n);
    vector<int> b(n);
    vector<int> c(n);


    /* Variable donde se almacenará el checksum final.
       Se pasa por referencia a la función para que pueda escribir
       el resultado directamente aquí. */
    vector<uint32_t> csc(20);
    vector<uint32_t> cs_asm(20);
    uint32_t  checksum_c = 0;
    uint32_t  checksum_asm = 0;

    /* Inicio de la medición de tiempo.
       Se usa un reloj de alta resolución para obtener medidas
       precisas del tiempo de ejecución real. */
    auto inicio = chrono::high_resolution_clock::now();


    /* Llamada al benchmark en C/C++.

       Se pasan punteros a los datos reales usando .data(),
       junto con el tamaño, número de iteraciones y el escalar.

       La función se encarga de:
       - inicializar los vectores
       - ejecutar el kernel varias veces
       - calcular el checksum final */

    for (int i = 0; i < repeticiones; i++)
    {
        inicializar(a.data(), b.data(), c.data(), n);

        auto t0 = chrono::high_resolution_clock::now();
        bench_cpp(a.data(), b.data(), c.data(), n, iteraciones, escalar, checksum_c);
        auto t1 = chrono::high_resolution_clock::now();

        chrono::duration<double> dt = t1 - t0;

        tiempo_c[i] = dt.count();   // <- ahora sí (double en segundos)
        csc[i] = checksum_c;
    }


    for (int i = 0; i < repeticiones; i++)
    {
        inicializar(a.data(), b.data(), c.data(), n);

        auto t0 = chrono::high_resolution_clock::now();
        bench_asm(a.data(), b.data(), c.data(), n, iteraciones, escalar, checksum_asm);
        auto t1 = chrono::high_resolution_clock::now();

        chrono::duration<double> dt = t1 - t0;

        tiempo_asm[i] = dt.count();   // <- ahora sí (double en segundos)
        cs_asm[i] = checksum_asm;
    }

    /* Salida por pantalla:
       - checksum para validar que el cálculo es correcto
       - tiempo total de ejecución medido */


    cout << "Iteracion        Tiempo_C        Tiempo_x86        Checksum_C        Checksum_x86" << endl;
    cout << "---------------------------------------------------------------------------------" << endl;
                            
    cout << fixed << setprecision(4);

    for (int i = 0; i < repeticiones; i++)
    {
        if (i < 9)
        {
            cout << "     " << i + 1 << "            " << tiempo_c[i] << "(s)" << "       " << tiempo_asm[i] << "(s)" << "        " << csc[i] << "        " << cs_asm[i] << endl;
        }
        else
        {
            cout << "     " << i + 1 << "           " << tiempo_c[i] << "(s)" << "       " << tiempo_asm[i] << "(s)" << "        " << csc[i] << "        " << cs_asm[i] << endl;
        }

    }
    return 0;
}