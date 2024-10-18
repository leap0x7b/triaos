unsigned int __mulsi3(unsigned int a, unsigned int b) {
    unsigned int p = 0;
    while (a != 0) {
        if ((a & 0x1) != 0) {
            p += b;
        }
        a >>= 1;
        b <<= 1;
    }
    return p;
}
