#define _POSIX_C_SOURCE 200809L
#include "abb.h"
#include <stdlib.h>
#include <string.h>
#include "pila.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#define IZQ 1
#define DER -1

typedef struct abb_nodo{
	struct abb_nodo* izq;
	struct abb_nodo* der;
	char* clave;
	void* dato;
}abb_nodo_t;

struct abb{
	size_t cantidad;
	abb_nodo_t* raiz;
	abb_comparar_clave_t cmp;
	abb_destruir_dato_t destruir_dato;
};

struct abb_iter{
  pila_t* pila;
  const char* desde;
  const char* hasta;
  int modo;
  abb_comparar_clave_t funcion_comparar;
};

/* ******************************************************************
 *                       PRIMITIVAS DEL ABB
 * *****************************************************************/

char* crear_clave(const char* clave){
	char* clave_nueva = malloc(sizeof(char) * (strlen(clave) + 1)); //para el \0
	if(!clave_nueva) return NULL;
	strcpy(clave_nueva, clave);
	return clave_nueva;
}

abb_nodo_t* abb_nodo_crear (const char* clave, void* dato){
	abb_nodo_t* nodo = malloc(sizeof(abb_nodo_t));
	if(!nodo) return NULL;
	nodo->clave = crear_clave(clave);
	nodo->dato = dato;
	nodo->izq = NULL;
	nodo->der = NULL;
	return nodo;
}

void* abb_nodo_destruir(abb_nodo_t* nodo){ //devuelve el dato y destruye el nodo
	if(!nodo) return NULL;
	//void* dato = nodo->dato;
	free((char*)nodo->clave); //por el malloc en crear_clave
	free(nodo);
	return nodo->dato;
}

abb_t* abb_crear(abb_comparar_clave_t cmp, abb_destruir_dato_t destruir_dato){
	abb_t* arbol = malloc(sizeof(abb_t));
	if(!arbol) return NULL;
	arbol->cantidad = 0;
	arbol->raiz = NULL;
	arbol->cmp = cmp;
	arbol->destruir_dato = destruir_dato;
	return arbol;
}

bool _abb_guardar(abb_nodo_t* padre, abb_nodo_t* nodo, abb_t *arbol, int hijo, const char *clave, void *dato){
	if(!nodo){ //aca creo el nodo del dato que voy a guardar
		nodo = abb_nodo_crear(clave,dato);
		arbol->cantidad++;
		if(hijo == DER) padre->der = nodo; //agrego la referencia del padre
		else padre->izq = nodo; //agrego la referencia del padre
		return true;
	}
	if(arbol->cmp(nodo->clave,clave) == 0){ //si la clave ya existe reemplazo el dato
		if(arbol->destruir_dato) arbol->destruir_dato(nodo->dato);
		nodo->dato = dato;
		return true;
	}
	if(arbol->cmp(nodo->clave, clave) < 0) return _abb_guardar(nodo, nodo->der, arbol, DER, clave, dato); //el entero es dependiendo del lado que se debe agregar para tener referencia del padre
	if(arbol->cmp(nodo->clave, clave) > 0) return _abb_guardar(nodo, nodo->izq, arbol, IZQ, clave, dato);
	return false;
}

bool abb_guardar(abb_t *arbol, const char *clave, void *dato){
	if(!arbol) return false;
	if(abb_cantidad(arbol) == 0){ //si el arbol esta vacio, creo la raíz
		abb_nodo_t* nodo = abb_nodo_crear(clave,dato);
		arbol->cantidad = 1;
		arbol->raiz = nodo;
		return true;
	}
		return _abb_guardar(NULL, arbol->raiz, arbol, 0, clave, dato);
}

abb_nodo_t* _abb_buscar(abb_nodo_t* nodo, const abb_t* arbol, const char* clave){ //recorrido pre-order
	if(!nodo) return NULL;
	if(arbol->cmp(nodo->clave,clave) == 0) return nodo;
	if(arbol->cmp(nodo->clave,clave) < 0) return _abb_buscar(nodo->der, arbol, clave);
	if(arbol->cmp(nodo->clave,clave) > 0) return _abb_buscar(nodo->izq, arbol, clave);
	return NULL;
}

