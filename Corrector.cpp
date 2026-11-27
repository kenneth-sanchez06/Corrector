// 7.62
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "corrector.h"

#define ALFABETO "abcdefghijklmnopqrstuvwxyzñÑ'&$#!%<[]>‐--+/\"_:?¿¡@="
#define MAX_PALABRAS 50000

// 1/6 de calificacion
// Crear el diccionario completo
void Diccionario(char* szNombre, char szPalabras[][TAMTOKEN], int iEstadisticas[], int& iNumElementos) {
    FILE* archivo;
    if (fopen_s(&archivo, szNombre, "r") != 0) {
        printf("No se pudo abrir el archivo: %s.\n", szNombre);
        return;
    }

    char palabra[TAMTOKEN];
    iNumElementos = 0;

    while (fscanf_s(archivo, "%49s", palabra, (unsigned)_countof(palabra)) != EOF) {
        int inicio = 0; // Índice para procesar subpalabras
        for (int i = 0; palabra[i]; i++) {
            // Verificar separadores
            if (isspace((unsigned char)palabra[i]) || strchr(",.;()[]", palabra[i])) {
                if (i > inicio) {
                    char subPalabra[TAMTOKEN];
                    int j = 0;
                    for (int k = inicio; k < i && j < TAMTOKEN - 1; k++) {
                        subPalabra[j++] = tolower((unsigned char)palabra[k]);
                    }
                    subPalabra[j] = '\0'; // Terminar subpalabra

                    if (strlen(subPalabra) > 0) {
                        // Agregar subpalabra al diccionario
                        int existe = -1;
                        for (int l = 0; l < iNumElementos; l++) {
                            if (strcmp(szPalabras[l], subPalabra) == 0) {
                                existe = l;
                                break;
                            }
                        }

                        if (existe == -1 && iNumElementos < NUMPALABRAS) {
                            strcpy_s(szPalabras[iNumElementos], TAMTOKEN, subPalabra);
                            iEstadisticas[iNumElementos] = 1;
                            iNumElementos++;
                        } else if (existe != -1) {
                            iEstadisticas[existe]++;
                        }
                    }
                }
                inicio = i + 1; // Avanzar inicio después del separador
            }
        }

        // Procesar palabra restante después del último separador
        if (palabra[inicio]) {
            char subPalabra[TAMTOKEN];
            int j = 0;
            for (int k = inicio; palabra[k] && j < TAMTOKEN - 1; k++) {
                subPalabra[j++] = tolower((unsigned char)palabra[k]);
            }
            subPalabra[j] = '\0';

            if (strlen(subPalabra) > 0) {
                // Agregar subpalabra al diccionario
                int existe = -1;
                for (int l = 0; l < iNumElementos; l++) {
                    if (strcmp(szPalabras[l], subPalabra) == 0) {
                        existe = l;
                        break;
                    }
                }

                if (existe == -1 && iNumElementos < NUMPALABRAS) {
                    strcpy_s(szPalabras[iNumElementos], TAMTOKEN, subPalabra);
                    iEstadisticas[iNumElementos] = 1;
                    iNumElementos++;
                } else if (existe != -1) {
                    iEstadisticas[existe]++;
                }
            }
        }
    }
    fclose(archivo);

    // Ordenar el diccionario alfabéticamente
    for (int i = 0; i < iNumElementos - 1; i++) {
        for (int j = i + 1; j < iNumElementos; j++) {
            if (strcmp(szPalabras[i], szPalabras[j]) > 0) {
                char temp[TAMTOKEN];
                strcpy_s(temp, TAMTOKEN, szPalabras[i]);
                strcpy_s(szPalabras[i], TAMTOKEN, szPalabras[j]);
                strcpy_s(szPalabras[j], TAMTOKEN, temp);

                int tempEstadistica = iEstadisticas[i];
                iEstadisticas[i] = iEstadisticas[j];
                iEstadisticas[j] = tempEstadistica;
            }
        }
    }
}


// Generar palabras similares (clonación)
// 0 de calificacion
void ClonaPalabras(char* szPalabraLeida, char szPalabrasSugeridas[][TAMTOKEN], int& iNumSugeridas) {
    iNumSugeridas = 0;
    int len = strlen(szPalabraLeida);

    // Eliminación de caracteres
    for (int i = 0; i < len; i++) {
        char copia[TAMTOKEN];
        strcpy_s(copia, TAMTOKEN, szPalabraLeida);
        memmove(&copia[i], &copia[i + 1], len - i);
        strcpy_s(szPalabrasSugeridas[iNumSugeridas++], TAMTOKEN, copia);
    }

    // Transposición de caracteres
    for (int i = 0; i < len - 1; i++) {
        char copia[TAMTOKEN];
        strcpy_s(copia, TAMTOKEN, szPalabraLeida);
        char temp = copia[i];
        copia[i] = copia[i + 1];
        copia[i + 1] = temp;
        strcpy_s(szPalabrasSugeridas[iNumSugeridas++], TAMTOKEN, copia);
    }

    // Sustitución de caracteres
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < strlen(ALFABETO); j++) {
            char copia[TAMTOKEN];
            strcpy_s(copia, TAMTOKEN, szPalabraLeida);
            copia[i] = ALFABETO[j];
            strcpy_s(szPalabrasSugeridas[iNumSugeridas++], TAMTOKEN, copia);
        }
    }

    // Inserción de caracteres
    for (int i = 0; i <= len; i++) {
        for (int j = 0; j < strlen(ALFABETO); j++) {
            char copia[TAMTOKEN];
            strncpy_s(copia, TAMTOKEN, szPalabraLeida, i);
            copia[i] = ALFABETO[j];
            strcpy_s(&copia[i + 1], TAMTOKEN - i - 1, &szPalabraLeida[i]);
            strcpy_s(szPalabrasSugeridas[iNumSugeridas++], TAMTOKEN, copia);
        }
    }
}
// 10 DE CALIFICACION
// Obtener la lista de palabras candidatas
void ListaCandidatas(char szPalabrasSugeridas[][TAMTOKEN], int iNumSugeridas, char szPalabras[][TAMTOKEN],
                     int iEstadisticas[], int iNumElementos, char szListaFinal[][TAMTOKEN],
                     int iPeso[], int& iNumLista) {
    iNumLista = 0;
    for (int i = 0; i < iNumSugeridas; i++) {
        for (int j = 0; j < iNumElementos; j++) {
            if (strcmp(szPalabrasSugeridas[i], szPalabras[j]) == 0) {
                int duplicado = 0;
                for (int k = 0; k < iNumLista; k++) {
                    if (strcmp(szListaFinal[k], szPalabras[j]) == 0) {
                        duplicado = 1;
                        break;
                    }
                }
                if (!duplicado) {
                    strcpy_s(szListaFinal[iNumLista], TAMTOKEN, szPalabras[j]);
                    iPeso[iNumLista] = iEstadisticas[j];
                    iNumLista++;
                }
            }
        }
    }

    // Ordenar la lista final por peso descendente
    for (int i = 0; i < iNumLista - 1; i++) {
        for (int j = i + 1; j < iNumLista; j++) {
            if (iPeso[i] < iPeso[j]) {
                int tempPeso = iPeso[i];
                iPeso[i] = iPeso[j];
                iPeso[j] = tempPeso;

                char tempPalabra[TAMTOKEN];
                strcpy_s(tempPalabra, TAMTOKEN, szListaFinal[i]);
                strcpy_s(szListaFinal[i], TAMTOKEN, szListaFinal[j]);
                strcpy_s(szListaFinal[j], TAMTOKEN, tempPalabra);
            }
        }
    }
}