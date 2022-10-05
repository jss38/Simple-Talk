#include <stdio.h>
#include <assert.h>
#include "list.h"

static List heads[LIST_MAX_NUM_HEADS];
static Node nodes[LIST_MAX_NUM_NODES];

static List freeList;
static bool freeListInit = false;

static void addHeadToFreeList(List* freeHead) {
    freeHead->nextList = NULL;
    
    if(freeList.headList == NULL) {
        freeList.headList = freeHead;
        freeList.tailList = freeHead;
        return;
    }

    (freeList.tailList)->nextList = freeHead;
    freeList.tailList = freeHead;
    
    return;
}

static void addNodeToFreeList(Node* freeNode) {
    freeNode->next = NULL;
    freeNode->prev = NULL;
    freeNode->data = NULL;
    
    if(freeList.head == NULL) {
        freeList.head = freeNode;
        freeList.tail = freeNode;
        return;
    }

    (freeList.tail)->next = freeNode;
    freeList.tail = freeNode;
    
    return;
}

static void initFreeList() {
    for(int i = 0; i < LIST_MAX_NUM_HEADS - 1; i++) {
        heads[i].nextList = &heads[i + 1];
    }

    freeList.headList = &heads[0];
    freeList.tailList = &heads[LIST_MAX_NUM_HEADS - 1];
    
    for(int j = 0; j < LIST_MAX_NUM_NODES - 1; j++) {
        nodes[j].next = &nodes[j + 1];
    }

    freeList.head = &nodes[0];
    freeList.tail = &nodes[LIST_MAX_NUM_NODES - 1];
}

List* List_create() {
    if(!freeListInit) {
        initFreeList();
        freeListInit = true;
    }

    if(freeList.headList == NULL) {
        return NULL;
    }
    
    List* newList = freeList.headList;

    newList->head = NULL;
    newList->tail = NULL;
    
    newList->curr = NULL;
    newList->LIST_OOB_STATUS = LIST_OOB_END;

    freeList.headList = (freeList.headList)->nextList;
    newList->nextList = NULL;

    newList->count = 0;
    return newList;
}

int List_count(List* pList) {
    return pList->count;
}

void* List_first(List* pList) {
    if(pList->head == NULL) {
        pList->curr = NULL;
        return pList->curr;
    }
    
    pList->curr = pList->head;
    
    return pList->curr->data;
}

void* List_last(List* pList) {
    if(pList->tail == NULL) {
        pList->curr = NULL;
        return pList->curr;
    }
    
    pList->curr = pList->tail;

    return pList->curr->data;
} 

void* List_next(List* pList) {
    if(pList->curr == NULL) {
        if(pList->LIST_OOB_STATUS == LIST_OOB_END) {
            return NULL;
        }

        pList->curr = pList->head;
        return pList->curr->data;
    }
    
    pList->curr = pList->curr->next;

    if(pList->curr == NULL) {
        pList->LIST_OOB_STATUS = LIST_OOB_END;
        return NULL;
    }

    return pList->curr->data;
}

void* List_prev(List* pList) {
    if(pList->curr == NULL) {
        if(pList->LIST_OOB_STATUS == LIST_OOB_START) {
            return NULL;
        }
        
        pList->curr = pList->tail;
        return pList->curr->data;
    }
    
    pList->curr = pList->curr->prev;

    if(pList->curr == NULL) {
        pList->LIST_OOB_STATUS = LIST_OOB_START;
        return NULL;
    }

    return pList->curr->data;
}

void* List_curr(List* pList) {
    if(pList->curr == NULL) {
        return NULL;
    }
    
    return pList->curr->data;
}


int List_insert_after(List* pList, void* pItem) {
    if(freeList.head == NULL) {
        return -1;
    }

    if(pList->curr == NULL) {
        if(pList->LIST_OOB_STATUS == LIST_OOB_START) {
            return List_prepend(pList, pItem);
        }
        return List_append(pList, pItem);
    }

    if(pList->curr->next == NULL) {
        return List_append(pList, pItem);
    }

    Node* newNode = freeList.head;
    
    freeList.head = (freeList.head)->next;
        
    newNode->data = pItem;

    pList->curr->next->prev = newNode;
    newNode->next = pList->curr->next;

    pList->curr->next = newNode;
    newNode->prev = pList->curr;

    pList->curr = newNode;

    pList->count += 1;
    return 0;
}

int List_insert_before(List* pList, void* pItem) {
    if(freeList.head == NULL) {
        return -1;
    }

    if(pList->curr == NULL) {
        if(pList->LIST_OOB_STATUS == LIST_OOB_START) {
            return List_prepend(pList, pItem);
        }
        return List_append(pList, pItem);
    }

    if(pList->curr->prev == NULL) {
        return List_prepend(pList, pItem);
    }

    Node* newNode = freeList.head;
    
    freeList.head = (freeList.head)->next;
        
    newNode->data = pItem;

    pList->curr->prev->next = newNode;
    newNode->prev = pList->curr->prev;

    pList->curr->prev = newNode;
    newNode->next = pList->curr;

    pList->curr = newNode;

    pList->count += 1;
    return 0;
}

