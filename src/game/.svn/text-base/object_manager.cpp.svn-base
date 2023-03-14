#include "StdAfx.h"
#include "object_manager.h"

#ifdef _DEBUG
#define LSQ_DEBUG
#endif // _DEBUG


struct ContainerTreeNodeS
{
	struct ContainerTreeNodeS *Parent;
	struct ContainerTreeNodeS *Left;
	struct ContainerTreeNodeS *Right;
	LSQ_BaseTypeT Data;
	LSQ_IntegerIndexT Key;
	int Balance;
};

typedef struct ContainerTreeNodeS ContainerTreeNodeT;

typedef struct
{
	ContainerTreeNodeT *Root;
	ContainerTreeNodeT *Last;
	ContainerTreeNodeT *First;
	int ElementsCount;
}ContainerT;

typedef enum
{
	ITERATOR_STATE_BEFORE_FIRST,
	ITERATOR_STATE_PAST_REAR,
	ITERATOR_STATE_NORMAL
}IteratorState;

typedef struct
{
	IteratorState State;
	ContainerT *Container;
	ContainerTreeNodeT *Node;
}IteratorT;

LSQ_HandleT LSQ_CreateSequence()
{
	ContainerT *newContainer = NULL;
	if ((newContainer = (ContainerT*)malloc(sizeof(ContainerT))) == NULL)
		return LSQ_HandleInvalid;
	newContainer->ElementsCount = 0;
	newContainer->First = newContainer->Last = newContainer->Root = NULL;
	return newContainer;
}

static void DestroyNext(ContainerTreeNodeT *NextNode)
{
	if (NextNode == NULL)
		return;
	if (NextNode->Left == NULL && NextNode->Right == NULL)
	{
		free(NextNode);
		return;
	}
	if (NextNode->Left != NULL)
		DestroyNext(NextNode->Left);
	if (NextNode->Right != NULL)
		DestroyNext(NextNode->Right);
	free(NextNode);
}

extern void LSQ_DestroySequence(LSQ_HandleT handle)
{
	ContainerT *container = (ContainerT*)handle;
	if (handle == NULL)
		return;
	DestroyNext(container->Root);
	free(container);
}

extern void LSQ_ClearSequence(LSQ_HandleT handle)
{
	ContainerT *container = (ContainerT*)handle;
	if (handle == NULL)
		return;
	DestroyNext(container->Root);
	container->ElementsCount = 0;
	container->First = container->Last = container->Root = NULL;
}

LSQ_IntegerIndexT LSQ_GetSize(LSQ_HandleT handle)
{
	ContainerT *container = (ContainerT*)handle;
	if (handle == NULL)
		return 0;
	return container->ElementsCount;
}

int LSQ_IsIteratorDereferencable(LSQ_IteratorT iterator)
{
	return !(LSQ_IsIteratorBeforeFirst(iterator) || LSQ_IsIteratorPastRear(iterator));
}

int LSQ_IsIteratorPastRear(LSQ_IteratorT iterator)
{
	IteratorT *iteratorl = (IteratorT*)iterator;
	if (iteratorl == NULL)
		return 0;
	return (iteratorl->State == ITERATOR_STATE_PAST_REAR);
}

int LSQ_IsIteratorBeforeFirst(LSQ_IteratorT iterator)
{
	IteratorT *iteratorl = (IteratorT*)iterator;
	if (iteratorl == NULL)
		return 0;
	return (iteratorl->State == ITERATOR_STATE_BEFORE_FIRST);
}

LSQ_BaseTypeT LSQ_DereferenceIterator(LSQ_IteratorT iterator)
{
	IteratorT *iteratorl = (IteratorT*)iterator;
	if (iteratorl == NULL)
		return LSQ_HandleInvalid;
	if (!LSQ_IsIteratorDereferencable(iterator))
		return NULL;
	return iteratorl->Node->Data;
}

LSQ_IntegerIndexT LSQ_GetIteratorKey(LSQ_IteratorT iterator)
{
	IteratorT *iteratorl = (IteratorT*)iterator;
	if (iteratorl == NULL)
		return 0;
	return iteratorl->Node->Key;
}

