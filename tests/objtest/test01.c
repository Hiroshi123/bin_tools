
int mock1() {
  return 1;
}

int mock2() {
  return 3;
}

char test__1(char* p) {
  // mock1();
  char res = f1();
  // int res = 5;
  return res;
}

