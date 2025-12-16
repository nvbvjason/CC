struct inner {
    char *three_msg;
};

struct inner partial = {
    "Hello!"
};

int main(void)
{
    static char* msg = "Hello!";
    return 0;
}