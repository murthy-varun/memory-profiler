/*
MIT License

Copyright (c) 2019 Varun Murthy (varun.tk@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stddef.h>
#include "linked_list.h"

void list_insert(list_node_t **head, list_node_t *node)
{
	if(!head) {
		return;
	}

	/* Always insert at head */
    node->next = *head;
	*head = node;
	return;
}

void* list_delete(list_node_t **head, void *key)
{
	list_node_t *current = NULL;
	
	if(!head) {
		return NULL;
	}

	current = *head;

	/* special case for head */
	if (current != NULL && current->key == key) {
        *head = current->next;
    }
	else {	
		list_node_t *prev = NULL;
		while (current != NULL && current->key != key) {
			prev = current;
			current = current->next;
		}

		if (current) {
			prev->next = current->next;
		}
	}

	return current;
}

void* list_find(list_node_t *head, void *key)
{
	list_node_t  *current = head;
	void         *ret = NULL;

	while (current != NULL && current->key != key) {
		current = current->next;
	}

	if (current) {
		ret = &current->val;
	}	

	return ret;
}
