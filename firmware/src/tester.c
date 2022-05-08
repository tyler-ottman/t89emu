int fib(int n);
int main() {
    int a = 5;
    int b = 0;
    int c[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i < a; i++) {
        b += 2;
    }
    return fib(c[4]);
}
int fib(int n) {
    if (n == 0 || n == 1)
        return n;
    else
        return (fib(n-1) + fib(n-2));
}