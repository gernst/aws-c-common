/*
* Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License").
* You may not use this file except in compliance with the License.
* A copy of the License is located at
*
*  http://aws.amazon.com/apache2.0
*
* or in the "license" file accompanying this file. This file is distributed
* on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
* express or implied. See the License for the specific language governing
* permissions and limitations under the License.
*/

#include <aws/common/hash_table.h>
#include <aws/testing/aws_test_harness.h>
#include <stdio.h>

static const char *test_str_1 = "test 1";
static const char *test_str_2 = "test 2";

static const char *test_val_str_1 = "value 1";
static const char *test_val_str_2 = "value 2";

AWS_TEST_CASE(test_hash_table_put_get, test_hash_table_put_get_fn)
static int test_hash_table_put_get_fn(struct aws_allocator *alloc, void *ctx) {
    struct aws_common_hash_table hash_table;
    int err_code = aws_common_hash_table_init(&hash_table, alloc, 10, aws_common_hash_string,
        aws_common_string_eq, NULL);
    struct aws_common_hash_element *pElem;
    int was_created;

    ASSERT_SUCCESS(err_code,
        "Hash Map init should have succeeded.");

    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_1, &pElem, &was_created);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");
    ASSERT_INT_EQUALS(1, was_created,
        "Hash Map put should have created a new element.");
    pElem->value = (void *)test_val_str_1;

    /* Try passing a NULL was_created this time */
    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_2, &pElem, NULL);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");
    pElem->value = (void *)test_val_str_2;

    err_code = aws_common_hash_table_find(&hash_table, (void *)test_str_1, &pElem);
    ASSERT_SUCCESS(err_code,
        "Hash Map get should have succeeded.");
    ASSERT_STR_EQUALS(test_val_str_1, (const char *)pElem->value,
        "Returned value for %s, should have been %s", test_str_1, test_val_str_1);

    err_code = aws_common_hash_table_find(&hash_table, (void *)test_str_2, &pElem);
    ASSERT_SUCCESS(err_code,
        "Hash Map get should have succeeded.");
    ASSERT_BIN_ARRAYS_EQUALS(test_val_str_2, strlen(test_val_str_2) + 1, (const char *)pElem->value, strlen(pElem->value) + 1,
        "Returned value for %s, should have been %s", test_str_2, test_val_str_2);

    aws_common_hash_table_clean_up(&hash_table);
    RETURN_SUCCESS("%s() pass", "test_hash_table_put_get");
}

static uint64_t hash_collide(const void *a) {
    return 4;
}

AWS_TEST_CASE(test_hash_table_hash_collision, test_hash_table_hash_collision_fn)
static int test_hash_table_hash_collision_fn(struct aws_allocator *alloc, void *ctx) {
    struct aws_common_hash_table hash_table;
    struct aws_common_hash_element *pElem;
    int err_code = aws_common_hash_table_init(&hash_table, alloc, 10, hash_collide,
        aws_common_string_eq, NULL);

    ASSERT_SUCCESS(err_code,
        "Hash Map init should have succeeded.");

    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_1, &pElem, NULL);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");
    pElem->value = (void *)test_val_str_1;

    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_2, &pElem, NULL);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");
    pElem->value = (void *)test_val_str_2;

    err_code = aws_common_hash_table_find(&hash_table, (void *)test_str_1, &pElem);
    ASSERT_SUCCESS(err_code,
        "Hash Map get should have succeeded.");
    ASSERT_STR_EQUALS(test_val_str_1, pElem->value,
        "Returned value for %s, should have been %s", test_str_1, test_val_str_1);

    err_code = aws_common_hash_table_find(&hash_table, (void *)test_str_2, &pElem);
    ASSERT_SUCCESS(err_code,
        "Hash Map get should have succeeded.");
    ASSERT_STR_EQUALS(test_val_str_2, pElem->value,
        "Returned value for %s, should have been %s", test_str_2, test_val_str_2);

    aws_common_hash_table_clean_up(&hash_table);
    RETURN_SUCCESS("%s() pass", "test_hash_table_hash_collision");
}

