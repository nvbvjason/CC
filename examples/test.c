struct three_bytes {
    char arr[3];
};

int main(void) {
    static struct three_bytes s;
    unsigned long start_addr = (unsigned long)&s;
    unsigned long arr_addr = (unsigned long)&s.arr;
    unsigned long arr0_addr = (unsigned long)&s.arr[0];
    unsigned long arr1_addr = (unsigned long)&s.arr[1];
    unsigned long arr1_addr_alt = (unsigned long)(s.arr + 1);
    unsigned long arr2_addr = (unsigned long)&s.arr[2];
    unsigned long arr_end = (unsigned long)(&s.arr + 1);
    unsigned long struct_end = (unsigned long)(&s + 1);
    if (start_addr != arr_addr) {
        return 1;
    }
    if (start_addr != arr0_addr) {
        return 2;
    }
    if (arr1_addr - start_addr != 1) {
        return 3;
    }
    if (arr1_addr != arr1_addr_alt) {
        return 4;
    }
    if (arr2_addr - start_addr != 2) {
        return 5;
    }
    if (arr_end - start_addr != 3) {
        return 6;
    }
    if (struct_end - start_addr != 3) {
        return 7;
    }
    return 0;
}