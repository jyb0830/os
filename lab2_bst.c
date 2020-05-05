/*
*	Operating System Lab
*	    Lab2 (Synchronization)
*	    Student id : 32164491, 32164618
*	    Student name : 지영본, 최석민
*
*   lab2_bst.c :
*       - thread-safe bst code.
*       - coarse-grained, fine-grained lock code
*
*   Implement thread-safe bst for coarse-grained version and fine-grained version.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "lab2_sync_types.h"

pthread_mutex_t cg_insert = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cg_remove = PTHREAD_MUTEX_INITIALIZER;

void print_inorder(lab2_node *node){
    if(node != NULL){
        print_inorder(node->left);
        printf(" %d ", node->key);
        print_inorder(node->right);
    }
}

int lab2_node_print_inorder(lab2_tree *tree) {
    if(tree == NULL)
        return -1;
    else{
        //print_inorder(tree->root);
        //printf("\n");
        return 0;
    }
}

lab2_tree *lab2_tree_create() {
    lab2_tree *tree;
    tree = malloc(sizeof(lab2_tree));
    tree->root = NULL;
    return tree;
}

lab2_node * lab2_node_create(int key) {
    lab2_node *node;
    node = malloc(sizeof(lab2_node));
    pthread_mutex_t tmp = PTHREAD_MUTEX_INITIALIZER;
    node->mutex = tmp;
    node->left = NULL;
    node->right = NULL;
    node->key = key;
    return node;
}

int lab2_node_insert(lab2_tree *tree, lab2_node *new_node){
    if(tree==NULL || new_node==NULL){                                   //인자인 트리나 new node가 비어있으면
        return -1;
    }
    else{                               
        if(tree->root == NULL){                                         //트리의 root 노드가 비어있으면
            tree->root = new_node;
            return 0;
        }
        else{                                                           //트리가 비어있지 않으면
            lab2_node *c_node = tree->root, *parent = tree->root;
            int lr=0;
            while(c_node != NULL){                                      //삽입할 노드의 위치를 찾기 위한 for문
                if(c_node->key == new_node->key)                        //새로 삽입하는 노드의 키 값이 이미 존재하는 경우
                    return -1;
                else if(c_node->key > new_node->key){
                    parent = c_node;
                    c_node = c_node->left;
                    lr=0;
                }
                else{
                    parent = c_node;
                    c_node = c_node->right;
                    lr=1;
                }
            }
                if(lr==0) parent->left = new_node;                      //lr이 0이면 왼쪽에 삽입
                else parent->right = new_node;                          //lr이 1이면 오른쪽에 삽입
                return 0;
        }
    }
}

int lab2_node_insert_fg(lab2_tree *tree, lab2_node *new_node){  
    start: 
     if(tree==NULL || new_node==NULL){                                   //인자가 NULL이면
        return -1;
    }
    else{                               
        if(tree->root == NULL){                                         //트리의 root가 NULL이면
            if(pthread_mutex_lock(&cg_insert) != 0){                    //Lock이 걸려있으면
                goto start;                                             //start로 돌아가서 다시 Lock을 확인(Lock으로 설정된 영역이
            }                                                           //짧기 때문에 계속 ready 상태를 유지하기 위함)
            tree->root = new_node;
            pthread_mutex_unlock(&cg_insert);
            return 0;
        }
        else{                                                           //트리가 비어있지 않으면
            lab2_node *c_node = tree->root, *parent = tree->root;
            int lr=0;
            while(c_node != NULL){                                     //삽입할 위치를 찾기 위한 for 문
                if(c_node->key == new_node->key)                       //삽입하려는 키 값이 이미 존재하는 경우
                    return -1;
                else if(c_node->key > new_node->key){
                    parent = c_node;
                    c_node = c_node->left;
                    lr=0;
                }
                else{
                    parent = c_node;
                    c_node = c_node->right;
                    lr=1;
                }
            }
            if(pthread_mutex_lock(&(parent->mutex)) != 0){              //Lock이 걸려있으면
                goto start;                                             //start로 돌아가 다시 Lock을 검사
            }
            if(lr==0) parent->left = new_node;
            else parent->right = new_node;
            pthread_mutex_unlock(&(parent->mutex));
            return 0;
            }
        } 
}

int lab2_node_insert_cg(lab2_tree *tree, lab2_node *new_node){
    pthread_mutex_lock(&cg_insert);
    if(tree==NULL || new_node==NULL){
        return -1;
    }
    else{  
        if(tree->root == NULL){
            tree->root = new_node;
            pthread_mutex_unlock(&cg_insert);
            return 0;
        }
        else{
            lab2_node *c_node = tree->root, *parent = tree->root;
            int lr=0;
            while(c_node != NULL){
                if(c_node->key == new_node->key){
                    pthread_mutex_unlock(&cg_insert);
                    return -1;
                }
                else if(c_node->key > new_node->key){
                    parent = c_node;
                    c_node = c_node->left;
                    lr=0;
                }
                else{
                    parent = c_node;
                    c_node = c_node->right;
                    lr=1;
                }
            }
                if(lr==0) parent->left = new_node;
                else parent->right = new_node; 
                pthread_mutex_unlock(&cg_insert);
                return 0;
            }
        }
} 

int lab2_node_remove(lab2_tree *tree, int key) {  
    lab2_node *c_node, *parent;
    int lr=0;
    if(tree == NULL){                                                //트리가 NULL이면
        return -1;
    }
    else if(tree->root == NULL){                                     //트리의 root가 NULL이면
        return -1;
    }
    else if(tree->root->left == NULL && tree->root->right == NULL){ //트리가 하나의 노드만 있으면
        if(key == tree->root->key){                                 //그 하나의 노드와 찾으려는 키 값이 같을 때
            c_node = tree->root;
            tree->root = NULL;
        }
        else{ 
            return -1;
        }
    }
    else{                                                           //트리의 노드가 한 개보다 많을 때
        c_node = tree->root; parent = tree->root;
        while(c_node != NULL){                                      //삭제하려는 키 값의 위치를 찾기 위한 while문
            if(c_node->key == key)
                break;
            else if(c_node->key > key){
                parent = c_node;
                c_node = c_node->left;
                lr=0;
            }
            else{
                parent = c_node;
                c_node = c_node->right;
                lr=1;
            }
        }
        if(c_node == NULL){                                         //삭제하려는 키 값을 가진 노드를 찾이 못했을 때
            return -1;
        }
        else if(c_node->left == NULL && c_node->right == NULL){     //삭제하려는 노드가 자식이 없을 때
            if(lr==0)
                parent->left = NULL;
            else
                parent->right = NULL;
        }
        else if(c_node->left != NULL && c_node->right !=NULL){      //삭제하려는 노드가 두 개의 자식을 가질 때
             lab2_node *replace = c_node->left, *r_parent = c_node;
             while(replace->right != NULL){                         //삭제하려는 노드의 왼쪽노드 중 가장 큰 노드를 찾기 위한 while문
                 r_parent = replace;
                 replace = replace->right;
             }
             if(replace == c_node->left){                           //왼쪽 노드 중 가장 큰 노드가 replace노드인 경우
                 if(c_node == tree->root){
                    replace->right = c_node->right;
                    tree->root = replace;
                 }
                 else if(lr==0){
                    replace->right = c_node->right;
                    parent->left = replace;
                 }
                 else{
                    replace->right = c_node->right;
                    parent->right = replace;

                }
             }
             else{                                                  //왼쪽 노드 중 가장 큰 노드가 replace 노드가 아닌 경우
                r_parent->right = replace->left;
                replace->left = c_node->left;
                replace->right = c_node->right;
                if(c_node == tree-> root){                         //삭제하려는 노드가 root 노드이면
                    tree->root = replace;
                }
                else if(lr==0)
                    parent->left = replace;
                else
                    parent->right = replace;
             }
        }
        else{                                                       //삭제하려는 노드가 한 개의 자식을 갖는다면
            if(c_node == tree->root){
                 if(c_node->left != NULL)                           //자식노드가 왼쪽이면
                    tree->root = c_node->left;                      //삭제하려는 노드의 왼쪽 자식이 root가 된다.
                else                                                //자식노드가 오른쪽이면
                    tree->root = c_node->right;                     //삭제하려는 노드의 오른쪽 자식이 root가 된다.

            }
            else if(lr==1){
                if(c_node->left != NULL)
                    parent->right = c_node->left;
                else
                    parent->right = c_node->right;
            }
            else{
                if(c_node->left != NULL)
                    parent->left = c_node->left;
                else
                    parent->left = c_node->right;
            }
        }
    }
    return 0;
}

int lab2_node_remove_fg(lab2_tree *tree, int key) {
    lab2_node *c_node, *parent;
    int lr=0;
    start:
    if(tree == NULL){
        return -1;
    }
    else if(tree->root == NULL){
        return -1;
    }
    else if(tree->root->left == NULL && tree->root->right == NULL){
        if(key == tree->root->key){
            c_node = tree->root;
            if(pthread_mutex_lock(&(c_node->mutex))!=0){
                goto start;
            }
            tree->root = NULL;
        }
        else{ 
            return -1;
        }
    }
    else{                                                          
        c_node = tree->root; parent = tree->root;
        while(c_node != NULL){
            if(pthread_mutex_lock(&(c_node->mutex))!=0){
                goto start;
            }
            else{
                pthread_mutex_unlock(&(c_node->mutex));
            }
            if(c_node->key == key)
                break;
            else if(c_node->key > key){
                parent = c_node;
                c_node = c_node->left;
                lr=0;
            }
            else{
                parent = c_node;
                c_node = c_node->right;
                lr=1;
            }
        }
        if(c_node == NULL){
            return -1;
        }
        else{
            if(pthread_mutex_lock(&(c_node->mutex))!=0){
                goto start;
            }
        }
        if(c_node->left == NULL && c_node->right == NULL){
            if(lr==0)
                parent->left = NULL;
            else
                parent->right = NULL;
            pthread_mutex_unlock(&(c_node->mutex));
        }
        else if(c_node->left != NULL && c_node->right !=NULL){
             lab2_node *replace = c_node->left, *r_parent = c_node;
             while(replace->right != NULL){
                 if(pthread_mutex_lock(&(replace->mutex))!=0){
                     pthread_mutex_unlock(&(c_node->mutex));
                     goto start;
                 }
                 else
                    pthread_mutex_unlock(&(replace->mutex));
                 r_parent = replace;
                 replace = replace->right;
             }
             if(replace == c_node->left){
                 if(c_node == tree->root){
                    replace->right = c_node->right;
                    tree->root = replace;
                 }
                 else if(lr==0){
                    replace->right = c_node->right;
                    parent->left = replace;
                 }
                 else{
                    replace->right = c_node->right;
                    parent->right = replace;

                }
             }
             else{
                r_parent->right = replace->left;
                replace->left = c_node->left;
                replace->right = c_node->right;
                if(c_node == tree-> root){
                    tree->root = replace;
                }
                else if(lr==0)
                    parent->left = replace;
                else
                    parent->right = replace;
             }
             pthread_mutex_unlock(&(c_node->mutex));
        }
        else{
            if(c_node == tree->root){
                 if(c_node->left != NULL)
                    tree->root = c_node->left;
                else
                    tree->root = c_node->right;

            }
            else if(lr==1){
                if(c_node->left != NULL)
                    parent->right = c_node->left;
                else
                    parent->right = c_node->right;
            }
            else{
                if(c_node->left != NULL)
                    parent->left = c_node->left;
                else
                    parent->left = c_node->right;
            }
            pthread_mutex_unlock(&(c_node->mutex));
        }
    }
    return 0;
}

int lab2_node_remove_cg(lab2_tree *tree, int key) {
    pthread_mutex_lock(&cg_remove);
    lab2_node *c_node, *parent;
    int lr=0;
    if(tree == NULL){
        pthread_mutex_unlock(&cg_remove);
        return -1;
    }
    else if(tree->root == NULL){
        pthread_mutex_unlock(&cg_remove);
        return -1;
    }
    else if(tree->root->left == NULL && tree->root->right == NULL){
        if(key == tree->root->key){
            c_node = tree->root;
            tree->root = NULL;
        }
        else{ 
            pthread_mutex_unlock(&cg_remove);
            return -1;
        }
    }
    else{
        c_node = tree->root; parent = tree->root;
        while(c_node != NULL){
            if(c_node->key == key)
                break;
            else if(c_node->key > key){
                parent = c_node;
                c_node = c_node->left;
                lr=0;
            }
            else{
                parent = c_node;
                c_node = c_node->right;
                lr=1;
            }
        }
        if(c_node == NULL){
            pthread_mutex_unlock(&cg_remove);
            return -1;
        }
        else if(c_node->left == NULL && c_node->right == NULL){
            if(lr==0)
                parent->left = NULL;
            else
                parent->right = NULL;
        }
        else if(c_node->left != NULL && c_node->right !=NULL){
             lab2_node *replace = c_node->left, *r_parent = c_node;
             while(replace->right != NULL){
                 r_parent = replace;
                 replace = replace->right;
             }
             if(replace == c_node->left){
                 if(c_node == tree->root){
                    replace->right = c_node->right;
                    tree->root = replace;
                 }
                 else if(lr==0){
                    replace->right = c_node->right;
                    parent->left = replace;
                 }
                 else{
                    replace->right = c_node->right;
                    parent->right = replace;

                }
             }
             else{
                r_parent->right = replace->left;
                replace->left = c_node->left;
                replace->right = c_node->right;
                if(c_node == tree-> root){
                    tree->root = replace;
                }
                else if(lr==0)
                    parent->left = replace;
                else
                    parent->right = replace;
             }
        }
        else{
            if(c_node == tree->root){
                 if(c_node->left != NULL)
                    tree->root = c_node->left;
                else
                    tree->root = c_node->right;

            }
            else if(lr==1){
                if(c_node->left != NULL)
                    parent->right = c_node->left;
                else
                    parent->right = c_node->right;
            }
            else{
                if(c_node->left != NULL)
                    parent->left = c_node->left;
                else
                    parent->left = c_node->right;
            }
        }
    }
    pthread_mutex_unlock(&cg_remove);
    return 0;
}

void lab2_tree_delete(lab2_tree *tree) {
    if(tree == NULL)
        return;
    else{
        lab2_node_delete(tree->root);
        free(tree);
    }
}

void lab2_node_delete(lab2_node *node) {
    if(node != NULL){                   
        lab2_node_delete(node->left);
        lab2_node_delete(node->right);
        free(node);
    }
}

