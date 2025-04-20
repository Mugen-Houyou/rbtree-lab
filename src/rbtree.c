#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>


node_t *minBySubtree(rbtree *t, node_t *z){
  node_t *result;
  result = z;

  while (result->left != t->nil)
    result = result->left;

  return result;
}

node_t *maxBySubtree(rbtree *t, node_t *z){
  node_t *result;
  result = z;

  while (result->right != t->nil)
    result = result->right;

  return result;
}

rbtree *new_rbtree(void){
  rbtree *root;

  printf("now in rbtree;");
  // root 생성
  root = calloc(1, sizeof(rbtree));
  if (root == NULL)
    return NULL;

  // root의 nil 생성, 색칠
  root->nil = calloc(1, sizeof(node_t));
  if (root->nil == NULL){
    free(root);
    return NULL;
  }
  root->nil->color = RBTREE_BLACK;

  // 자기자신, 부모, 자식 전부 다 nil로.
  root->root = root->nil;
  root->root->parent = root->nil;
  root->root->left = root->nil;
  root->root->right = root->nil;

  return root;
}

void rbtreeTransplant(rbtree *t, node_t *old, node_t *new){
  if (old->parent == t->nil)
    t->root = new;
  else if (old == old->parent->left)
    old->parent->left = new;
  else
    old->parent->right = new;

  new->parent = old->parent;
}

void rotateLeft(rbtree *t, node_t **axis){
  node_t *y;

  y = (*axis)->right;
  (*axis)->right = y->left;

  if (y->left != t->nil)
    y->left->parent = *axis;

  y->parent = (*axis)->parent;

  if ((*axis)->parent == t->nil)
    t->root = y;
  else if ((*axis) == (*axis)->parent->left)
    (*axis)->parent->left = y;
  else
    (*axis)->parent->right = y;

  y->left = (*axis);
  (*axis)->parent = y;
}

void rotateRight(rbtree *t, node_t **axis){
  node_t *y;

  y = (*axis)->left;
  (*axis)->left = y->right;

  if (y->right != t->nil)
    y->right->parent = *axis;

  y->parent = (*axis)->parent;

  if ((*axis)->parent == t->nil)
    t->root = y;
  else if ((*axis) == (*axis)->parent->left)
    (*axis)->parent->left = y;
  else
    (*axis)->parent->right = y;

  y->right = (*axis);
  (*axis)->parent = y;
}

void removeAllNodes(rbtree *t, node_t **node){
  // 주의: 이건 노드들만. RBT 자체와 t->nil은 밖에서 free()할 것.
  if (*node == NULL || *node == t->nil)
    return;

  removeAllNodes(t, &((*node)->left));
  removeAllNodes(t, &((*node)->right));
  free(*node);
  *node = NULL;
}

node_t *newNode(rbtree *t, const key_t key){
  node_t *newChild;

  newChild = (node_t *)calloc(1, sizeof(node_t));
  if (newChild == NULL)
    return NULL;

  newChild->key = key;
  newChild->left = t->nil;
  newChild->right = t->nil;
  newChild->parent = t->nil;
  newChild->color = (t->root == t->nil) ? RBTREE_BLACK : RBTREE_RED;

  return newChild;
}

void restoreEraseOrder(rbtree *t, node_t *x){
  node_t *w, *y, *uncle, *me;

  me = x;
  x = NULL;

  // 대상이 NIL면 돌아가 (필요없으니까)
  if (me == t->nil)
    return;

  for (;;){
    // 대상이 흑색 루트면 돌아가 (필요없으니까)
    if (me == t->root && me->color == RBTREE_BLACK)
      break;

    if (me == me->parent->left){ // 대상이 왼쪽 자식
      w = me->parent->right;
      if (w->color == RBTREE_RED){
        w->color = RBTREE_BLACK;
        me->parent->color = RBTREE_RED;
        rotateLeft(t, me->parent);
        w = me->parent->right;
      }

      if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK){
        w->color = RBTREE_RED;
        me = me->parent;
      }else{
        if (w->right->color == RBTREE_BLACK){
          w->left->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          rotateRight(t, w);
          w = me->parent->right;
        }
        w->color = me->parent->color;
        me->parent->color = RBTREE_BLACK;
        w->right->color = RBTREE_RED;
        rotateLeft(t, me->parent);
        me = t->root;
      }
    }else{ // 대상이 오른쪽 자식

    }
  }
  // 루트를 흑색으로
  t->root->color = RBTREE_BLACK;
}