extern LSQ_IteratorT LSQ_GetElementByIndex(LSQ_HandleT handle, LSQ_IntegerIndexT index)
{
	ContainerT *container = (ContainerT*)handle;
	IteratorT *iterator = NULL;
	if (container == NULL)
		return LSQ_HandleInvalid;
	if ((iterator =  (IteratorT*)malloc(sizeof(IteratorT))) == NULL)
		return LSQ_HandleInvalid;
	iterator->Container = container;
	iterator->Node = container->Root;
	iterator->State = ITERATOR_STATE_NORMAL;
	while (iterator->Node != NULL)
	{
		if (iterator->Node->Key == index)
			return iterator;
		if (index < iterator->Node->Key)
			iterator->Node = iterator->Node->Left;
		else
			iterator->Node = iterator->Node->Right;
	}
	iterator->State = ITERATOR_STATE_PAST_REAR;
	return iterator;
}

LSQ_IteratorT LSQ_GetFrontElement(LSQ_HandleT handle)
{
	IteratorT *iterator = NULL;
	if (handle == NULL)
		return LSQ_HandleInvalid;
	if ((iterator = (IteratorT*)malloc(sizeof(IteratorT))) == NULL)
		return LSQ_HandleInvalid;
	iterator->Container = (ContainerT*)handle;
	iterator->Node = ((ContainerT*)handle)->First;
	iterator->State = iterator->Node == NULL ? ITERATOR_STATE_PAST_REAR : ITERATOR_STATE_NORMAL;
	return iterator;
}

void LSQ_SetFrontElement(LSQ_IteratorT iterator_handle)
{
	if (iterator_handle == NULL)
		return;
	IteratorT *iterator = (IteratorT*)iterator_handle;
	if (!iterator->Container)
	{
		iterator->State = ITERATOR_STATE_PAST_REAR;
		return;
	}
	iterator->Node = iterator->Container->First;
	iterator->State = iterator->Node == NULL ? ITERATOR_STATE_PAST_REAR : ITERATOR_STATE_NORMAL;
}

LSQ_IteratorT LSQ_GetPastRearElement(LSQ_HandleT handle)
{
	IteratorT *iterator = NULL;
	if (handle == NULL)
		return LSQ_HandleInvalid;
	if ((iterator = (IteratorT*)malloc(sizeof(IteratorT))) == NULL)
		return LSQ_HandleInvalid;
	iterator->Container = (ContainerT*)handle;
	iterator->Node = NULL;
	iterator->State = ITERATOR_STATE_PAST_REAR;
	return iterator;
}

void LSQ_DestroyIterator(LSQ_IteratorT iterator)
{
	free(iterator);
}

void LSQ_AdvanceOneElement(LSQ_IteratorT iterator)
{
	IteratorT *iteratorl = (IteratorT *)iterator;
	if (iteratorl == NULL)
		return;
	if (iteratorl->State == ITERATOR_STATE_BEFORE_FIRST)
	{
		if (iteratorl->Container->ElementsCount == 0)
		{
			iteratorl->State = ITERATOR_STATE_PAST_REAR;
			return;
		}
		iteratorl->State = ITERATOR_STATE_NORMAL;
		iteratorl->Node = iteratorl->Container->First;
		return;
	}
	if (iteratorl->State == ITERATOR_STATE_PAST_REAR)
		return;
	if (iteratorl->Node == iteratorl->Container->Last)
	{
		iteratorl->State = ITERATOR_STATE_PAST_REAR;
		return;
	}
	if (iteratorl->Node->Right != NULL)
	{
		iteratorl->Node = iteratorl->Node->Right;
		while (iteratorl->Node->Left != NULL)
			iteratorl->Node = iteratorl->Node->Left;
	}
	else
	{
		while (iteratorl->Node->Parent->Left != iteratorl->Node)
		{
			iteratorl->Node = iteratorl->Node->Parent;
		}
		iteratorl->Node = iteratorl->Node->Parent;
	}
}

