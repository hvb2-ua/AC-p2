#include <cstdint>

void inicializar(int* a, int* b, int* c, int n)
{
    /* Inicialización:
       - a empieza en 0 porque es el vector resultado
       - b y c usan fórmulas deterministas para que todo sea reproducible */
    for (size_t i = 0; i < n; i++)
    {
        a[i] = 0;
        b[i] = i * 17 + 4;
        c[i] = i * 46 + 13;
    }
}