AWS_TEST_CASE(test_hash_table_hash_overwrite, test_hash_table_hash_overwrite_fn)
static int test_hash_table_hash_overwrite_fn(struct aws_allocator *alloc, void *ctx) {
    struct aws_common_hash_table hash_table;
    struct aws_common_hash_element *pElem;
    int err_code = aws_common_hash_table_init(&hash_table, alloc, 10, aws_common_hash_string,
        aws_common_string_eq, NULL);
    int was_created = 42;

    ASSERT_SUCCESS(err_code,
        "Hash Map init should have succeeded.");

    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_1, &pElem, &was_created); //(void *)test_val_str_1);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");
    ASSERT_INT_EQUALS(1, was_created, "Hash Map create should have created a new element.");
    pElem->value = (void *)test_val_str_1;

    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_1, &pElem, &was_created);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");
    ASSERT_INT_EQUALS(0, was_created, "Hash Map create should not have created a new element.");
    ASSERT_PTR_EQUALS(test_val_str_1, pElem->value, "Create should have returned the old value.");
    pElem->value = (void *)test_val_str_2;

    pElem = NULL;
    err_code = aws_common_hash_table_find(&hash_table, (void *)test_str_1, &pElem);
    ASSERT_SUCCESS(err_code,
        "Hash Map get should have succeeded.");
    ASSERT_PTR_EQUALS(test_val_str_2, pElem->value, "The new value should have been preserved on get");

    aws_common_hash_table_clean_up(&hash_table);
    RETURN_SUCCESS("%s() pass", "test_hash_table_hash_overwrite");
}

static struct aws_common_hash_element last_removed;
static int removal_counter = 0;
static void destroy_fn(struct aws_common_hash_element element) {
    last_removed = element;
    removal_counter++;
}
static void reset_destroy_ck() {
    removal_counter = 0;
    memset(&last_removed, 0, sizeof(last_removed));
}

AWS_TEST_CASE(test_hash_table_hash_remove, test_hash_table_hash_remove_fn)
static int test_hash_table_hash_remove_fn(struct aws_allocator *alloc, void *ctx) {
    struct aws_common_hash_table hash_table;
    struct aws_common_hash_element *pElem, elem;
    int err_code = aws_common_hash_table_init(&hash_table, alloc, 10, aws_common_hash_string,
        aws_common_string_eq, destroy_fn);
    int was_present = 42;

    reset_destroy_ck();

    ASSERT_SUCCESS(err_code,
        "Hash Map init should have succeeded.");

    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_1, NULL, NULL);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");

    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_2, &pElem, NULL);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");
    pElem->value = (void *)test_val_str_2;

    /* Create a second time; this should not invoke destroy */
    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_2, &pElem, NULL);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");

    ASSERT_INT_EQUALS(0, removal_counter, "No elements should be destroyed at this point");

    err_code = aws_common_hash_table_remove(&hash_table, (void *)test_str_1, &elem, &was_present);
    ASSERT_SUCCESS(err_code,
        "Hash Map remove should have succeeded.");
    ASSERT_INT_EQUALS(0, removal_counter, "No elements should be destroyed at this point");
    ASSERT_INT_EQUALS(1, was_present, "Item should have been removed");

    err_code = aws_common_hash_table_find(&hash_table, (void *)test_str_1, &pElem);
    ASSERT_SUCCESS(err_code, "Find for nonexistent item should still succeed");
    ASSERT_NULL(pElem, "Expected item to be nonexistent");

    err_code = aws_common_hash_table_find(&hash_table, (void *)test_str_2, &pElem);
    ASSERT_SUCCESS(err_code,
        "Hash Map get should have succeeded.");

    ASSERT_PTR_EQUALS(test_val_str_2, pElem->value, "Wrong value returned from second get");

    // If we delete and discard the element, destroy_fn should be invoked
    elem = *pElem; // save to compare later
    err_code = aws_common_hash_table_remove(&hash_table, (void *)test_str_2, NULL, NULL);
    ASSERT_SUCCESS(err_code,
        "Remove should have succeeded.");
    ASSERT_INT_EQUALS(1, removal_counter, "One element should be destroyed at this point");
    ASSERT_PTR_EQUALS(last_removed.value, test_val_str_2, "Wrong element destroyed");

    // If we delete an element that's not there, we shouldn't invoke destroy_fn
    err_code = aws_common_hash_table_remove(&hash_table, (void *)test_str_1, NULL, &was_present);
    ASSERT_SUCCESS(err_code, "Remove still should succeed on nonexistent items");
    ASSERT_INT_EQUALS(0, was_present, "Remove should indicate item not present");
    ASSERT_INT_EQUALS(1, removal_counter, "We shouldn't delete an item if none was found");

    aws_common_hash_table_clean_up(&hash_table);
    RETURN_SUCCESS("%s() pass", "test_hash_table_hash_remove");
}