void LSQ_RewindOneElement(LSQ_IteratorT iterator)
{
	IteratorT *iteratorl = (IteratorT *)iterator;
	if (iteratorl == NULL)
		return;
	if (iteratorl->State == ITERATOR_STATE_PAST_REAR)
	{
		if (iteratorl->Container->ElementsCount == 0)
		{
			iteratorl->State = ITERATOR_STATE_BEFORE_FIRST;
			return;
		}
		iteratorl->State = ITERATOR_STATE_NORMAL;
		iteratorl->Node = iteratorl->Container->Last;
		return;
	}
	if (iteratorl->State == ITERATOR_STATE_BEFORE_FIRST)
		return;
	if (iteratorl->Node == iteratorl->Container->First)
	{
		iteratorl->State = ITERATOR_STATE_BEFORE_FIRST;
		return;
	}
	if (iteratorl->Node->Left != NULL)
	{
		iteratorl->Node = iteratorl->Node->Left;
		while (iteratorl->Node->Right != NULL)
			iteratorl->Node = iteratorl->Node->Right;
	}
	else
	{
		while (iteratorl->Node->Parent->Right != iteratorl->Node)
		{
			iteratorl->Node = iteratorl->Node->Parent;
		}
		iteratorl->Node = iteratorl->Node->Parent;
	}
}
/* Функция, перемещающая итератор на заданное смещение со знаком */
void LSQ_ShiftPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT shift)
{
	int absshift = (shift > 0) ? shift : -shift;
	if (iterator == NULL)
		return;
	while (absshift != 0)
	{
		(shift > 0) ? LSQ_AdvanceOneElement(iterator) : LSQ_RewindOneElement(iterator);
		absshift--;
	}
}
/* Функция, устанавливающая итератор на элемент с указанным номером */
void LSQ_SetPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT pos)
{
	IteratorT *iteratorl = (IteratorT*)iterator;
	if (iteratorl == NULL)
		return;
	iteratorl->State = ITERATOR_STATE_BEFORE_FIRST;
	iteratorl->Node = NULL;
	LSQ_ShiftPosition(iterator, pos+1);
}

/* Обновляет потомка предка узла, при его замене в случае, если мы не знаем, левым был потомок или правым*/
static void UpdateParent(ContainerT *container, ContainerTreeNodeT *descendant,
						 ContainerTreeNodeT *newDescendant)
{
	if (descendant->Parent != NULL)
	{
		if (descendant->Parent->Left == descendant)
			descendant->Parent->Left = newDescendant;
		if (descendant->Parent->Right == descendant)
			descendant->Parent->Right = newDescendant;
	}
	else
	{
		container->Root = newDescendant;
	}
	newDescendant->Parent = descendant->Parent;
	/* Код который вызывал эту процедуру должен обновить предка старого
		потомка т.к. в разных случаях поворота он может быть разным, а тут мы его обнулим*/
	descendant->Parent = NULL;
}

static void RightRotation(ContainerT *container, ContainerTreeNodeT *root)
{
	ContainerTreeNodeT *pivot = root->Left;
	UpdateParent(container, root, pivot);
	root->Left = pivot->Right;
	if (root->Left != NULL)
		root->Left->Parent = root;
	pivot->Right = root;
	root->Parent = pivot;
 	pivot->Balance = 0;
 	root->Balance = 0;
}
static void LeftRotation(ContainerT *container, ContainerTreeNodeT *root)
{
	ContainerTreeNodeT *pivot = root->Right;
	UpdateParent(container, root, pivot);
	root->Right = pivot->Left;
	if (root->Right != NULL)
		root->Right->Parent = root;
	pivot->Left = root;
	root->Parent = pivot;
 	pivot->Balance = 0;
 	root->Balance = 0;
}

