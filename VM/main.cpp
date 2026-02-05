#include "MontadorSemVibecode.h"

int main(int argc, char *argv[]) {

  MontadorSemVibecode stuff = MontadorSemVibecode();

  stuff.montar("../data/programa1.txt", "../data/programa1_result.txt");
}