abb_nodo_t* abb_buscar(const abb_t* arbol, const char* clave){
	if(!arbol) return NULL;
	return _abb_buscar(arbol->raiz, arbol, clave); //empieza a buscar por la raiz
}

abb_nodo_t* buscar_padre(abb_t* arbol, abb_nodo_t* nodo, const char *clave){ //funcion que busca el padre de un nodo y si es hijo derecho o izquierdo

	if(!nodo) return NULL;

	if(!nodo->izq && !nodo->der) return NULL;

	if((nodo->izq && arbol->cmp(nodo->izq->clave, clave) == 0) || (nodo->der && arbol->cmp(nodo->der->clave, clave) == 0)) return nodo;

	if(arbol->cmp(nodo->clave, clave) > 0) return buscar_padre(arbol, nodo->izq, clave);

	if(arbol->cmp(nodo->clave, clave) < 0) return buscar_padre(arbol, nodo->der, clave);

	return NULL;
}

void* buscar_reemplazante(abb_t *arbol, abb_nodo_t *nodo){
	while(nodo && nodo->izq)
		nodo = nodo->izq;
  return nodo;
}

void destruir_nodo(abb_nodo_t* nodo){
	free(nodo->clave);
	free(nodo);
}

void acomodar_abb_1_hijo(abb_t* arbol, abb_nodo_t* padre, abb_nodo_t* nodo, const char* clave){
	if(nodo->izq && !nodo->der){ //Si tiene hijo izquierdo
		if(padre){
			if(padre->izq && arbol->cmp(padre->izq->clave, clave) == 0) padre->izq = nodo->izq;
			else if(padre->der && arbol->cmp(padre->der->clave, clave) == 0) padre->der = nodo->izq;
		}else arbol->raiz = nodo->izq;
	}else if(!nodo->izq && nodo->der){ //Si tiene hijo derecho
		if(padre){
			if(padre->izq && arbol->cmp(padre->izq->clave, clave) == 0) padre->izq = nodo->der;
			else if(padre->der && arbol->cmp(padre->der->clave, clave) == 0) padre->der = nodo->der;
		}else arbol->raiz = nodo->der;
	}
}

void *abb_borrar(abb_t *arbol, const char *clave){ //devuelve el dato del valor borrado
	if(abb_cantidad(arbol) == 0 || !arbol) return NULL;
	abb_nodo_t* nodo = abb_buscar(arbol,clave);
	if(!nodo) return NULL;
	if(!nodo->izq && !nodo->der){ //si no tiene hijos
		arbol->cantidad -= 1;
		void* dato = nodo->dato;
		abb_nodo_t* padre = buscar_padre(arbol, arbol->raiz, clave);
		if(padre){
			if(padre->izq && arbol->cmp(padre->izq->clave, clave) == 0) padre->izq = NULL;
			else if(padre->der && arbol->cmp(padre->der->clave, clave) == 0) padre->der = NULL;
		}else arbol->raiz = NULL;
		destruir_nodo(nodo);
		return dato;
	}else if(nodo->izq && nodo->der){ //si tiene dos hijos
		abb_nodo_t* reemplazante = buscar_reemplazante(arbol, nodo->der);
		char* clave_aux = strdup(reemplazante->clave);
		void* dato_a_devolver = nodo->dato;
		void* dato = abb_borrar(arbol, reemplazante->clave);
		free(nodo->clave);
		nodo->clave = clave_aux;
		nodo->dato = dato;
		return dato_a_devolver;
	}else{ //si solo tiene uno
		abb_nodo_t* padre = buscar_padre(arbol, arbol->raiz, clave);
		void* dato = nodo->dato;
		arbol->cantidad -= 1;
		acomodar_abb_1_hijo(arbol, padre, nodo, clave);
		destruir_nodo(nodo);
		return dato;
	}
	return NULL;
}

void *abb_obtener(const abb_t *arbol, const char *clave){
	abb_nodo_t* nodo = abb_buscar(arbol,clave);
	if(!nodo) return NULL;
	return nodo->dato;
}

bool abb_pertenece(const abb_t *arbol, const char *clave){
	abb_nodo_t* nodo = abb_buscar(arbol,clave);
	char* dato_aux = abb_obtener(arbol,clave);
	if(!nodo) return false;
	return dato_aux == nodo->dato;
}

