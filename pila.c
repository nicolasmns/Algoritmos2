#include "pila.h"
#include <stdlib.h>
#define CAPACIDAD_INI 10
#define REDIMENSION 2
#define CRITERIO_REDIMENSION 4
/* Definición del struct pila proporcionado por la cátedra.
 */
struct pila {
  void** datos;
  size_t cantidad;  // Cantidad de elementos almacenados.
  size_t capacidad;  // Capacidad del arreglo 'datos'.
};

/* *****************************************************************
 *                    PRIMITIVAS DE LA PILA
 * *****************************************************************/
bool redimensionar(pila_t* pila, size_t nueva_redimension);
// ...
pila_t* pila_crear(void){
  pila_t* pila = malloc(sizeof(pila_t));
  if (pila == NULL) return NULL;
  pila->capacidad = CAPACIDAD_INI;
  pila->cantidad = 0;
  pila->datos = malloc(sizeof(void*) * pila->capacidad);
  if(pila->datos == NULL){
    free(pila);
    return NULL;
  }
  return pila;
}

void pila_destruir(pila_t *pila){
  free(pila->datos);
  free(pila);
}

bool pila_esta_vacia(const pila_t *pila){
  return pila != NULL && pila->cantidad == 0;
}

bool pila_apilar(pila_t *pila, void* valor){
  if(pila == NULL) return false;
  pila->cantidad+=1;
  if(pila->capacidad == pila->cantidad){
    bool resultado = redimensionar(pila, pila->capacidad*REDIMENSION);
    if(!resultado) return false;
  }
  pila->datos[pila->cantidad] = valor;
  return true;
}

void* pila_ver_tope(const pila_t *pila){
  if(pila == NULL || pila_esta_vacia(pila)) return NULL;
  return pila->datos[pila->cantidad];
}

void* pila_desapilar(pila_t *pila){
  if(pila == NULL || pila_esta_vacia(pila)) return NULL;
  void* tope_anterior = pila->datos[pila->cantidad];
  pila->cantidad -= 1;
  if(pila->cantidad <= pila->capacidad/CRITERIO_REDIMENSION){
		bool resultado = redimensionar(pila, pila->capacidad/REDIMENSION);
    if(!resultado) return false;
	}
  return tope_anterior;
}

bool redimensionar(pila_t* pila, size_t nueva_redimension){
  void* nuevos_datos = realloc(pila->datos, sizeof(void*)* nueva_redimension);
  if(nuevos_datos == NULL) return false;
  pila->datos = nuevos_datos;
	pila->capacidad = nueva_redimension;
  return true;
}
