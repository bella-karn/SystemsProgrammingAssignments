/* 
* Bella Karn
* code for strmapbis.c, note: almost all of it, other than new functions, is from original strmap.c file from project 0 (specifiying to avoid self plagarism). I got a 100, so 
* I figured it was a good place to start
* Any additonal sources listed here:
* ChatGPT as specified in line 88
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strmapbis.h"

unsigned int hash_string(const char *key)
{
    unsigned int hash = 0;

    while (*key)
    {
        hash = (hash << 5) + *key++;
    }

    return hash;
}

/* Create a new hashtab, initialized to empty. */
strmap_t *strmap_create(int numbuckets)
{
    // does the clipping first, this would need to be right before I allocate anything
    if (numbuckets > MAX_BUCKETS)
    {
        numbuckets = MAX_BUCKETS;
    }
    else if (numbuckets < MIN_BUCKETS)
    {
        numbuckets = MIN_BUCKETS;
    }
    
    // Allocates memory to the map.
    strmap_t *temp = (strmap_t *) malloc(sizeof(strmap_t));

    // this creates/allocates memory to the array of pointers smel_t
    temp->strmap_buckets = (smel_t **)calloc(numbuckets,sizeof(smel_t*));
    temp->strmap_size = 0;
    temp->strmap_nbuckets = numbuckets;

    return temp;    
}

void strmap_resize(strmap_t *m, double target)
{
    double LF = strmap_getloadfactor(m);
    if ((1 - LFSLOP) * target <= LF && LF <= (1 + LFSLOP) * target)
    {
        return;
    }
    else 
    {
        // Step 1: calculate the number of buckets needed to make that precondition true, and make sure it is in the right range
        int correctNum = (int) (strmap_getsize(m) / target);
        // the following condition superceeds the previous condition
        if (correctNum < MIN_BUCKETS)
        {
            correctNum = MIN_BUCKETS;
        }
        else if (correctNum > MAX_BUCKETS)
        {
            correctNum = MAX_BUCKETS;
        }

        // step 2: create the array of buckets
        if (strmap_getnbuckets(m) != correctNum)
        {
            // the new array of buckets that I can allocate memory for
            smel_t **new_buckets = (smel_t **)calloc(correctNum, sizeof(smel_t*));

            for (int i = 0; i < strmap_getnbuckets(m); i++)
            {
                smel_t *current = m-> strmap_buckets[i];
                while (current != NULL)
                {
                    smel_t *next = current -> sme_next;
                    unsigned int index = hash_string(current -> sme_key) % correctNum;

                    // So I originally had the following to put things in the new array, and it didn't work very well so I asked chatGPT and replaced the put with what it gave me
                    // strmap_put(m, current -> sme_key, current -> sme_value);
                    // Specific lines given by chatGPT: 89-90
                    // 
                    current->sme_next = new_buckets[index];
                    new_buckets[index] = current;

                    current = next;
                }
            }
            // step 3: Free everything back up
            free(m->strmap_buckets);
            m ->strmap_buckets = new_buckets;
            m->strmap_nbuckets = correctNum;
        }
    }
}

/* Insert an element with the given key and value.
 *  Return the previous value associated with that key, or null if none.
 */
void *strmap_put(strmap_t *m, char *key, void *value)
{
    // need to find bucket index
    unsigned int index = hash_string(key) % m->strmap_nbuckets;
    smel_t *currentbucket = m->strmap_buckets[index];

    smel_t *prevBucket = NULL;
    
    while (currentbucket != NULL)
    {
        if(strcmp(currentbucket->sme_key, key) == 0)
        {
            void *prevValue = currentbucket->sme_value;
            currentbucket->sme_value = value;
            return prevValue;
        }
        prevBucket = currentbucket;
        currentbucket = currentbucket->sme_next;
    }

    // Allocates memory again
    smel_t *added = (smel_t *)malloc(sizeof(smel_t));

    if (added == NULL)
    {
        return NULL;
    }

    added->sme_key = strdup(key);

    if (added->sme_key == NULL)
    {
        free(added);
        return NULL;
    }

    added->sme_value = value;
    added->sme_next = NULL;

    if(prevBucket == NULL)
    {
        m->strmap_buckets[index] = added;
    }
    else
    {
        prevBucket->sme_next = added;
    }

    m->strmap_size++;
    return NULL;
}

/* return the value associated with the given key, or null if none */
void *strmap_get(strmap_t *m, char *key)
{
    // This code was altered to avoid seg faults that became apparent in second test file
    if (m == NULL)
    {
        return NULL;
    }
    // getting index, pretty much copied from above
    int index = hash_string(key) % m->strmap_nbuckets;
    char *currentkey = key;
    
    // This should be avoiding collisions
    if (index >= 0 && index < m->strmap_nbuckets)
    {
        smel_t *currentBucket = m->strmap_buckets[index];
        while (currentBucket != NULL)
        {
            if(strcmp(currentBucket->sme_key, key) == 0)
            {
                return currentBucket->sme_value;
            }
            currentBucket = currentBucket->sme_next;
        }
    }

}

/* remove the element with the given key and return its value.
   Return null if the hashtab contains no element with the given key */
void *strmap_remove(strmap_t *m, char *key)
{
    // Calculate the index using the same method as strmap_put
    unsigned int index = hash_string(key) % m->strmap_nbuckets;

    smel_t *currentBucket = m->strmap_buckets[index];
    smel_t *prevBucket = NULL;

    while (currentBucket != NULL)
    {
        if (strcmp(currentBucket->sme_key, key) == 0)
        {
            void *removedValue = currentBucket->sme_value;

            if (prevBucket == NULL)
            {
                m->strmap_buckets[index] = currentBucket->sme_next;
            }
            else
            {
                prevBucket->sme_next = currentBucket->sme_next;
            }

            free(currentBucket->sme_key);
            free(currentBucket);

            m->strmap_size--;

            return removedValue;
        }

        prevBucket = currentBucket;
        currentBucket = currentBucket->sme_next;
    }

    return NULL;
}

/* return the # of elements in the hashtab */
int strmap_getsize(strmap_t *m)
{
    return m->strmap_size;
}

/* return the # of buckets in the hashtab */
int strmap_getnbuckets(strmap_t *m)
{
    return m->strmap_nbuckets;
}

/* print out the contents of each bucket */
void strmap_dump(strmap_t *m)
{
    int totalElements = 0;

    // Count total elements
    for (int i = 0; i < strmap_getnbuckets(m); i++)
    {
        smel_t *current = m->strmap_buckets[i];
        while (current != NULL)
        {
            current = current->sme_next;
            totalElements++;
        }
    }

    // Print total elements
    printf("total elements = %d.\n", totalElements);

    // Print bucket details
    for (int i = 0; i < strmap_getnbuckets(m); i++)
    {
        smel_t *current = m->strmap_buckets[i];
        int bucketCount = 0;

        while (current != NULL)
        {
            current = current->sme_next;
            bucketCount++;
        }

        if (bucketCount > 0)
        {
            printf("bucket %d:\n", i);

            current = m->strmap_buckets[i];
            while (current != NULL)
            {
                printf("    %s->%p\n", current->sme_key, current->sme_value);
                current = current->sme_next;
            }
        }
    }
}

/* return the current load factor of the map */
// simple arithmatic!
double strmap_getloadfactor(strmap_t *m)
{
    double loadFactor = ((double)m->strmap_size)/((double)m->strmap_nbuckets);
    return loadFactor;
}