static void LeftRightRotation(ContainerT *container, ContainerTreeNodeT *root)
{
	ContainerTreeNodeT *pivot = root->Left, *bottom = pivot->Right;
	int OldBottomBalance = bottom->Balance;
	LeftRotation(container, root->Left);
	RightRotation(container, root);
	bottom->Balance = 0;
	switch (OldBottomBalance)
	{
	case 0:
		root->Balance = 0;
		pivot->Balance = 0;
		break;
	case 1:
		root->Balance = 0;
		pivot->Balance = -1;
		break;
	case -1:
		root->Balance = 1;
		pivot->Balance = 0;
		break;
	}
}

static void RightLeftRotation(ContainerT *container, ContainerTreeNodeT *root)
{
	ContainerTreeNodeT *pivot = root->Right, *bottom = pivot->Left;
	int OldBottomBalance = bottom->Balance;
	RightRotation(container, root->Right);
	LeftRotation(container, root);
	bottom->Balance = 0;
	switch (OldBottomBalance)
	{
	case 0:
		root->Balance = 0;
		pivot->Balance = 0;
		break;
	case 1:
		root->Balance = -1;
		pivot->Balance = 0;
		break;
	case -1:
		root->Balance = 0;
		pivot->Balance = 1;
		break;
	}
}




void LSQ_InsertElement(LSQ_HandleT handle, LSQ_IntegerIndexT key, LSQ_BaseTypeT value)
{
	ContainerT *container = (ContainerT*)handle;
	ContainerTreeNodeT *NewNode = NULL, *TempNode = NULL, *ParentNode = NULL, *PrevNode = NULL;
	if (container == NULL)
		return;
	TempNode = container->Root;

	// Поиск, существует ли элемент с данным ключом.
	// Одновременно, ищем, куда можно вставить новый элемент.
	while (TempNode != NULL)
	{
		if (TempNode->Key == key)
		{
			// Элемент найден, обновляем значение.
			TempNode->Data = value;
			return;
		}
		ParentNode = TempNode;
		if (key < TempNode->Key)
			TempNode = TempNode->Left;
		else
			TempNode = TempNode->Right;
	}

	// Элемент с данным ключом не существует, создаем новый.
	NewNode = (ContainerTreeNodeT*)malloc(sizeof(ContainerTreeNodeT));
	if (NewNode == NULL)
		return;
 	NewNode->Key = key;
 	NewNode->Data = value;
 	NewNode->Balance = 0;
 	NewNode->Left = NULL;
 	NewNode->Right = NULL;
 	NewNode->Parent = ParentNode;

	// Вставка элемента
 	if (container->ElementsCount == 0)
	{
		container->First = container->Last = container->Root = NewNode;
		container->ElementsCount++;
		return;
	}
	if (key < ParentNode->Key)
	{
		ParentNode->Left = NewNode;
		if (container->First == ParentNode)
			container->First = NewNode;
	}
	else
	{
		if (container->Last == ParentNode)
			container->Last = NewNode;
		ParentNode->Right = NewNode;
	}

	// Балансировка дерева
	PrevNode = NewNode;
	TempNode = NewNode->Parent;
	while (TempNode != NULL)
	{
		if (TempNode->Left == PrevNode)
			TempNode->Balance--;
		if (TempNode->Right == PrevNode)
			TempNode->Balance++;

		if (TempNode->Balance == -2)
		{
			if (TempNode->Left != NULL && TempNode->Left->Balance == 1)
			{
				LeftRightRotation(container, TempNode);
				if (TempNode->Balance != 0)
					TempNode->Parent->Balance--;
			}
			else
				RightRotation(container, TempNode);

		}
		if (TempNode->Balance == 2)
		{
			if (TempNode->Right != NULL && TempNode->Right->Balance == -1)
			{
				RightLeftRotation(container, TempNode);
				if (TempNode->Balance != 0)
					TempNode->Parent->Balance++;
			}
			else
				LeftRotation(container, TempNode);

		}
		if (TempNode->Balance == 0)
			break;
		PrevNode = TempNode;
		TempNode = TempNode->Parent;
	}
	container->ElementsCount++;
}

