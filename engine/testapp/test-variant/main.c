#include "e8core/variant/variant.h"
#include <stdio.h>

static void
test_clustered_list()
{

    struct {
        E8_CLUSTERED_LIST(int)
    } list;

    e8_clustered_list_init(&list, sizeof(int));

    int TEST_SIZE = 200;

    printf("Append: ");

    int i;
    for (i = 0; i < TEST_SIZE; ++i) {
        void *a;
        e8_clustered_list_append(&list, &a);
        (*(int*)a) = i;
    }

    printf("Ok\n");
    fflush(stdout);

    struct {
        E8_COLLECTION(int)
    } coll;

    printf("Dump: ");

    e8_clustered_list_dump(&list, &coll);
    e8_clustered_list_free(&list);


    if (coll.size != TEST_SIZE) {
        printf("Failed\n");
        return;
    }

    for (i = 0; i < TEST_SIZE; ++i) {
        if (coll.data[i] != i) {
            printf("Failed\n");
            return;
        }
    }
    e8_collection_free(&coll);

    printf("Ok\n");

}

int main()
{
    test_clustered_list();
    return 0;
}
