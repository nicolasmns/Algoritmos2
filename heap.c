#include "heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CAPACIDAD_ORIGINAL 10
#define FACTOR_REDIMENSION 2

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/


struct heap {
	void** datos;
	size_t cant;
	size_t tam;
	cmp_func_t cmp;
};


/* ******************************************************************
 *                        FUNCIONES VARIAS
 * *****************************************************************/

void swap(void** arr, size_t a, size_t b){
    void* aux = arr[a];
    arr[a] = arr[b];
    arr[b] = aux;
}

bool heap_redimensionar(heap_t* heap, size_t tam_nuevo){
	if (tam_nuevo < CAPACIDAD_ORIGINAL)	tam_nuevo = CAPACIDAD_ORIGINAL;
	void** datos_nuevos = realloc(heap->datos, tam_nuevo * sizeof(void*));
	if (!datos_nuevos)	return false;
	heap->datos = datos_nuevos;
	heap->tam = tam_nuevo;
	return true;
}

size_t obtener_padre(size_t pos){
	return (pos-1) / 2;
}

size_t obtener_hijo_izq(size_t pos) {
    return pos * 2 + 1;
}

size_t obtener_hijo_der(size_t pos) {
    return pos * 2 + 2;
}

void upheap(void** arr, size_t pos, cmp_func_t cmp){
	if (pos == 0)	return;
	size_t pos_padre = obtener_padre(pos);
	if (cmp(arr[pos], arr[pos_padre]) > 0){
		swap(arr, pos, pos_padre);
		upheap(arr, pos_padre, cmp);
	}
}

void downheap(void** arr, size_t cant, size_t pos, cmp_func_t cmp){
	if (pos == cant) return;

	size_t max = pos;
	size_t pos_hijo_izq = obtener_hijo_izq(pos);
	size_t pos_hijo_der = obtener_hijo_der(pos);

	if ((pos_hijo_izq < cant) && (cmp(arr[pos_hijo_izq], arr[max]) > 0 ))	max = pos_hijo_izq;
	if ((pos_hijo_der < cant) && (cmp(arr[pos_hijo_der], arr[max]) > 0 ))	max = pos_hijo_der;

	if (max != pos){
		swap(arr, max, pos);
		downheap(arr, cant, max, cmp);
	}
}

void heapify(void** arr, size_t cant, cmp_func_t cmp){
	if (cant <=1) return;
	for (size_t i = cant/2; i > 0; i--){
		downheap(arr, cant, i-1, cmp);
	}
}


 /* ******************************************************************
 *                        PRIMITIVAS DEL HEAP
 * *****************************************************************/


heap_t *heap_crear(cmp_func_t cmp){
	heap_t* heap = malloc(sizeof(heap_t));
	if (!heap)	return NULL;

	void** datos = malloc(sizeof(void*) * CAPACIDAD_ORIGINAL);
	if (!datos){
		free(heap);
		return NULL;
	}

	heap->datos = datos;
	heap->cant = 0;
	heap->tam = CAPACIDAD_ORIGINAL;
	heap->cmp = cmp;
	return heap;
}

heap_t *heap_crear_arr(void *arreglo[], size_t n, cmp_func_t cmp){
	heap_t* heap = malloc(sizeof(heap_t));
	if (!heap) return NULL;
	size_t capacidad = 0;

	if (n < CAPACIDAD_ORIGINAL){
		capacidad = CAPACIDAD_ORIGINAL;
	}else{
		capacidad = n;
	}
	void** datos = malloc(sizeof(void*) * capacidad);

	if (!datos){
		free(heap);
		return NULL;
	}

	heap->cant = n;
	heap->tam = capacidad;
	heap->cmp = cmp;

	for(size_t j=0; j<n; j++){
		datos[j] = arreglo[j];
	}
	heapify(datos, n, cmp);
	heap->datos = datos;
	return heap;
}

void heap_destruir(heap_t *heap, void destruir_elemento(void *e)){
	while(!heap_esta_vacio(heap)){
        void* dato = heap_desencolar(heap);
        if(destruir_elemento != NULL) destruir_elemento(dato);
    }
    free(heap->datos);
    free(heap);
}

size_t heap_cantidad(const heap_t *heap){
	if (!heap)	return 0;
	return heap->cant;
}

bool heap_esta_vacio(const heap_t *heap){
	return heap->cant == 0;
}

bool heap_encolar(heap_t *heap, void *elem){
	if (heap->cant == heap->tam){
		if (!heap_redimensionar(heap, heap->tam * FACTOR_REDIMENSION))	return false;
	}
	heap->datos[heap->cant] = elem;
	upheap(heap->datos, heap->cant, heap->cmp);
	heap->cant++;
	return true;
}

void* heap_ver_max(const heap_t *heap){
	if (!heap) return NULL;
	return heap->datos[0];
}

void *heap_desencolar(heap_t *heap){
	if (!heap) return NULL;

	void* valor = heap->datos[0];
	heap->cant--;
	heap->datos[0] = heap->datos[heap->cant];
	downheap(heap->datos, heap->cant, 0, heap->cmp);

	if ((heap->cant > CAPACIDAD_ORIGINAL) && (heap->cant <= heap->tam / 4)){
		heap_redimensionar(heap, heap->tam / 2);
	}
	return valor;
}

void heap_sort(void *elementos[], size_t cant, cmp_func_t cmp){
	if (cant == 0) return;
	heapify(elementos, cant, cmp);
	for (size_t ultimo = cant - 1; ultimo > 0; ultimo--){
		swap(elementos, 0, ultimo);
		downheap(elementos, ultimo, 0, cmp);
	}
}
