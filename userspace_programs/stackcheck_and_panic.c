void user_main() {
    // stack
    int a = 123;
    int b = a * 2;
    int arr[10];
    arr[0] = b;
    arr[1] = b + 1;

    // panic
    asm volatile("ud2");
    while(1);
}