AWS_TEST_CASE(test_hash_table_hash_clear_allows_cleanup, test_hash_table_hash_clear_allows_cleanup_fn)
static int test_hash_table_hash_clear_allows_cleanup_fn(struct aws_allocator *alloc, void *ctx) {
    struct aws_common_hash_table hash_table;
    int err_code = aws_common_hash_table_init(&hash_table, alloc, 10, aws_common_hash_string,
        aws_common_string_eq, destroy_fn);

    ASSERT_SUCCESS(err_code,
        "Hash Map init should have succeeded.");

    reset_destroy_ck();

    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_1, NULL, NULL);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");
    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_2, NULL, NULL);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");

    aws_common_hash_table_clear(&hash_table);
    ASSERT_INT_EQUALS(2, removal_counter, "Clear should destroy all items");

    struct aws_common_hash_element *pElem;
    err_code = aws_common_hash_table_find(&hash_table, (void *)test_str_1, &pElem);
    ASSERT_SUCCESS(err_code, "Find should still succeed after clear");
    ASSERT_NULL(pElem, "Element should not be found");

    reset_destroy_ck();

    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_1, NULL, NULL);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");
    err_code = aws_common_hash_table_create(&hash_table, (void *)test_str_2, NULL, NULL);
    ASSERT_SUCCESS(err_code,
        "Hash Map put should have succeeded.");

    aws_common_hash_table_clean_up(&hash_table);
    ASSERT_INT_EQUALS(2, removal_counter, "Cleanup should destroy all items");

    RETURN_SUCCESS("%s() pass", "test_hash_table_hash_clear_allows_cleanup");
}

AWS_TEST_CASE(test_hash_table_on_resize_returns_correct_entry, test_hash_table_on_resize_returns_correct_entry_fn)
static int test_hash_table_on_resize_returns_correct_entry_fn(struct aws_allocator *alloc, void *ctx) {
    struct aws_common_hash_table hash_table;
    int err_code = aws_common_hash_table_init(&hash_table, alloc, 10, aws_common_hash_ptr,
        aws_common_ptr_eq, NULL);

    ASSERT_SUCCESS(err_code,
        "Hash Map init should have succeeded.");

    for (int i = 0; i < 20; i++) {
        struct aws_common_hash_element *pElem;
        int was_created;
        err_code = aws_common_hash_table_create(&hash_table, (void *)(intptr_t)i, &pElem, &was_created);

        ASSERT_SUCCESS(err_code, "Create should have succeeded");
        ASSERT_INT_EQUALS(1, was_created, "Create should have created new element");
        ASSERT_PTR_EQUALS(NULL, pElem->value, "New element should have null value");
        pElem->value = &hash_table;
    }

    aws_common_hash_table_clean_up(&hash_table);
    RETURN_SUCCESS("%s() pass", "test_hash_table_on_resize_returns_correct_entry");
}

static int foreach_cb_tomask(void *context, struct aws_common_hash_element *pElement) {
    int *pMask = context;
    uintptr_t index = (uintptr_t)pElement->key;

    *pMask |= (1 << index);

    return AWS_COMMON_HASH_TABLE_ITER_CONTINUE;
}