void LSQ_InsertNewMaxElement(LSQ_HandleT handle, LSQ_IntegerIndexT key, LSQ_BaseTypeT value)
{
	ContainerT *container = (ContainerT*)handle;
	ContainerTreeNodeT *NewNode = NULL, *TempNode = NULL, *ParentNode = NULL, *PrevNode = NULL;
	if (container == NULL)
		return;
	TempNode = container->Root;

	// Поиск места, куда можно вставить новый элемент. Это всегда правая ветвь,
	// т.к. мы вставляем элемент с ключом, заведомо большим, чем все ключи в дереве.
	while (TempNode != NULL)
	{
		ParentNode = TempNode;
		TempNode = TempNode->Right;
	}

	// Создаем новый ключ.
	NewNode = (ContainerTreeNodeT*)malloc(sizeof(ContainerTreeNodeT));
	if (NewNode == NULL)
		return;
 	NewNode->Key = key;
 	NewNode->Data = value;
 	NewNode->Balance = 0;
 	NewNode->Left = NULL;
 	NewNode->Right = NULL;
 	NewNode->Parent = ParentNode;

	// Вставка эдемента
 	if (container->ElementsCount == 0)
	{
		container->First = container->Last = container->Root = NewNode;
		container->ElementsCount++;
		return;
	}
	if (container->Last == ParentNode)
		container->Last = NewNode;
	ParentNode->Right = NewNode;

	// Балансировка дерева
	PrevNode = NewNode;
	TempNode = NewNode->Parent;
	while (TempNode != NULL)
	{
		if (TempNode->Left == PrevNode)
			TempNode->Balance--;
		if (TempNode->Right == PrevNode)
			TempNode->Balance++;

		if (TempNode->Balance == -2)
		{
			if (TempNode->Left != NULL && TempNode->Left->Balance == 1)
			{
				LeftRightRotation(container, TempNode);
				if (TempNode->Balance != 0)
					TempNode->Parent->Balance--;
			}
			else
				RightRotation(container, TempNode);

		}
		if (TempNode->Balance == 2)
		{
			if (TempNode->Right != NULL && TempNode->Right->Balance == -1)
			{
				RightLeftRotation(container, TempNode);
				if (TempNode->Balance != 0)
					TempNode->Parent->Balance++;
			}
			else
				LeftRotation(container, TempNode);

		}
		if (TempNode->Balance == 0)
			break;
		PrevNode = TempNode;
		TempNode = TempNode->Parent;
	}
	container->ElementsCount++;
}

void LSQ_DeleteFrontElement(LSQ_HandleT handle)
{
	ContainerT *container = (ContainerT *)handle;
	if (container == NULL)
		return;
	LSQ_DeleteElement(handle, container->First->Key);
}

void LSQ_DeleteRearElement(LSQ_HandleT handle)
{
	ContainerT *container = (ContainerT *)handle;
	if (container == NULL)
		return;
	LSQ_DeleteElement(handle, container->Last->Key);
}

//static ContainerTreeNodeT** GetReferenceFromParent(ContainerTreeNodeT *SourceNode)
//{
//	if (SourceNode->Parent == NULL)
//		return NULL;
//	if (SourceNode == SourceNode->Parent->Left)
//		return &(SourceNode->Parent->Left);
//	if (SourceNode == SourceNode->Parent->Right)
//		return &(SourceNode->Parent->Right);
//	return NULL;
//}

static void SwapNodes(ContainerTreeNodeT *One, ContainerTreeNodeT *Other)
{
	LSQ_BaseTypeT Temp = One->Data;
	One->Data = Other->Data;
	Other->Data = Temp;

	LSQ_IntegerIndexT Temp2;
	Temp2 = One->Key;
	One->Key = Other->Key;
	Other->Key = Temp2;
}

