/*
*	Operating System Lab
*	    Lab2 (Synchronization)
*	    Student id : 32143199  
*	    Student name : 이민승 
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
    lab2_tree *rt;
    rt = malloc(sizeof(lab2_tree));
    rt->root = NULL; 
    return rt;
}

lab2_node * lab2_node_create(int key) {
    lab2_node *rn;
    rn = malloc(sizeof(lab2_node));
    pthread_mutex_t tmp = PTHREAD_MUTEX_INITIALIZER;
    rn->mutex = tmp;
    rn->left = NULL;
    rn->right = NULL;
    rn->key = key;
    return rn;
}

int lab2_node_insert(lab2_tree *tree, lab2_node *new_node){
    if(tree==NULL || new_node==NULL){                                   //tree or node is invalid
        return -1;
    }
    else{                               
        if(tree->root == NULL){                                         //tree is empty
            tree->root = new_node;
            return 0;
        }
        else{                                                           //tree is not empty
            lab2_node *current = tree->root, *parent = tree->root;
            int lr=0;
            while(current != NULL){                                     //find node's place
                if(current->key == new_node->key)
                    return -1;
                else if(current->key > new_node->key){
                    parent = current;
                    current = current->left;
                    lr=0;
                }
                else{
                    parent = current;
                    current = current->right;
                    lr=1;
                }
            }
                if(lr==0) parent->left = new_node;                       //insert node
                else parent->right = new_node;
                return 0;
        }
    }
}

int lab2_node_insert_fg(lab2_tree *tree, lab2_node *new_node){  
    start: 
     if(tree==NULL || new_node==NULL){                                   //tree or node is invalid
        return -1;
    }
    else{                               
        if(tree->root == NULL){                                         //tree is empty
            if(pthread_mutex_lock(&cg_insert) != 0){
                goto start;
            }
            tree->root = new_node;
            pthread_mutex_unlock(&cg_insert);
            return 0;
        }
        else{                                                           //tree is not empty
            lab2_node *current = tree->root, *parent = tree->root;
            int lr=0;
            while(current != NULL){                                     //find node's place
                if(current->key == new_node->key)                       //same node already inserted
                    return -1;
                else if(current->key > new_node->key){
                    parent = current;
                    current = current->left;
                    lr=0;
                }
                else{
                    parent = current;
                    current = current->right;
                    lr=1;
                }
            }
            if(pthread_mutex_lock(&(parent->mutex)) != 0){
                goto start;
            }
            if(lr==0) parent->left = new_node;                           //insert node
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
            lab2_node *current = tree->root, *parent = tree->root;
            int lr=0;
            while(current != NULL){
                if(current->key == new_node->key){
                    pthread_mutex_unlock(&cg_insert);
                    return -1;
                }
                else if(current->key > new_node->key){
                    parent = current;
                    current = current->left;
                    lr=0;
                }
                else{
                    parent = current;
                    current = current->right;
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
    lab2_node *current, *parent;
    int rl=0;
    if(tree == NULL){                                                //tree is invalid
        return -1;
    }
    else if(tree->root == NULL){                                     //tree has no node
        return -1;
    }
    else if(tree->root->left == NULL && tree->root->right == NULL){ //tree has only one node
        if(key == tree->root->key){
            current = tree->root;
            tree->root = NULL;
        }
        else{ 
            return -1;
        }
    }
    else{                                                           //tree has more than one node
        current = tree->root; parent = tree->root;
        while(current != NULL){
            if(current->key == key)
                break;
            else if(current->key > key){
                parent = current;
                current = current->left;
                rl=0;
            }
            else{
                parent = current;
                current = current->right;
                rl=1;
            }
        }
        if(current == NULL){                                         //node has not been found 
            return -1;
        }
        else if(current->left == NULL && current->right == NULL){   //current has no child
            if(rl==0)
                parent->left = NULL;
            else
                parent->right = NULL;
        }
        else if(current->left != NULL && current->right !=NULL){    //current has 2 children
             lab2_node *replace = current->left, *r_parent = current;  
             while(replace->right != NULL){                         //find biggest node in left sub-tree of current
                 r_parent = replace;
                 replace = replace->right;
             }
             if(replace == current->left){                               //replace node is left child
                 if(current == tree->root){
                    replace->right = current->right;
                    tree->root = replace;
                 }
                 else if(rl==0){
                    replace->right = current->right;
                    parent->left = replace;
                 }
                 else{
                    replace->right = current->right;
                    parent->right = replace;

                }
             }
             else{                                                  //replace node is not left child
                r_parent->right = replace->left;
                replace->left = current->left;
                replace->right = current->right;
                if(current == tree-> root){                         //current is root node
                    tree->root = replace;
                }
                else if(rl==0)
                    parent->left = replace;
                else
                    parent->right = replace;
             }
        }
        else{                                                       //current has 1 child
            if(current == tree->root){
                 if(current->left != NULL)
                    tree->root = current->left;
                else
                    tree->root = current->right;

            }
            else if(rl==1){
                if(current->left != NULL)
                    parent->right = current->left;
                else
                    parent->right = current->right;
            }
            else{
                if(current->left != NULL)
                    parent->left = current->left;
                else
                    parent->left = current->right;
            }
        }
    }
    return 0;
}

int lab2_node_remove_fg(lab2_tree *tree, int key) {
    lab2_node *current, *parent;
    int rl=0;
    start:
    if(tree == NULL){                                                //tree is invalid
        return -1;
    }
    else if(tree->root == NULL){                                     //tree has no node
        return -1;
    }
    else if(tree->root->left == NULL && tree->root->right == NULL){ //tree has only one node
        if(key == tree->root->key){
            current = tree->root;
            if(pthread_mutex_lock(&(current->mutex))!=0){
                goto start;
            }
            tree->root = NULL;
        }
        else{ 
            return -1;
        }
    }
    else{                                                           //tree has more than one node
        current = tree->root; parent = tree->root;
        while(current != NULL){
            if(pthread_mutex_lock(&(current->mutex))!=0){
                goto start;
            }
            else{
                pthread_mutex_unlock(&(current->mutex));
            }
            if(current->key == key)
                break;
            else if(current->key > key){
                parent = current;
                current = current->left;
                rl=0;
            }
            else{
                parent = current;
                current = current->right;
                rl=1;
            }
        }
        if(current == NULL){                                         //node has not been found 
            return -1;
        }
        else{
            if(pthread_mutex_lock(&(current->mutex))!=0){
                goto start;
            }
        }
        if(current->left == NULL && current->right == NULL){   //current has no child
            if(rl==0)
                parent->left = NULL;
            else
                parent->right = NULL;
            pthread_mutex_unlock(&(current->mutex));
        }
        else if(current->left != NULL && current->right !=NULL){    //current has 2 children
             lab2_node *replace = current->left, *r_parent = current;  
             while(replace->right != NULL){                         //find biggest node in left sub-tree of current
                 if(pthread_mutex_lock(&(replace->mutex))!=0){
                     pthread_mutex_unlock(&(current->mutex));
                     goto start;
                 }
                 else
                    pthread_mutex_unlock(&(replace->mutex));
                 r_parent = replace;
                 replace = replace->right;
             }
             if(replace == current->left){                               //replace node is left child
                 if(current == tree->root){
                    replace->right = current->right;
                    tree->root = replace;
                 }
                 else if(rl==0){
                    replace->right = current->right;
                    parent->left = replace;
                 }
                 else{
                    replace->right = current->right;
                    parent->right = replace;

                }
             }
             else{                                                  //replace node is not left child
                r_parent->right = replace->left;
                replace->left = current->left;
                replace->right = current->right;
                if(current == tree-> root){                         //current is root node
                    tree->root = replace;
                }
                else if(rl==0)
                    parent->left = replace;
                else
                    parent->right = replace;
             }
             pthread_mutex_unlock(&(current->mutex));
        }
        else{                                                       //current has 1 child
            if(current == tree->root){
                 if(current->left != NULL)
                    tree->root = current->left;
                else
                    tree->root = current->right;

            }
            else if(rl==1){
                if(current->left != NULL)
                    parent->right = current->left;
                else
                    parent->right = current->right;
            }
            else{
                if(current->left != NULL)
                    parent->left = current->left;
                else
                    parent->left = current->right;
            }
            pthread_mutex_unlock(&(current->mutex));
        }
    }
    return 0;
}

int lab2_node_remove_cg(lab2_tree *tree, int key) {
    pthread_mutex_lock(&cg_remove);
    lab2_node *current, *parent;
    int rl=0;
    if(tree == NULL){                                                //tree is invalid
        pthread_mutex_unlock(&cg_remove);
        return -1;
    }
    else if(tree->root == NULL){                                     //tree has no node
        pthread_mutex_unlock(&cg_remove);
        return -1;
    }
    else if(tree->root->left == NULL && tree->root->right == NULL){ //tree has only one node
        if(key == tree->root->key){
            current = tree->root;
            tree->root = NULL;
        }
        else{ 
            pthread_mutex_unlock(&cg_remove);
            return -1;
        }
    }
    else{                                                           //tree has more than one node
        current = tree->root; parent = tree->root;
        while(current != NULL){
            if(current->key == key)
                break;
            else if(current->key > key){
                parent = current;
                current = current->left;
                rl=0;
            }
            else{
                parent = current;
                current = current->right;
                rl=1;
            }
        }
        if(current == NULL){                                         //node has not been found 
            pthread_mutex_unlock(&cg_remove);
            return -1;
        }
        else if(current->left == NULL && current->right == NULL){   //current has no child
            if(rl==0)
                parent->left = NULL;
            else
                parent->right = NULL;
        }
        else if(current->left != NULL && current->right !=NULL){    //current has 2 children
             lab2_node *replace = current->left, *r_parent = current;  
             while(replace->right != NULL){                         //find biggest node in left sub-tree of current
                 r_parent = replace;
                 replace = replace->right;
             }
             if(replace == current->left){                               //replace node is left child
                 if(current == tree->root){
                    replace->right = current->right;
                    tree->root = replace;
                 }
                 else if(rl==0){
                    replace->right = current->right;
                    parent->left = replace;
                 }
                 else{
                    replace->right = current->right;
                    parent->right = replace;

                }
             }
             else{                                                  //replace node is not left child
                r_parent->right = replace->left;
                replace->left = current->left;
                replace->right = current->right;
                if(current == tree-> root){                         //current is root node
                    tree->root = replace;
                }
                else if(rl==0)
                    parent->left = replace;
                else
                    parent->right = replace;
             }
        }
        else{                                                       //current has 1 child
            if(current == tree->root){
                 if(current->left != NULL)
                    tree->root = current->left;
                else
                    tree->root = current->right;

            }
            else if(rl==1){
                if(current->left != NULL)
                    parent->right = current->left;
                else
                    parent->right = current->right;
            }
            else{
                if(current->left != NULL)
                    parent->left = current->left;
                else
                    parent->left = current->right;
            }
        }
    }
    pthread_mutex_unlock(&cg_remove);
    return 0;
}

void lab2_tree_delete(lab2_tree *tree) {                            //free every node in tree and itself
    if(tree == NULL)
        return;
    else{
        lab2_node_delete(tree->root);
        free(tree);
    }
}

void lab2_node_delete(lab2_node *node) {                            //travel from root to every node to delete them
    if(node != NULL){                   
        lab2_node_delete(node->left);
        lab2_node_delete(node->right);
        free(node);
    }
}