static int iter_count = 0;
static int foreach_cb_deltarget(void *context, struct aws_common_hash_element *pElement) {
    void **pTarget = context;
    int rv = AWS_COMMON_HASH_TABLE_ITER_CONTINUE;

    if (pElement->key == *pTarget) {
        rv |= AWS_COMMON_HASH_TABLE_ITER_DELETE;
    }
    iter_count++;

    return rv;
}

static int foreach_cb_cutoff(void *context, struct aws_common_hash_element *pElement) {
    int *pRemain = context;
    iter_count++;
    if (--*pRemain) {
        return AWS_COMMON_HASH_TABLE_ITER_CONTINUE;
    } else {
        return 0;
    }
}
static int foreach_cb_cutoff_del(void *context, struct aws_common_hash_element *pElement) {
    int *pRemain = context;
    iter_count++;
    if (--*pRemain) {
        return AWS_COMMON_HASH_TABLE_ITER_CONTINUE;
    } else {
        *pRemain = (int)pElement->key;
        return AWS_COMMON_HASH_TABLE_ITER_DELETE;
    }
}


AWS_TEST_CASE(test_hash_table_foreach, test_hash_table_foreach_fn)
static int test_hash_table_foreach_fn(struct aws_allocator *alloc, void *ctx) {
    struct aws_common_hash_table hash_table;
    ASSERT_SUCCESS(aws_common_hash_table_init(&hash_table, alloc, 10, aws_common_hash_ptr, aws_common_ptr_eq, NULL),
        "hash table init"
    );

    for (int i = 0; i < 8; i++) {
        struct aws_common_hash_element *pElem;
        ASSERT_SUCCESS(aws_common_hash_table_create(&hash_table, (void *)(intptr_t)i, &pElem, NULL), "insert element");
        pElem->value = NULL;
    }

    // We should find all four elements
    int mask = 0;
    ASSERT_SUCCESS(
        aws_common_hash_table_foreach(&hash_table, foreach_cb_tomask, &mask), "foreach invocation");
    ASSERT_INT_EQUALS(0xff, mask, "bitmask");

    void *target = (void *)(uintptr_t)3;
    iter_count = 0;
    ASSERT_SUCCESS(
        aws_common_hash_table_foreach(&hash_table, foreach_cb_deltarget, &target), "foreach invocation");
    ASSERT_INT_EQUALS(8, iter_count, "iteration should not stop when deleting");

    mask = 0;
    ASSERT_SUCCESS(
        aws_common_hash_table_foreach(&hash_table, foreach_cb_tomask, &mask), "foreach invocation");
    ASSERT_INT_EQUALS(0xf7, mask, "element 3 deleted");

    iter_count = 0;
    int remain = 4;
    ASSERT_SUCCESS(
        aws_common_hash_table_foreach(&hash_table, foreach_cb_cutoff, &remain), "foreach invocation");
    ASSERT_INT_EQUALS(0, remain, "no more remaining iterations");
    ASSERT_INT_EQUALS(4, iter_count, "correct iteration count");

    iter_count = 0;
    remain = 4;

    ASSERT_SUCCESS(
        aws_common_hash_table_foreach(&hash_table, foreach_cb_cutoff_del, &remain), "foreach invocation");
    ASSERT_INT_EQUALS(4, iter_count, "correct iteration count");
    // we use remain as a side channel to report which element we deleted
    int expected_mask = 0xf7 & ~(1 << remain);


    mask = 0;
    ASSERT_SUCCESS(
        aws_common_hash_table_foreach(&hash_table, foreach_cb_tomask, &mask), "foreach invocation");
    ASSERT_INT_EQUALS(expected_mask, mask, "stop element deleted");

    aws_common_hash_table_clean_up(&hash_table);

    RETURN_SUCCESS("%s() pass", "test_hash_table_foreach");
}

struct churn_entry {
    void *key;
    int original_index;
    void *value;
    int is_removed;
};

static int qsort_churn_entry(const void *a, const void *b) {
    const struct churn_entry *const *p1 = a, *const *p2 = b;
    const struct churn_entry *e1 = *p1, *e2 = *p2;

    if (e1->key < e2->key) {
        return -1;
    } else if (e1->key > e2->key) {
        return 1;
    } else if (e1->original_index < e2->original_index) {
        return -1;
    } else if (e1->original_index > e2->original_index) {
        return 1;
    } else {
        return 0;
    }
}

