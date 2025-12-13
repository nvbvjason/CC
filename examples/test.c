void *malloc(unsigned long size);

struct inner {
    double d;
    int i;
};

struct outer {
    char a;
    char b;
    struct inner substruct;
};

struct outer *get_static_struct_ptr(void) {
    static struct outer s;
    return &s;
}

int main(void) {
    struct inner small = {12.0, 13};
    get_static_struct_ptr()->substruct = small;
    if (get_static_struct_ptr()->substruct.d != 12.0) {
        return 0;
    }
    if (get_static_struct_ptr()->substruct.i != 13) {
        return 0;
    }
    return 1;
}