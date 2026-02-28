#include <cstdint>

void bench_cpp(int* a, int* b, int* c, int n, int iteraciones, int escalar, uint32_t& checksum)
{

    /* Bucle principal:
       Repetimos el kernel varias veces para aumentar la carga y que los
       tiempos sean más estables. */

    
        for (size_t i = 0; i < iteraciones; i++)
        {
            for (size_t j = 0; j < n; j++)
            {
                a[j] += b[j] + escalar * c[j];
            }
        }

    /* Checksum:
       Se usa XOR para mezclar todos los valores del vector en un único número.
       Si cambia cualquier elemento, el checksum también cambia.

       Además se suma la posición (índice) para que valores iguales en sitios
       distintos no se cancelen y no salgan patrones raros.

       Esto sirve para comprobar que el cálculo está hecho y para evitar que
       el compilador elimine el trabajo por “no usarse el resultado”. */

    uint32_t cs = 0;
    for (size_t i = 0; i < n; i++)
    {
        cs ^= (uint32_t)a[i] + (uint32_t)i;
    }
    checksum = cs;
}