static long timestamp() {
    uint64_t time = 0;
    aws_sys_clock_get_ticks(&time);
    return (long)(time / 1000);
}

AWS_TEST_CASE(test_hash_churn, test_hash_churn_fn)
static int test_hash_churn_fn(struct aws_allocator *alloc, void *ctx) {
    int i = 0;
    struct aws_common_hash_table hash_table;
    int nentries = 2 * 512 * 1024;
    int err_code = aws_common_hash_table_init(&hash_table, alloc, nentries, aws_common_hash_ptr,
        aws_common_ptr_eq, NULL);

    if(AWS_ERROR_SUCCESS != err_code) {
        FAIL("hash table creation failed: %d", err_code);
    }

    /* Probability that we deliberately try to overwrite.
       Note that random collisions can occur, and are not explicitly avoided. */
    double pOverwrite = 0.05;
    double pDelete = 0.05;

    struct churn_entry *entries = calloc(sizeof(*entries), nentries);
    struct churn_entry **permuted = calloc(sizeof(*permuted), nentries);

    for (i = 0; i < nentries; i++) {
        struct churn_entry *e = &entries[i];
        permuted[i] = e;
        e->original_index = i;

        int mode = 0; /* 0 = new entry, 1 = overwrite, 2 = delete */

        if (i != 0) {
            double p = (double)rand();
            if (p < pOverwrite) {
                mode = 1;
            } else if (p < pOverwrite + pDelete) {
                mode = 2;
            }
        }

        e->is_removed = 0;
        if (mode == 0) {
            e->key = (void *)(uintptr_t)rand();
            e->value = (void *)(uintptr_t)rand();
        } else if (mode == 1) {
            e->key = entries[(size_t)rand() % i].key; /* not evenly distributed but close enough */
            e->value = (void *)(uintptr_t)rand();
        } else if (mode == 2) {
            e->key = entries[(size_t)rand() % i].key; /* not evenly distributed but close enough */
            e->value = 0;
            e->is_removed = 1;
        }
    }

    qsort(permuted, nentries, sizeof(*permuted), qsort_churn_entry);

    long start = timestamp();

    for (i = 0; i < nentries; i++) {
        if (!(i % 100000)) {
            printf("Put progress: %d/%d\n", i, nentries);
        }
        struct churn_entry *e = &entries[i];
        if (e->is_removed) {
            int was_present;
            err_code = aws_common_hash_table_remove(&hash_table, e->key, NULL, &was_present);
            if (i == 0 && entries[i - 1].key == e->key && entries[i - 1].is_removed) {
                ASSERT_INT_EQUALS(0, was_present,"Expected item to be missing");
            } else {
                ASSERT_INT_EQUALS(1, was_present, "Expected item to be present");
            }
        } else {
            struct aws_common_hash_element *pElem;
            int was_created;
            err_code = aws_common_hash_table_create(&hash_table, e->key, &pElem, &was_created);

            pElem->value = e->value;
        }
    }

    for (i = 0; i < nentries; i++) {
        if (!(i % 100000)) {
            printf("Check progress: %d/%d\n", i, nentries);
        }
        struct churn_entry *e = permuted[i];

        if (i < nentries - 1 && permuted[i + 1]->key == e->key) {
            // overwritten on subsequent step
            continue;
        }

        struct aws_common_hash_element *pElem;
        aws_common_hash_table_find(&hash_table, e->key, &pElem);

        if (e->is_removed) {
            ASSERT_NULL(pElem, "expected item to be deleted");
        } else {
            ASSERT_NOT_NULL(pElem, "expected item to be present");
            ASSERT_PTR_EQUALS(e->value, pElem->value, "wrong value for item");
        }
    }

    aws_common_hash_table_clean_up(&hash_table);

    long end = timestamp();

    free(entries);

    RETURN_SUCCESS("%s() pass elapsed=%ld us", "test_hash_churn", end - start);
}
