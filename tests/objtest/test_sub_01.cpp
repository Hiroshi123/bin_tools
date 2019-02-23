
class B {
public:
  int f1();
};

int f1() {
  return 3;
}

class A {
  int a = 3;
  int b = 5;
  int f1();
  int f2();
};

int A::f1() {
  return a+b+3;
}

int A::f2() {
  return a;
}

