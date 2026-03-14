#include <cstdint>
#include <cstddef>

/* 
   En esta versión del benchmark usamos instrucciones SSE para
   vectorizar el cálculo. Esto permite procesar 4 enteros a la vez
   en lugar de uno solo, aprovechando los registros SIMD de la CPU.

   La idea es hacer exactamente el mismo cálculo que en las otras
   versiones pero de forma más eficiente, para poder comparar los
   tiempos y ver la ganancia de speedup que se obtiene gracias a
   la vectorización.
  */

void bench_sse(int* a, int* b, int* c, int n, int iteraciones, int escalar, uint32_t& checksum)
{
    int* pa = a;
    int* pb = b;
    int* pc = c;
    int  S = escalar;
    int  N = n;
    int  I = iteraciones;
    uint32_t* pchk = &checksum;

    int bloques = n / 4;                    // Calculamos cuántos bloques completos de 4 enteros caben dentro del vector.
                                            // Lo hacemos así porque con SSE procesamos 4 enteros de 32 bits a la vez
                                            // en un mismo registro XMM de 128 bits.
                                            // Por ejemplo, si n = 16, entonces bloques = 4, porque hay 4 grupos de 4 enteros.
                                            // Si n no es múltiplo de 4, los elementos que sobren se procesarán después en el bucle escalar del resto.

    __asm
    {
        push ebx                            // Guardamos EBX porque lo vamos a utilizar dentro del bloque asm
        push esi                            // Guardamos ESI porque lo usaremos como puntero al vector a
        push edi                            // Guardamos EDI porque lo usaremos como puntero al vector b

        mov eax, S                          // Pasamos el escalar S al registro EAX
        movd xmm7, eax                      // Cargamos el valor de EAX en el registro XMM7
        pshufd xmm7, xmm7, 0                // Replicamos el escalar en las 4 posiciones de XMM7 -> xmm7 = [S,S,S,S]

        mov ecx, I                          // Pasamos la variable iteraciones al registro ECX (contador externo)

        BucleExterno :
        test ecx, ecx                       // Comprobamos si ECX (iteraciones restantes) es <= 0
            jle FinBucleExterno             // Si no quedan iteraciones salimos del bucle externo

            mov esi, pa                     // ESI apunta al inicio del vector a (a[0])
            mov edi, pb                     // EDI apunta al inicio del vector b (b[0])
            mov ebx, pc                     // EBX apunta al inicio del vector c (c[0])

            mov edx, bloques                // Pasamos a EDX el numero de bloques de 4 enteros que vamos a procesar con SSE

            RecorrerVectorSIMD :
        test edx, edx                       // Comprobamos si EDX (bloques restantes) es <= 0
            jle FinRecorrerVectorSIMD       // Si no quedan bloques salimos del bucle SIMD

            movdqu xmm0, xmmword ptr[ebx]   // Cargamos c[j..j+3] en XMM0 (EBX apunta al bloque actual de c)
            movdqu xmm1, xmmword ptr[edi]   // Cargamos b[j..j+3] en XMM1 (EDI apunta al bloque actual de b)
            movdqu xmm2, xmmword ptr[esi]   // Cargamos a[j..j+3] en XMM2 (ESI apunta al bloque actual de a)

            pmulld xmm0, xmm7               // Multiplicamos c[j..j+3] por el escalar S -> xmm0 = S * c[j..j+3]
            paddd xmm0, xmm1                // Sumamos b[j..j+3] + S*c[j..j+3] y dejamos el resultado en XMM0
            paddd xmm2, xmm0                // Sumamos a[j..j+3] + (b[j..j+3] + S*c[j..j+3]) y dejamos el resultado en XMM2

            movdqu xmmword ptr[esi], xmm2  // Guardamos el bloque resultado en a[j..j+3]

            add esi, 16                     // Avanza el puntero de a al siguiente bloque de 4 enteros (4*4 bytes = 16)
            add edi, 16                     // Avanza el puntero de b al siguiente bloque
            add ebx, 16                     // Avanza el puntero de c al siguiente bloque

            dec edx                         // Decrementamos el contador de bloques SIMD
            jmp RecorrerVectorSIMD          // Volvemos al inicio del bucle SIMD

            FinRecorrerVectorSIMD :

        mov edx, N                          // Pasamos la variable n al registro EDX
            and edx, 3                      // Calculamos el resto de elementos que no caben en bloques de 4 -> resto = n % 4

            RecorrerVectorResto :
            test edx, edx                   // Comprobamos si quedan elementos sueltos por procesar
            jle FinRecorrerVectorResto      // Si no quedan elementos salimos del bucle del resto

            mov eax, [ebx]                  // Cargamos c[j] en EAX
            imul eax, S                     // Multiplicamos c[j] por el escalar S
            add eax, [edi]                  // Sumamos b[j] + S*c[j]
            add[esi], eax                   // a[j] += resultado

            add esi, 4                      // Avanza el puntero de a al siguiente elemento
            add edi, 4                      // Avanza el puntero de b al siguiente elemento
            add ebx, 4                      // Avanza el puntero de c al siguiente elemento

            dec edx                         // Decrementamos el contador del resto
            jmp RecorrerVectorResto         // Volvemos al inicio del bucle del resto

            FinRecorrerVectorResto :

        dec ecx                             // Decrementamos el contador externo (queda una iteracion menos)
            jmp BucleExterno                // Volvemos al inicio del bucle externo

            FinBucleExterno :

        mov esi, pa                         // Volvemos a apuntar al inicio del vector a para calcular el checksum
            mov ecx, N                      // Pasamos la variable n al registro ECX (contador del checksum)
            xor eax, eax                    // Inicializamos el acumulador del checksum -> cs = 0
            xor edx, edx                    // Inicializamos el indice i = 0

            BucleChecksum :
        test ecx, ecx                       // Comprobamos si quedan elementos por recorrer
            jle FinChecksum                 // Si ECX <= 0 terminamos el checksum

            mov ebx, [esi]                  // Cargamos a[i] en EBX
            add ebx, edx                    // Sumamos a[i] + i
            xor eax, ebx                    // cs ^= (a[i] + i)

            add esi, 4                      // Avanzamos al siguiente elemento de a
            inc edx                         // i = i + 1
            dec ecx                         // Decrementamos el contador
            jmp BucleChecksum               // Volvemos al inicio del bucle

            FinChecksum :
        mov edi, pchk                       // Pasamos la direccion de checksum a EDI
            mov[edi], eax                   // Guardamos el valor final del checksum

            pop edi                         // Restauramos EDI
            pop esi                         // Restauramos ESI
            pop ebx                         // Restauramos EBX
    }
}