int main(int argc, char** argv) {
    if (argc < 4) return ERR_USAGE;
    return traiter_leaks(argv[1], argv[2], argv[3]);
}