void LSQ_DeleteElement(LSQ_HandleT handle, LSQ_IntegerIndexT key)
{
	ContainerT *container = (ContainerT*)handle;
	ContainerTreeNodeT *TempNode = NULL, *PrevNode = NULL, *DeletedNode = NULL;
	IteratorT *iterator = NULL;
	if (container == NULL)
		return;
	iterator = (IteratorT*)LSQ_GetElementByIndex(handle, key);
	if (iterator == NULL || iterator->State == ITERATOR_STATE_PAST_REAR)
	{
		LSQ_DestroyIterator(iterator);
		return;
	}

	while (iterator->Node->Left != NULL || iterator->Node->Right != NULL)
	{
		TempNode = iterator->Node;
		if (iterator->Node->Balance == 1)
			LSQ_AdvanceOneElement(iterator);
		else
			LSQ_RewindOneElement(iterator);
		SwapNodes(TempNode, iterator->Node);
	}
	if (iterator->Node->Left == NULL && iterator->Node->Right == NULL)
	{
		if (iterator->Node == container->First)
		{
			LSQ_AdvanceOneElement(iterator);
			TempNode = iterator->Node;
			LSQ_RewindOneElement(iterator);
			container->First = TempNode;
		}
		if (iterator->Node == container->Last)
		{
			LSQ_RewindOneElement(iterator);
			TempNode = iterator->Node;
			LSQ_AdvanceOneElement(iterator);
			container->Last = TempNode;
		}
		PrevNode = DeletedNode = iterator->Node;
		TempNode = iterator->Node->Parent;
		free(iterator->Node);
		LSQ_DestroyIterator(iterator);
	}
	while (TempNode != NULL)
	{

		if (TempNode->Left == PrevNode)
			TempNode->Balance++;
		if (TempNode->Left == DeletedNode)
			TempNode->Left = NULL;
		if (TempNode->Right == PrevNode)
			TempNode->Balance--;
		if (TempNode->Right == DeletedNode)
			TempNode->Right = NULL;


		if (abs(TempNode->Balance) == 1)
			break;

		if (TempNode->Balance == -2)
		{
			if (TempNode->Left != NULL && TempNode->Left->Balance == 1)
			{
				LeftRightRotation(container, TempNode);
				TempNode->Parent->Balance++;
			}
			else
			{
				int OldPivotBalance = TempNode->Left->Balance;
				RightRotation(container, TempNode);
				if (abs(OldPivotBalance) != 1)
				{
					TempNode->Balance = -1;
					TempNode->Parent->Balance = 1;
				}
				TempNode->Parent->Balance++;
			}
		}

		if (TempNode->Balance == 2)
		{
			if (TempNode->Right != NULL && TempNode->Right->Balance == -1)
			{
				RightLeftRotation(container, TempNode);
				TempNode->Parent->Balance--;
			}
			else
			{
				int OldPivotBalance = TempNode->Right->Balance;
				LeftRotation(container, TempNode);
				if (abs(OldPivotBalance) != 1)
				{
					TempNode->Balance = 1;
					TempNode->Parent->Balance = -1;
				}
				TempNode->Parent->Balance--;
			}
		}
		PrevNode = TempNode;
		TempNode = TempNode->Parent;
	}
	container->ElementsCount--;
	if (container->ElementsCount == 0)
	{
		container->First = container->Last = container->Root = NULL;
	}
}


// Отладочные функции проверки дерева.
#ifdef LSQ_DEBUG

#undef MAX	// MAX был уже определен в IceTypes.h как макрос
static __inline int MAX(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}

int CheckBalance(ContainerTreeNodeT *NextNode)
{
	if (NextNode == NULL)
		return 0;
	if (NextNode->Left == NULL && NextNode->Right == NULL)
	{
		assert(NextNode->Balance == 0);
		return 1;
	}
	{
		int leftsubheight = CheckBalance(NextNode->Left);
		int rightsubheight = CheckBalance(NextNode->Right);
		assert(rightsubheight - leftsubheight == NextNode->Balance);
		return MAX(leftsubheight, rightsubheight) + 1;
	}
}

void CheckTree(LSQ_HandleT handle)
{
	ContainerT *contaier = (ContainerT*)handle;
	if (handle == LSQ_HandleInvalid)
		return;
	CheckBalance(contaier->Root);
}

#endif // LSQ_DEBUG
