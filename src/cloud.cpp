#include <iostream>
#include <stdio.h>
#include <string.h>
#include "../inc/json-c/json.h"
#include "../inc/site.hpp"
#include "../inc/cloud.hpp"

using namespace std;

// Implémentation des fonctions de Cloud

Cloud* init_cloud_json(const char* file_name) {
    //! Implémentation temporaire ne marchera pas pour un fichier de plus de MAX_LEN_BUFFER_JSON octets
    // <https://www.google.com/search?channel=fs&client=ubuntu&q=use+json+with+c>
    //malloc
    FILE *file;
    char buffer[MAX_LEN_BUFFER_JSON];

	file = fopen(file_name, "r");
    if (file == NULL) {
        perror("Can't acces file:");
        return NULL;
    }

	fread(buffer, MAX_LEN_BUFFER_JSON, 1, file);
    if (!feof(file)) {
        cerr << "Attention le fichier n'a pas été entièrement lu!"<< endl;
    }
	fclose(file);
    
    return init_cloud(buffer);
}


Cloud* init_cloud(const char* json) {
    Cloud* cloud;
	struct json_object *parsed_json;
	struct json_object *name;
	struct json_object *cpu;
	struct json_object *memory;
	struct json_object *site;

	size_t n_sites;

	size_t i;

    parsed_json = json_tokener_parse(json);

    n_sites = json_object_array_length(parsed_json);
    cloud = (Cloud*) malloc(sizeof(Cloud));
    if (n_sites > 0) {
        cloud->sites = (Site**) malloc(n_sites * sizeof(Site*));
        if (cloud->sites == NULL) {
            perror("Erreur malloc:");
            return NULL;
        }
        cloud->size = n_sites;
    }
    for (i = 0; i < n_sites; i++) {
        site = json_object_array_get_idx(parsed_json, i);

        json_object_object_get_ex(site, "name", &name);
        json_object_object_get_ex(site, "cpu", &cpu);        
        json_object_object_get_ex(site, "memory", &memory);        

        cloud->sites[i] = init_site(
            json_object_get_string(name), 
            json_object_get_int(cpu), 
            json_object_get_int(memory)
        );
        
    }
    return cloud;
}


int destroy_cloud(Cloud* cloud) {
    for (int i = 0; i < cloud->size; i++) {
        if (destroy_site(cloud->sites[i]) == -1) return -1;
    }
    free(cloud->sites);
    cloud->size = 0;
    return 0;
}

int add_site(Cloud* cloud,Site* site) {
    if (cloud == NULL || site == NULL) {
        cerr << "Null pointeur" << endl;
        return -1;
    }
    cloud->sites = (Site**) realloc(cloud->sites, cloud->size + 1);
    if (cloud->sites == NULL) return -1;
    cloud->sites[cloud->size] = site;
    cloud->size += 1;
    return 0;
}

Site* get_site(Cloud* cloud, int i) {
    if (cloud == NULL || i >= cloud->size) return NULL;
    return cloud->sites[i];
}

int rm_site(Cloud* cloud, int i) {
    if (cloud == NULL || i >= cloud->size || cloud->sites[i] == NULL) return -1;
    if (destroy_site(cloud->sites[i]) == -1) return -1;
    for (int j = i; j < cloud->size - 1; j++)
        cloud->sites[j] = cloud->sites[j+1];
    cloud->sites = (Site**) realloc(cloud->sites, cloud->size - 1);
    cloud->size -= 1;
    return (cloud->sites==NULL)?-1:0;
}


void print_cloud(Cloud* cloud) {
    if (cloud == NULL) {
        cout << "cloud = NULL" << endl;
        return;
    }
    for(int i = 0; i < cloud->size; i++) {
        cout << "Site " << i+1 << ": ";
        print_site(cloud->sites[i]);
        cout << endl;
    }
    return;
}

int code_cloud(Cloud* cloud, char* code, int size_string) {
    if (cloud == NULL) return -1;
    for (int i = 0; i < size_string; i++)
        code[i] = '\0';
    
    int curseur = 0;
    code[curseur++] = '[';
    for (int i = 0; i < cloud->size; i++) {
        char snum[MAX_LEN_CPU];
        code[curseur++] = '{';
        strcat(code, "'name':'");curseur+=strlen("'name':'");
        strcat(code, cloud->sites[i]->name);curseur+=strlen(cloud->sites[i]->name);
        strcat(code, "','cpu':");curseur+=strlen("','cpu':");
        sprintf(snum, "%d", get_cpu_available(cloud->sites[i]));
        strcat(code, snum);curseur+=strlen(snum);
        strcat(code, ",'memory':");curseur+=strlen(",'memory':");
        sprintf(snum, "%d", get_memory_available(cloud->sites[i]));
        strcat(code, snum);curseur+=strlen(snum);
        code[curseur++] = '}';
        code[curseur++] = ',';

    }
    code[--curseur] =']';
    return curseur;
}

Cloud* decode_cloud(char* code, int size_string) {
    if (size_string > MAX_LEN_BUFFER_JSON) return NULL;
    return init_cloud(code);
}