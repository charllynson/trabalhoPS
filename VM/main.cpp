#include "MontadorSemVibecode.h"

int main(int argc, char *argv[]) {

  MontadorSemVibecode stuff = MontadorSemVibecode();

  stuff.montar("../data/programa2.txt", "./programa2_result.txt");
}