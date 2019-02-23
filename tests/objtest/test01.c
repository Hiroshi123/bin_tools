
int mock1() {
  return 1;
}

int mock2() {
  f2();  
  return 2;
}

char test__1() {
  mock1();
  int res = f1();
  return res == 5;
}
