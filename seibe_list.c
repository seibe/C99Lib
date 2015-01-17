#include <stdlib.h>
#include "seibe_list.h"

typedef struct seibelist_elem {
	struct seibelist_elem *next;
	struct seibelist_elem *prev;
	void *data;
} SeibeListElem;
typedef struct seibelist {
	struct seibelist_elem *first;
	struct seibelist_elem *last;
	int length;
} _SeibeList;

SeibeList SeibeList_create()
{
	SeibeList list;

	list = (SeibeList)malloc(sizeof(_SeibeList));
	if (list == NULL) return NULL;

	list->first = NULL;
	list->last = NULL;
	list->length = 0;

	return list;
}

void SeibeList_destroy(SeibeList list)
{
	if (!list) return;
	SeibeListElem *elem, *dest;

	elem = list->first;
	while (elem) {
		dest = elem;
		elem = elem->next;
		free(dest);
	}

	free(list);
	list = NULL;
}

void SeibeList_forEach(SeibeList list, SeibeListFunc func)
{
	if (!list) return;
	SeibeListElem *elem, *dest;
	int n, res;

	elem = list->first;
	while(elem) {
		res = 0;
		func(elem->data, &res);

		switch(res) {
			case -2:
				return;

			case -1:
				if (elem->prev) elem->prev->next = elem->next;
				else list->first = elem->next;
				if (elem->next) elem->next->prev = elem->prev;
				else list->last = elem->prev;
				--(list->length);

				dest = elem;
				elem = elem->next;
				free(dest);
				break;

			default:
				elem = elem->next;
		}
	}
}

void SeibeList_push(SeibeList list, void *data)
{
	if (!list) return;
	SeibeListElem *elem;

	elem = (SeibeListElem *)malloc(sizeof(SeibeListElem));
	elem->next = NULL;
	elem->prev = list->last;
	elem->data = data;

	if (list->length == 0) list->first = elem;
	else list->last->next = elem;	
	list->last = elem;

	++(list->length);
}

void SeibeList_unshift(SeibeList list, void *data)
{
	SeibeListElem *elem;

	if (!list) return;

	elem = (SeibeListElem *)malloc(sizeof(SeibeListElem));
	elem->next = list->first;
	elem->prev = NULL;
	elem->data = data;

	if (list->length == 0) list->last = elem;
	else list->first->prev = elem;	
	list->first = elem;

	++(list->length);
}

void SeibeList_insert(SeibeList list, int index, void *data)
{
	if (!list) return;
	int i, length;
	SeibeListElem *elem, *new;

	length = list->length;
	if (index >= length) return;

	elem = list->first;
	for (i = 0; i < index; ++i) {
		elem = elem->next;
	}

	new = (SeibeListElem *)malloc(sizeof(SeibeListElem));
	new->next = elem;
	new->prev = elem->prev;
	new->data = data;

	elem->prev->next = new;
	elem->prev = new;

	if (new->prev == NULL) list->first = new;
	if (new->next == NULL) list->last = new;
	++(list->length);
}

void *SeibeList_pop(SeibeList list)
{
	if (!list || list->length == 0) return NULL;
	void *data;
	SeibeListElem *elem;

	elem = list->last;
	data = elem->data;
	
	list->last = elem->prev;
	if (list->last) list->last->next = NULL;
	--(list->length);
	if (list->length <= 1) list->first = list->last;

	free(elem);
	return data;
}

void *SeibeList_shift(SeibeList list)
{
	if (!list || list->length == 0) return NULL;
	void *data;
	SeibeListElem *elem;

	elem = list->first;
	data = elem->data;
	
	list->first = elem->next;
	if (list->first) list->first->prev = NULL;
	--(list->length);
	if (list->length <= 1) list->last = list->first;

	free(elem);
	return data;
}

void *SeibeList_remove(SeibeList list, int index)
{
	if (!list || list->length == 0) return NULL;
	void *data;
	SeibeListElem *elem;

	index = index < 0 ? list->length + index : index;
	elem = list->first;
	while (elem && index > 0) {
		elem = elem->next;
		--index;
	}

	if (!elem) return NULL;
	data = elem->data;

	if (elem->prev) elem->prev->next = elem->next;
	if (elem->next) elem->next->prev = elem->prev;
	--(list->length);

	free(elem);
	return data;
}

void *SeibeList_get(SeibeList list, int index)
{
	if (!list || list->length == 0) return NULL;
	SeibeListElem *elem;

	index = index < 0 ? list->length + index : index;
	elem = list->first;
	while (elem && index > 0) {
		elem = elem->next;
		--index;
	}

	if (!elem) return NULL;
	return elem->data;
}

int SeibeList_getLength(SeibeList list)
{
	if (!list) return -1;
	return list->length;
}