size_t abb_cantidad(abb_t *arbol){
	if(!arbol) return 0;
	return arbol->cantidad;
}

bool _abb_in_order(abb_nodo_t* nodo, bool visitar(const char *, void *, void *), void *extra){
	if(nodo->izq) if(!_abb_in_order(nodo->izq, visitar, extra)) return false;
	if(!visitar(nodo->clave, nodo->dato, extra)) return false;
	if(nodo->der) if(!_abb_in_order(nodo->der, visitar, extra)) return false;
	return true;
}

void abb_in_order(abb_t *arbol, bool visitar(const char *, void *, void *), void *extra){
	if(arbol->raiz && visitar) _abb_in_order(arbol->raiz, visitar, extra);
}

void _abb_destruir(abb_t *arbol,abb_nodo_t* nodo){ //destruimos recorriendo post-order
	free(nodo->clave);
	if(nodo){
		if(nodo->izq) _abb_destruir(arbol, nodo->izq);
		if(nodo->der) _abb_destruir(arbol, nodo->der);
		if(arbol->destruir_dato) arbol->destruir_dato(nodo->dato);
	}
	free(nodo);
}

void abb_destruir(abb_t *arbol){
	if(arbol->raiz) _abb_destruir(arbol, arbol->raiz);
	free(arbol);
}


/* ******************************************************************
 *                 PRIMITIVAS DEL ITERADOR EXTERNO
 * *****************************************************************/
void abb_iter_apilar_modificado(pila_t* pila, abb_nodo_t* nodo, abb_iter_t* iter){
	if (!nodo) return;
	bool validar_desde;
	bool validar_hasta;
	if(iter->funcion_comparar(iter->desde, nodo->clave) <= 0) validar_desde = true;
	else validar_desde = false;
	if(iter->funcion_comparar(iter->hasta, nodo->clave) >= 0)	validar_hasta = true;
	else validar_hasta = false;
	if(validar_desde && validar_hasta){ //si la fecha se encuentra entre dos rangos chequeo el modo y apilo
		pila_apilar(pila, nodo);
		if (iter->modo) abb_iter_apilar_modificado(iter->pila, nodo->izq, iter);
		else abb_iter_apilar_modificado(iter->pila, nodo->der, iter);
		return;
	}
	if(iter->modo ? validar_desde : validar_hasta) abb_iter_apilar_modificado(iter->pila, iter->modo ? nodo->izq : nodo->der, iter);
	else abb_iter_apilar_modificado(iter->pila, iter->modo ? nodo->der : nodo->izq, iter);
	return;
}
abb_iter_t* abb_iter_in_crear(const abb_t* arbol, const char* desde, const char* hasta, const char* modo){
	//al iterador del abb le agregamos el modo, fecha y hasta.
	//dependiendo si es asc o desc, apilamos de una u otra forma.
	abb_iter_t* iter = malloc(sizeof(abb_iter_t));
  if(!iter) return NULL;
  pila_t* pila = pila_crear();
  if(!pila){
    free(iter);
    return NULL;
  }
	iter->desde = desde;
	iter->hasta = hasta;
	iter->funcion_comparar = arbol->cmp;
	iter->pila = pila;
	//asc = 1 y desc = 0
	if (strcmp(modo, "asc") == 0) iter->modo = 1;
	else iter->modo = 0;
	abb_iter_apilar_modificado(iter->pila, arbol->raiz, iter);
	return iter;
}

bool abb_iter_in_avanzar(abb_iter_t* iter){
	if(abb_iter_in_al_final(iter)) return false;
  abb_nodo_t* aux = pila_desapilar(iter->pila);
	if (iter->modo) abb_iter_apilar_modificado(iter->pila, aux->der, iter);
	else abb_iter_apilar_modificado(iter->pila, aux->izq, iter);
	return true;
}

const char* abb_iter_in_ver_actual(const abb_iter_t* iter){
    if(abb_iter_in_al_final(iter)) return NULL;
    abb_nodo_t* aux = pila_ver_tope(iter->pila);
    return aux->clave;
}

bool abb_iter_in_al_final(const abb_iter_t *iter){ //si el actual es null es porque esta al final
	return pila_esta_vacia(iter->pila);
}

void abb_iter_in_destruir(abb_iter_t* iter){
    pila_destruir(iter->pila);
    free(iter);
}
