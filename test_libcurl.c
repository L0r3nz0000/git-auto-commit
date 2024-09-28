#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct string {
    char *ptr;
    size_t len;
};

// Funzione per inizializzare una stringa vuota
void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(s->len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

// Funzione per scrivere i dati ricevuti nella risposta
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

// Funzione per fare una POST e ottenere l'URL dello stream
char* get_prediction_stream_url() {
    CURL *curl;
    CURLcode res;
    struct string s;
    init_string(&s);

    curl = curl_easy_init();
    if(curl) {
        // Imposta l'URL e l'header di autorizzazione
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.replicate.com/v1/models/meta/meta-llama-3-70b-instruct/predictions");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        char auth_header[100];

        char* REPLICATE_API_TOKEN = getenv("REPLICATE_API_TOKEN");
        if (REPLICATE_API_TOKEN == NULL) {
          printf("REPLICATE_API_TOKEN is not set, aborting.\n");
          return 1;
        }

        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", REPLICATE_API_TOKEN);
        headers = curl_slist_append(headers, auth_header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Corpo della richiesta
        const char *data = "{\"stream\": true, \"input\": {\"top_p\": 0.9, \"prompt\": \"Work through this problem step by step:\\n\\nQ: Sarah has 7 llamas. Her friend gives her 3 more trucks of llamas. Each truck has 5 llamas. How many llamas does Sarah have in total?\", \"min_tokens\": 0, \"temperature\": 0.6, \"prompt_template\": \"<|begin_of_text|><|start_header_id|>system<|end_header_id|>\\n\\nYou are a helpful assistant<|eot_id|><|start_header_id|>user<|end_header_id|>\\n\\n{prompt}<|eot_id|><|start_header_id|>assistant<|end_header_id|>\\n\\n\", \"presence_penalty\": 1.15}}";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        // Imposta la callback per scrivere la risposta
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        // Esegui la richiesta
        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        else {
            printf("Risposta ricevuta: %s\n", s.ptr);  // Per vedere la risposta completa
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    // Estrai l'URL dello stream dalla risposta
    char *stream_url = strstr(s.ptr, "\"stream\":\"");
    if (stream_url) {
        stream_url += 10;  // Salta "stream\":\""
        char *end = strchr(stream_url, '"');
        if (end) {
            *end = '\0';
        }
        return strdup(stream_url);  // Ritorna l'URL dello stream
    }

    return NULL;
}

// Funzione per connettersi allo stream
void stream_events(const char *stream_url) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        // Imposta l'URL dello stream e gli header
        curl_easy_setopt(curl, CURLOPT_URL, stream_url);
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Accept: text/event-stream");
        headers = curl_slist_append(headers, "Cache-Control: no-store");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Imposta la callback per scrivere la risposta
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        struct string s;
        init_string(&s);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        // Esegui la richiesta
        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        else {
            printf("Eventi dallo stream:\n%s\n", s.ptr);  // Stampa gli eventi dallo stream
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

int main(void) {
    // Ottieni l'URL dello stream
    char *stream_url = get_prediction_stream_url();
    if (stream_url) {
        printf("URL dello stream: %s\n", stream_url);

        // Connettiti allo stream e leggi gli eventi
        stream_events(stream_url);

        free(stream_url);
    } else {
        printf("Errore: non è stato possibile ottenere l'URL dello stream.\n");
    }

    return 0;
}
