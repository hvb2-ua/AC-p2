#include <cstdint>
#include <cstddef>

void bench_asm(int* a, int* b, int* c, int n, int iteraciones, int escalar, uint32_t& checksum)
{
    int* pa = a;
    int* pb = b;
    int* pc = c;
    int  S = escalar;
    int  N = n;
    int  I = iteraciones;
    uint32_t* pchk = &checksum;

    _asm
    {
        mov ecx, I                      // Pasamos la variable iteraciones al registro ECX (lo usamos como contador externo)

        BucleExterno :
        test ecx, ecx                   // Comprobamos si ECX (iteraciones restantes) es <= 0
            jle  FinBucleExterno        // Si ya no quedan iteraciones salimos del bucle externo

            mov edx, N                  // Pasamos la variable n al registro EDX (contador del bucle interno)
            mov esi, pa                 // ESI apunta al inicio del vector a (a[0])
            mov edi, pb                 // EDI apunta al inicio del vector b (b[0])
            mov ebx, pc                 // EBX apunta al inicio del vector c (c[0])

            RecorrerVector :
        test edx, edx                   // Comprobamos si EDX (elementos restantes) es <= 0
            jle  FinBucleInterno        // Si no quedan elementos salimos del bucle interno

            mov eax, [ebx]              // Cargamos c[j] en EAX (EBX apunta al elemento actual)
            imul eax, S                 // Multiplicamos c[j] por el escalar S → eax = S * c[j]
            add eax, [edi]              // Sumamos b[j] + S*c[j] y dejamos el resultado en EAX
            add[esi], eax               // a[j] += resultado (ESI apunta al elemento actual de a)

            add esi, 4                  // Avanza el puntero de a al siguiente elemento (cada int ocupa 4 bytes)
            add edi, 4                  // Avanza el puntero de b al siguiente elemento
            add ebx, 4                  // Avanza el puntero de c al siguiente elemento

            dec edx                     // Decrementamos el contador interno (queda un elemento menos por procesar)
            jmp RecorrerVector          // Volvemos al inicio del bucle interno

            FinBucleInterno :
        dec ecx                         // Decrementamos el contador externo (queda una iteración menos)
            jmp BucleExterno            // Volvemos al inicio del bucle externo

            FinBucleExterno :

            mov esi, pa                 // Volvemos a apuntar al inicio del vector a (a[0]) para calcular el checksum
            mov ecx, N                  // Pasamos la variable n al registro ECX (contador del bucle de checksum)
            mov eax, 0                  // Inicializamos el acumulador del checksum a 0 → cs = 0
            mov edx, 0                  // Inicializamos el índice i = 0 (lo usaremos para sumar la posición)

            BucleChecksum :
            test ecx, ecx               // Comprobamos si quedan elementos por recorrer en el checksum
            jle  FinChecksum            // Si ECX <= 0, terminamos el cálculo del checksum

            mov ebx, [esi]              // Cargamos a[i] en EBX (ESI apunta al elemento actual de a)
            add ebx, edx                // Sumamos a[i] + i (EDX es el índice)
            xor eax, ebx                // Hacemos cs ^= (a[i] + i) y lo dejamos acumulado en EAX

            add esi, 4                  // Avanzamos al siguiente elemento de a (int = 4 bytes)
            inc edx                     // Incrementamos el índice i (i = i + 1)
            dec ecx                     // Decrementamos el contador del checksum (queda un elemento menos)
            jmp BucleChecksum           // Volvemos al inicio del bucle de checksum

            FinChecksum:
            
            mov edi, pchk               // Pasamos la direccion de memoria de checksum a EDI (donde vamos a guardarlo)
            mov[edi], eax               // Guardamos el valor final del checksum (EAX) en la variable checksum
    }
}