int List_append(List* pList, void* pItem) {
    if(freeList.head == NULL) {
        return -1;
    }
    
    if(pList->head == NULL) {
       Node* newNode = freeList.head;
    
       freeList.head = (freeList.head)->next;
       
       newNode->data = pItem;
       
       pList->head = newNode;
       pList->tail = newNode;
       pList->curr = newNode;

       newNode->next = NULL;
       
       pList->count += 1;
       return 0;
    }

    Node* newNode = freeList.head;
    
    freeList.head = (freeList.head)->next;
        
    newNode->data = pItem;
    
    newNode->prev = pList->tail;
    pList->tail->next = newNode;

    pList->tail = newNode;
    pList->curr = newNode;

    newNode->next = NULL;
        
    pList->count += 1;
    return 0;
}

int List_prepend(List* pList, void* pItem) {
    if(freeList.head == NULL) {
        return -1;
    }

    if(pList->head == NULL) {
       Node* newNode = freeList.head;
    
       freeList.head = (freeList.head)->next;
       
       newNode->data = pItem;
       
       pList->head = newNode;
       pList->tail = newNode;
       pList->curr = newNode;

       newNode->next = NULL;
       
       pList->count += 1;
       return 0;
    }
    
    Node* newNode = freeList.head;
    
    freeList.head = (freeList.head)->next;

    newNode->data = pItem;    
    
    newNode->next = pList->head;
    pList->head->prev = newNode;

    pList->head = newNode;
    pList->curr = newNode;
    
    newNode->prev = NULL;

    pList->count += 1;
    return 0;
}

void* List_remove(List* pList) {
    if(pList->head == NULL || pList->curr == NULL) {
        return NULL;
    }
    
    Node* currItem = pList->curr;
    void* currItemData = currItem->data;
    
    if(pList->curr == pList->head) {
        if(pList->count == 1) {
            pList->head = NULL;
            pList->curr = NULL;
            pList->tail = NULL;

            addNodeToFreeList(currItem);

            pList->count = 0;
            return currItemData;
        }
        
        
        pList->head = pList->head->next;
        pList->head->prev->next = NULL;
        pList->head->prev = NULL;

        pList->curr = pList->head;
        
        addNodeToFreeList(currItem);

        pList->count -= 1;
        return currItemData;
    }

    if(pList->curr == pList->tail) {
        pList->tail = pList->tail->prev;
        pList->tail->next->prev = NULL;
        pList->tail->next = NULL;

        pList->curr = pList->tail->next;
        pList->LIST_OOB_STATUS = LIST_OOB_END;
        
        addNodeToFreeList(currItem);
        
        pList->count -= 1;
        return currItemData;
    }
    
    pList->curr->prev->next = pList->curr->next;
    pList->curr->next->prev = pList->curr->prev;
    
    pList->curr = pList->curr->next;

    addNodeToFreeList(currItem);
    
    pList->count -= 1;
    return currItemData;
}

void* List_trim(List* pList) {
    pList->curr = pList->tail;
    
    void* currItemData = List_remove(pList);

    // If calling List_remove(pList) removed the only node in pList
    // Then the count should be 0
    // And the currItemData should be returned
    if(pList->count == 0) {
        return currItemData;
    }
    
    pList->curr = pList->tail;
    
    pList->count -= 1;
    return currItemData;
}

void List_concat(List* pList1, List* pList2) {
    if(pList1->head == NULL) {
        if(pList2->head != NULL) {
            pList1->head = pList2->head;
            pList1->tail = pList2->tail;
            addHeadToFreeList(pList2);
        }
        addHeadToFreeList(pList2);
        return;
    }

    if(pList2->head == NULL) {
        addHeadToFreeList(pList2);
        return;
    }
    
    pList1->tail->next = pList2->head;
    pList2->head->prev = pList1->tail;
    pList1->tail = pList2->tail;
    pList2->tail = NULL;
    
    addHeadToFreeList(pList2);
    
    pList2->head = NULL;

    pList1->count = pList1->count + pList2->count;
}

void List_free(List* pList, FREE_FN pItemFreeFn) {
    if(pList->head == NULL) {
        addHeadToFreeList(pList);
        pList->count = 0;
        return;
    }

    if(pList->head->next == NULL) {
        (*pItemFreeFn)(pList->head->data);
        addHeadToFreeList(pList);
        pList->count = 0;
        return;
    }
    
    Node* ptr = pList->head->next;

    (*pItemFreeFn)(pList->head->data);
    addHeadToFreeList(pList);

    while(ptr != NULL) {
        (*pItemFreeFn)(ptr->data);
        addNodeToFreeList(ptr);
        ptr = ptr->next;
    }

    pList->count = 0;
}


void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    if(pList->curr == NULL && pList->LIST_OOB_STATUS == LIST_OOB_START) {
        pList->curr = pList->head;
    }
    
    while(pList->curr != NULL) {
        if(pComparator(pList->curr->data, pComparisonArg)) {
            return pList->curr->data;
        }
        
        pList->curr = pList->curr->next;
    }
    
    pList->LIST_OOB_STATUS = LIST_OOB_END;
    
    return pList->curr;
}