void restoreInsertOrder(rbtree *t, node_t *z){
  int infLoop;
  node_t *me, *uncle, *grandp; // 문장 너무 길어짐

  me = z;

  for (;;){
    // 부모가 없으면 돌아가 (필요없으니까)
    if (me->parent == NULL || me->parent == t->nil)
      break;

    // 부모가 블랙이면 돌아가 (필요없으니까)
    if (me->parent->color == RBTREE_BLACK)
      break;

    // 부모가 루트면 (or 조부모 없으면) 블랙으로 칠하고 끝
    if (me->parent == t->root || me->parent->parent == t->nil){
      me->parent->color = RBTREE_BLACK;
      break;
    }

    // 얼리 리턴 끝, 조부모 및 삼촌 지정
    grandp = me->parent->parent;
    uncle = (me->parent == grandp->left) ? grandp->right : grandp->left;

    // 케이스 1. 부모, 삼촌이 모두 RED - 좌측 버전
    if (me->parent->color == RBTREE_RED && uncle->color == RBTREE_RED){
      me->parent->color = RBTREE_BLACK;
      uncle->color = RBTREE_BLACK;
      grandp->color = RBTREE_RED;
      me = grandp; // 조부모로 올라감 - 이유는 R-R을 R-B로 바꿨으니 굳이 부모 볼 이유가 없음
      continue;    // 이걸로 삼촌은 무.적.권. BLACK.
    }

    // 케이스 2, 3. 부모는 RED, 숙부는 BLACK, 삼각형 형태
    if (me->parent == grandp->left){ // Left‑Right 삼각형 => Left‑Left 일자
      if (me == me->parent->right){
        me = me->parent;
        rotateLeft(t, &me);
      }
      me->parent->color = RBTREE_BLACK;
      grandp->color = RBTREE_RED;
      rotateRight(t, &grandp);
    }
    else{ // Right‑Left 삼각형 => Right‑Right 일자
      if (me == me->parent->left){
        me = me->parent;
        rotateRight(t, &me);
      }
      me->parent->color = RBTREE_BLACK;
      grandp->color = RBTREE_RED;
      rotateLeft(t, &grandp);
    }

    // 마지막으로 R-R여부 체크 후 continue/break 결정
    if (me->parent->color == RBTREE_RED)
      continue;
    else
      break;
  }
  // 루트를 흑색으로
  t->root->color = RBTREE_BLACK;
}
/*
  "구현하는 ADT가 multiset이므로 이미 같은 key의 값이 존재해도 하나 더 추가합니다."
  즉 중복 허용.
*/
node_t *rbtree_insert(rbtree *t, const key_t key){
  node_t *child, *parent, *insertPos, *parentPrev;

  parent = t->nil;     // 부모 후보
  insertPos = t->root; // 탐색용
  // parent_prev = parent;

  // 넣을 위치 찾기
  while (insertPos != t->nil)
  {
    parent = insertPos;

    if (key < insertPos->key)
      insertPos = insertPos->left;
    else
      insertPos = insertPos->right; // 같은 키면 일단 오른쪽 ㄱㄱ

    // parent_prev = insert_pos;
  }

  child = newNode(t, key); // 넣을 자식 노드 준비

  // 넣기
  if (parent == t->nil)
    t->root = child; // 루트가 없음
  else if (child->key < parent->key)
    parent->left = child; // 왼쪽에
  else
    parent->right = child; // 오른쪽에

  child->parent = parent;

  restoreInsertOrder(t, child);

  return child;
}

void delete_rbtree(rbtree *t){
  if (t == NULL)
    return;
  removeAllNodes(t, &(t->root));
  free(t->nil);
  free(t);
  t->root = NULL;
  t->nil = NULL;
}

node_t *rbtree_find(const rbtree *t, const key_t key){
  node_t *currNode;

  currNode = t->root; // 루트부터

  // 탐색
  while (currNode != t->nil) {
    if (key < currNode->key)
      currNode = currNode->left;
    else if (key > currNode->key)
      currNode = currNode->right;
    else if (key == currNode->key)
      return currNode;
    else
      break;
  }

  return NULL;
}

node_t *rbtree_min(const rbtree *t){
  node_t *result;
  result = t->root;

  while (result->left != t->nil)
    result = result->left;

  return result;
}

node_t *rbtree_max(const rbtree *t){
  node_t *result;
  result = t->root;

  while (result->right != t->nil)
    result = result->right;

  return result;
}

int rbtree_erase(rbtree *t, node_t *p){
  node_t *x, *y;
  color_t yOrigColor;

  y = p;
  yOrigColor = y->color;

  // 삭제하려는 노드의 자녀(NIL제외)가 0개 또는 1개라면 => "삭제되는 색 = 삭제되는 노드의 색"
  if (p->left == t->nil){ // 자녀가 1 이하 (적어도 왼쪽은 없음)
    x = p->right;
    rbtreeTransplant(t, p, p->right);

  } else if (p->right == t->nil){ // 자녀가 1 (오른쪽 없음)
    x = p->left; 
    rbtreeTransplant(t, p, p->left);

  } else { // 자녀가 2
    y = minBySubtree(t, p->right);
    yOrigColor = p->color;
    x = y->right;

    if (y != p->right) {
      rbtreeTransplant(t, y, y->right);
      y->right = p->right;
      y->right->parent = y;
    } else {
      x->parent = y;
    }

    rbtreeTransplant(t, p, y);
    y->left = p->left;
    y->left->parent = y;
    y->color = p->color;
  }

  if (yOrigColor == RBTREE_BLACK)
    restoreEraseOrder(t, x);

  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n){
  // TODO: implement to_array
  return 0;
}
